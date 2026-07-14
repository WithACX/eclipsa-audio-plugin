#!/usr/bin/env bash
#
# acx-upstream-pr.sh -- one deliberate command to open a CLEAN PR to a public
# upstream from an A-CX fork (DPM-07, objective O9).
#
# What it does, in order:
#   1. Verify preconditions (git repo, upstream + fork remotes, clean tree).
#   2. Fetch the upstream and fast-forward local `main` to the upstream default
#      branch (main mirrors upstream cleanly -- ADR-0067 topology).
#   3. Create a fresh contribution branch FROM `main`, so A-CX tooling commits
#      (which live only on `acx/dev`) structurally cannot appear in the diff.
#   4. Cherry-pick the contribution commits (merge-base(upstream, <source>)..
#      <source>) onto the fresh branch.
#   5. Run the repo's installed pre-push guard against the exact upstream-bound
#      range as a BACKSTOP; abort if any guarded A-CX path is present.
#   6. Push the branch to the fork and open the PR against the upstream
#      (head = fork branch, base = upstream default branch).
#
# It is a DELIBERATE, opt-in action. deliver-code never calls it. It works from
# Claude Code and Cowork (both drive git/gh the same way).
#
# This is the ADR-0067 "cut the PR from main" flow, automated. The clean-branch
# topology is the PRIMARY control; the pre-push guard is the backstop.
#
# Usage:
#   acx-upstream-pr.sh --branch <name> [--source <ref>] [--base <branch>]
#                      [--title <text>] [--body <text>] [--no-pr] [--dry-run]
#
# Options:
#   --branch <name>   Contribution branch to create and push (required).
#   --source <ref>    Ref whose commits form the contribution (default: HEAD,
#                     as it was before syncing main). Its commits since its
#                     merge-base with the upstream base are replayed onto the
#                     fresh branch.
#   --base <branch>   Upstream base branch (default: auto-detected upstream HEAD,
#                     falling back to `main`).
#   --title <text>    PR title (default: the source's last commit subject).
#   --body <text>     PR body (default: a short generated note).
#   --no-pr           Do everything except open the PR (push only).
#   --dry-run         Print the plan and run read-only steps; make no branch,
#                     no push, no PR.
#
# Environment:
#   ACX_UPSTREAM_REMOTE   upstream remote name (default: upstream)
#   ACX_FORK_REMOTE       fork remote name    (default: origin)
#
set -euo pipefail

UPSTREAM_REMOTE="${ACX_UPSTREAM_REMOTE:-upstream}"
FORK_REMOTE="${ACX_FORK_REMOTE:-origin}"

BRANCH=""
SOURCE="HEAD"
BASE=""
TITLE=""
BODY=""
OPEN_PR=1
DRY_RUN=0

die()  { echo "acx-upstream-pr: ERROR: $*" >&2; exit 1; }
note() { echo "acx-upstream-pr: $*" >&2; }

# runcmd: execute a command given as separate args (no eval, no re-quoting).
# In --dry-run mode it prints the command instead of running it.
runcmd() {
  if [ "$DRY_RUN" -eq 1 ]; then
    printf '  [dry-run]' >&2; printf ' %q' "$@" >&2; printf '\n' >&2
    return 0
  fi
  "$@"
}

while [ $# -gt 0 ]; do
  case "$1" in
    --branch)  BRANCH="${2:-}"; shift 2 ;;
    --source)  SOURCE="${2:-}"; shift 2 ;;
    --base)    BASE="${2:-}";   shift 2 ;;
    --title)   TITLE="${2:-}";  shift 2 ;;
    --body)    BODY="${2:-}";   shift 2 ;;
    --no-pr)   OPEN_PR=0; shift ;;
    --dry-run) DRY_RUN=1; shift ;;
    -h|--help) sed -n '2,46p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) die "unknown argument: $1 (see --help)" ;;
  esac
done

# --- 1. Preconditions -------------------------------------------------------
command -v git >/dev/null 2>&1 || die "git not found on PATH"
command -v gh  >/dev/null 2>&1 || die "gh (GitHub CLI) not found on PATH"
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || die "not inside a git repository"

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

[ -n "$BRANCH" ] || die "--branch <name> is required"
git remote get-url "$UPSTREAM_REMOTE" >/dev/null 2>&1 \
  || die "no '$UPSTREAM_REMOTE' remote (set ACX_UPSTREAM_REMOTE or add the remote)"
git remote get-url "$FORK_REMOTE" >/dev/null 2>&1 \
  || die "no '$FORK_REMOTE' remote (set ACX_FORK_REMOTE or add the remote)"

if [ -n "$(git status --porcelain)" ]; then
  die "working tree is not clean -- commit or stash changes before running"
fi

# Resolve the source tip and its subject BEFORE any branch switching.
SOURCE_SHA="$(git rev-parse --verify "$SOURCE^{commit}" 2>/dev/null)" \
  || die "--source '$SOURCE' is not a valid commit"
[ -z "$TITLE" ] && TITLE="$(git log -1 --pretty=%s "$SOURCE_SHA")"

git show-ref --verify --quiet "refs/heads/$BRANCH" \
  && die "branch '$BRANCH' already exists -- choose a new --branch name"

# Locate the installed pre-push guard now, BEFORE creating any branch, so a
# missing guard aborts cleanly with nothing to unwind. Honor core.hooksPath
# (an existing hooks manager), then the tracked and legacy hook paths.
GUARD=""
HOOKS_PATH="$(git config --get core.hooksPath || true)"
for cand in "${HOOKS_PATH:+$HOOKS_PATH/pre-push}" ".a-cx/hooks/pre-push" ".git/hooks/pre-push"; do
  [ -n "$cand" ] && [ -f "$cand" ] && { GUARD="$cand"; break; }
done
if [ "$DRY_RUN" -eq 0 ] && [ -z "$GUARD" ]; then
  die "pre-push guard not found (run /setup-repo) -- refusing to open an upstream PR without the leak guard"
fi

# --- 2. Fetch upstream + fast-forward main ---------------------------------
note "fetching '$UPSTREAM_REMOTE'..."
runcmd git fetch --prune "$UPSTREAM_REMOTE"

if [ -z "$BASE" ]; then
  # Upstream default branch, e.g. 'main' or 'master'.
  BASE="$(git remote show "$UPSTREAM_REMOTE" 2>/dev/null \
          | sed -n 's/.*HEAD branch: //p' | head -1)"
  [ -n "$BASE" ] || BASE="main"
fi
note "upstream base branch: $BASE"
git rev-parse --verify --quiet "refs/remotes/$UPSTREAM_REMOTE/$BASE" >/dev/null \
  || die "upstream base '$UPSTREAM_REMOTE/$BASE' not found after fetch"

# Contribution commit range: commits reachable from <source> but not from the
# freshly fetched upstream base (already-merged commits are excluded).
MERGE_BASE="$(git merge-base "$UPSTREAM_REMOTE/$BASE" "$SOURCE_SHA")" \
  || die "cannot find a merge-base between $UPSTREAM_REMOTE/$BASE and $SOURCE"
COMMIT_COUNT="$(git rev-list --count "$MERGE_BASE..$SOURCE_SHA")"
[ "$COMMIT_COUNT" -gt 0 ] || die "no contribution commits in $MERGE_BASE..$SOURCE (nothing to PR)"
note "replaying $COMMIT_COUNT commit(s) onto a clean branch from $BASE"

# Best-effort: keep a local `main` current with the upstream base. The PR branch
# is cut from the remote-tracking ref directly (below), so a divergent or absent
# local `main` never blocks the PR -- this is a convenience, never a gate.
if [ "$DRY_RUN" -eq 0 ] && git show-ref --verify --quiet "refs/heads/main"; then
  if git merge-base --is-ancestor "refs/heads/main" "$UPSTREAM_REMOTE/$BASE"; then
    if git checkout main >/dev/null 2>&1 && git merge --ff-only "$UPSTREAM_REMOTE/$BASE" >/dev/null 2>&1; then
      note "fast-forwarded local 'main' to $UPSTREAM_REMOTE/$BASE"
    fi
  else
    note "local 'main' has diverged from $UPSTREAM_REMOTE/$BASE; leaving it untouched (the PR branch is cut from $UPSTREAM_REMOTE/$BASE, not local main)"
  fi
fi

# --- 3 + 4. Fresh branch from the upstream base, cherry-pick contribution ---
runcmd git checkout -b "$BRANCH" "$UPSTREAM_REMOTE/$BASE"
if [ "$DRY_RUN" -eq 0 ]; then
  if ! git cherry-pick "$MERGE_BASE..$SOURCE_SHA"; then
    git cherry-pick --abort 2>/dev/null || true
    git checkout - >/dev/null 2>&1 || true
    git branch -D "$BRANCH" >/dev/null 2>&1 || true
    die "cherry-pick onto '$BRANCH' hit a conflict -- rebase your source onto $UPSTREAM_REMOTE/$BASE and retry"
  fi
fi

# --- 5. Pre-push guard backstop (respect it; never bypass) ------------------
# GUARD was located in the preconditions, before the branch existed.
if [ "$DRY_RUN" -eq 0 ]; then
  BRANCH_SHA="$(git rev-parse HEAD)"
  BASE_SHA="$(git rev-parse "$UPSTREAM_REMOTE/$BASE")"
  UPSTREAM_URL="$(git remote get-url "$UPSTREAM_REMOTE")"
  note "checking the upstream-bound diff against the pre-push guard ($GUARD)..."
  # Invoke the guard exactly as git would: `<hook> <remote-name> <remote-url>`
  # with the ref update on stdin. Run it directly when executable (its shebang
  # picks the interpreter -- works for the Python A-CX guards and for a shell
  # hooks-manager pre-push); fall back to python3 for a non-executable copy.
  if [ -x "$GUARD" ]; then
    guard_cmd=( "$GUARD" "$UPSTREAM_REMOTE" "$UPSTREAM_URL" )
  else
    guard_cmd=( python3 "$GUARD" "$UPSTREAM_REMOTE" "$UPSTREAM_URL" )
  fi
  # Feed the guard the exact ref update git would send on a push to upstream.
  if ! printf 'refs/heads/%s %s refs/heads/%s %s\n' "$BRANCH" "$BRANCH_SHA" "$BASE" "$BASE_SHA" \
       | ACX_UPSTREAM_REMOTE="$UPSTREAM_REMOTE" "${guard_cmd[@]}"; then
    git checkout - >/dev/null 2>&1 || true
    git branch -D "$BRANCH" >/dev/null 2>&1 || true
    die "guard blocked the diff -- a guarded A-CX path is in the contribution; clean it and retry"
  fi
  note "guard clean: no A-CX paths in the upstream-bound diff"
fi

# --- 6. Push to fork + open PR against upstream ----------------------------
runcmd git push -u "$FORK_REMOTE" "$BRANCH"

if [ "$OPEN_PR" -eq 0 ]; then
  note "pushed '$BRANCH' to '$FORK_REMOTE'. --no-pr set: skipping PR creation."
  exit 0
fi

UPSTREAM_SLUG="$(gh repo view "$(git remote get-url "$UPSTREAM_REMOTE")" --json nameWithOwner -q .nameWithOwner 2>/dev/null || true)"
[ -n "$UPSTREAM_SLUG" ] || die "cannot resolve the upstream repo slug from remote '$UPSTREAM_REMOTE'"
FORK_OWNER="$(gh repo view "$(git remote get-url "$FORK_REMOTE")" --json owner -q .owner.login 2>/dev/null || true)"
[ -n "$FORK_OWNER" ] || die "cannot resolve the fork owner from remote '$FORK_REMOTE'"
[ -z "$BODY" ] && BODY="Contribution from an A-CX fork. Cut cleanly from \`$BASE\`; contains no A-CX tooling paths."

note "opening PR: $FORK_OWNER:$BRANCH -> $UPSTREAM_SLUG:$BASE"
runcmd gh pr create --repo "$UPSTREAM_SLUG" --base "$BASE" \
  --head "$FORK_OWNER:$BRANCH" --title "$TITLE" --body "$BODY"
note "done."
