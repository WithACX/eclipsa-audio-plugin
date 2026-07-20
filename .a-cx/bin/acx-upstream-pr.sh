#!/usr/bin/env bash
#
# acx-upstream-pr.sh -- one deliberate command to open a CLEAN PR to a public
# upstream from an A-CX fork (DPM-07, objective O9).
#
# Version: 2  (PRG-02: assigns an authorized reviewer on the opened upstream PR;
#              existing repos must re-run /setup-repo to pick up this version)
#
# What it does, in order:
#   1. Verify preconditions (git repo, upstream + fork remotes, clean tree).
#   2. Resolve the contribution source. When --source is omitted, default to the
#      OPEN fork PR for the current branch (most upstream PRs mirror a fork PR);
#      fall back to HEAD when no such PR is found. Explicit --source overrides.
#   3. Fetch the upstream and fast-forward local `main` to the upstream default
#      branch (main mirrors upstream cleanly -- ADR-0067 topology).
#   4. Create a fresh contribution branch FROM `main`, so A-CX tooling commits
#      (which live only on `acx/dev`) structurally cannot appear in the diff.
#   5. Replay the contribution commits (merge-base(upstream, <source>)..<source>)
#      onto the fresh branch, AUTO-EXCLUDING commits that touch only guarded
#      internal paths (DPM-11). A commit that MIXES internal paths with
#      contribution code aborts the run (nothing pushed) -- split it per the
#      DPM-10 "Commit Separation on acx/dev" rule and retry. Exclusion reuses the
#      leak guard's shared `classify` matcher, so "internal" means one thing.
#   6. Run the repo's installed pre-push guard against the exact upstream-bound
#      range as a BACKSTOP; abort if any guarded A-CX path is present.
#   7. Push the branch to the fork and open the PR against the upstream
#      (head = fork branch, base = upstream default branch).
#
# It is a DELIBERATE, opt-in action. deliver-code never calls it. It works from
# Claude Code and Cowork (both drive git/gh the same way).
#
# This is the ADR-0067 "cut the PR from main" flow, automated. The clean-branch
# topology is the PRIMARY control; the pre-push guard is the backstop.
#
# Usage:
#   acx-upstream-pr.sh --branch <name> --reviewer <login> [--source <ref>]
#                      [--base <branch>] [--title <text>] [--body <text>]
#                      [--no-pr] [--dry-run]
#
# Options:
#   --branch <name>   Contribution branch to create and push (required).
#   --reviewer <login> Upstream collaborator to request as reviewer on the opened
#                     PR (required to open a PR; not needed with --no-pr). Must not
#                     be the PR author -- the /upstream-pr command validates that via
#                     the shared reviewer-eligibility helper before calling this
#                     script. GitHub enforces upstream-collaborator status on
#                     assignment; a rejected assignment fails the run (see below).
#   --source <ref>    Ref whose commits form the contribution. Default: the open
#                     fork PR for the current branch, else HEAD. Its commits since
#                     their merge-base with the upstream base are replayed onto the
#                     fresh branch, minus any config-only and merge commits (see
#                     step 5). NOTE: the range is "everything on <ref> not yet in
#                     upstream", which on a drifted fork can be far wider than one
#                     fix -- use --single to contribute only <ref> itself.
#   --single          Contribute ONLY the --source commit (its parent..<source>),
#                     not the whole merge-base range. Use when --source names a
#                     single commit and you want just that commit upstreamed.
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
REVIEWER=""
SOURCE="HEAD"
SOURCE_EXPLICIT=0
BASE=""
TITLE=""
BODY=""
OPEN_PR=1
DRY_RUN=0
SINGLE=0
# Above this many replayed commits, warn loudly (a wide range usually means the
# fork has drifted from upstream and --source/--single should be narrowed).
WIDE_RANGE_WARN=10

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
    --branch)   BRANCH="${2:-}"; shift 2 ;;
    --reviewer) REVIEWER="${2:-}"; shift 2 ;;
    --source)  SOURCE="${2:-}"; SOURCE_EXPLICIT=1; shift 2 ;;
    --base)    BASE="${2:-}";   shift 2 ;;
    --title)   TITLE="${2:-}";  shift 2 ;;
    --body)    BODY="${2:-}";   shift 2 ;;
    --single)  SINGLE=1; shift ;;
    --no-pr)   OPEN_PR=0; shift ;;
    --dry-run) DRY_RUN=1; shift ;;
    -h|--help) sed -n '2,68p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
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
# A reviewer is mandatory to OPEN a PR (objective O2): an upstream contribution
# must never land with nobody accountable to review it. --no-pr (push only) needs
# no reviewer since no PR is opened. The /upstream-pr command prompts for the
# login and validates reviewer-not-author via the shared eligibility helper before
# invoking this script; this check is the script-level backstop for that guarantee.
if [ "$OPEN_PR" -eq 1 ] && [ -z "$REVIEWER" ]; then
  die "--reviewer <login> is required to open a PR (an upstream collaborator, not the author). Use --no-pr to push the clean branch without opening a PR."
fi
# Belt-and-suspenders (matches the SOURCE guard): refuse a reviewer that looks
# like an option, so it can never be parsed as a gh flag rather than a login.
case "$REVIEWER" in
  -*) die "reviewer '$REVIEWER' looks like an option -- refusing" ;;
esac
git remote get-url "$UPSTREAM_REMOTE" >/dev/null 2>&1 \
  || die "no '$UPSTREAM_REMOTE' remote (set ACX_UPSTREAM_REMOTE or add the remote)"
git remote get-url "$FORK_REMOTE" >/dev/null 2>&1 \
  || die "no '$FORK_REMOTE' remote (set ACX_FORK_REMOTE or add the remote)"

if [ -n "$(git status --porcelain)" ]; then
  die "working tree is not clean -- commit or stash changes before running"
fi

# Default source = the open fork PR for the current branch (DPM-11). Most
# upstream PRs mirror a fork PR, so this removes the per-run --source bookkeeping.
# Explicit --source always wins; fall back to HEAD when no open fork PR is found.
if [ "$SOURCE_EXPLICIT" -eq 0 ]; then
  CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  FORK_SLUG="$(gh repo view "$(git remote get-url "$FORK_REMOTE")" --json nameWithOwner -q .nameWithOwner 2>/dev/null || true)"
  PR_HEAD=""
  if [ -n "$FORK_SLUG" ] && [ -n "$CURRENT_BRANCH" ] && [ "$CURRENT_BRANCH" != "HEAD" ]; then
    PR_HEAD="$(gh pr list --repo "$FORK_SLUG" --head "$CURRENT_BRANCH" --state open --json headRefOid -q '.[0].headRefOid' 2>/dev/null || true)"
  fi
  if [ -n "$PR_HEAD" ]; then
    SOURCE="$PR_HEAD"
    note "default source: open fork PR on '$CURRENT_BRANCH' (head ${PR_HEAD:0:12})"
    # The PR head is the last PUSHED tip. Warn if local HEAD is ahead so newer
    # local commits are not silently left out of the contribution.
    HEAD_SHA="$(git rev-parse HEAD 2>/dev/null || true)"
    if [ -n "$HEAD_SHA" ] && [ "$HEAD_SHA" != "$PR_HEAD" ] \
       && git merge-base --is-ancestor "$PR_HEAD" "$HEAD_SHA" 2>/dev/null; then
      AHEAD="$(git rev-list --count "$PR_HEAD..$HEAD_SHA" 2>/dev/null || echo '?')"
      note "note: $AHEAD local commit(s) on HEAD are not yet in the fork PR and will NOT be included -- push them to the PR first, or pass --source HEAD"
    fi
  else
    SOURCE="HEAD"
    note "no open fork PR for '${CURRENT_BRANCH:-detached HEAD}'; defaulting source to HEAD"
  fi
fi

# Guard against a source that could be parsed as a git option (belt-and-suspenders,
# matching check_upstream_leak.py; headRefOid is API hex today, but future-proof it).
case "$SOURCE" in
  -*) die "source '$SOURCE' looks like an option -- refusing" ;;
esac

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

# Snapshot the guard NOW, before any checkout (#456). The backstop (step 5) runs
# AFTER the fresh branch is checked out from the upstream base, where the guard's
# working-tree path (e.g. `.a-cx/hooks/pre-push`) does NOT exist -- it is an
# internal-only path never present on the public upstream. Copy the guard's
# content to a temp file up front and invoke that copy later, so the backstop
# still runs from the clean checkout instead of failing on a missing file.
GUARD_SNAPSHOT=""
if [ -n "$GUARD" ]; then
  GUARD_SNAPSHOT="$(mktemp "${TMPDIR:-/tmp}/acx-prepush.XXXXXX")"
  cp "$GUARD" "$GUARD_SNAPSHOT"
  # Owner-only exec (mktemp made it 0600; `cp` does not carry the exec bit). The
  # python3 fallback at the backstop covers the rare chmod failure.
  chmod u+x "$GUARD_SNAPSHOT" 2>/dev/null || true
  # shellcheck disable=SC2064
  trap "rm -f '$GUARD_SNAPSHOT'" EXIT
fi

# Locate the classify tool for the config-only exclusion (DPM-11). It reuses the
# leak guard's shared matcher: prefer the tracked .a-cx/bin copy (external forks),
# then the installed guard itself (detached repos keep check_upstream_leak.py at
# core.hooksPath/pre-push). Absent -> exclusion is skipped with a warning (the
# pre-push guard still backstops), never a silent leak.
CLASSIFY_TOOL=""
for cand in ".a-cx/bin/check_upstream_leak.py" "${HOOKS_PATH:+$HOOKS_PATH/pre-push}" ".a-cx/hooks/pre-push"; do
  [ -n "$cand" ] && [ -f "$cand" ] && { CLASSIFY_TOOL="$cand"; break; }
done

# classify_commit <sha> -> prints 'empty'|'config-only'|'contribution'|'mixed',
# or 'error' if the tool fails. Runs the tool directly when executable, else via
# python3 (a non-executable tracked copy).
classify_commit() {
  local c="$1"
  if [ -x "$CLASSIFY_TOOL" ]; then
    "$CLASSIFY_TOOL" classify "$c" 2>/dev/null || echo "error"
  else
    python3 "$CLASSIFY_TOOL" classify "$c" 2>/dev/null || echo "error"
  fi
}

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

# Enumerate the candidate commits, oldest first. --single contributes ONLY the
# source commit; otherwise the full range since the merge-base. Merge commits are
# excluded either way: their changes arrive via the individual commits they
# merged, and `git cherry-pick` cannot replay a merge without an explicit parent.
# Enumerating individual non-merge SHAs (never a raw `A..B` range) is what keeps
# a "Merge pull request" commit in a drifted fork's history from crashing the
# cherry-pick (#456).
FULL_COUNT="$(git rev-list --count "$MERGE_BASE..$SOURCE_SHA")"
MERGE_COUNT="$(git rev-list --count --merges "$MERGE_BASE..$SOURCE_SHA")"
if [ "$SINGLE" -eq 1 ]; then
  if git merge-base --is-ancestor "$SOURCE_SHA" "$UPSTREAM_REMOTE/$BASE" 2>/dev/null; then
    die "--single commit $SOURCE is already in $UPSTREAM_REMOTE/$BASE (nothing to PR)"
  fi
  if [ "$(git rev-list --no-walk --count --merges "$SOURCE_SHA")" -ne 0 ]; then
    die "--single commit $SOURCE is a merge commit -- pick a non-merge commit to contribute"
  fi
  CANDIDATES="$(git rev-list --no-walk "$SOURCE_SHA")"
else
  [ "$FULL_COUNT" -gt 0 ] || die "no contribution commits in $MERGE_BASE..$SOURCE (nothing to PR)"
  CANDIDATES="$(git rev-list --reverse --no-merges "$MERGE_BASE..$SOURCE_SHA")"
  # List the skipped merges by SHA+subject (not just a count) so a merge that
  # carried conflict-resolution-only edits -- whose diff exists ONLY in the merge
  # and so is not replayed -- is visible for inspection rather than silently dropped.
  if [ "$MERGE_COUNT" -ne 0 ]; then
    note "skipping $MERGE_COUNT merge commit(s) (changes normally replay via the commits they merged; inspect if any carried conflict-resolution-only edits):"
    while IFS= read -r m; do
      [ -n "$m" ] && note "  skip-merge $(git log -1 --pretty='%h %s' "$m")"
    done < <(git rev-list --merges "$MERGE_BASE..$SOURCE_SHA")
  fi
fi
[ -n "$CANDIDATES" ] || die "no non-merge contribution commits in the selected range (nothing to PR)"

# Filter the replay set (DPM-11): drop config-only commits, ABORT on a mixed
# commit (nothing pushed -- no branch exists yet, so nothing to unwind). Done
# before branch creation and in both real and dry-run mode so the plan is
# accurate. REPLAY holds the commits to cherry-pick, oldest first.
REPLAY=()
if [ -n "$CLASSIFY_TOOL" ]; then
  while IFS= read -r c; do
    [ -n "$c" ] || continue
    verdict="$(classify_commit "$c")"
    case "$verdict" in
      config-only)
        note "excluding config-only commit ${c:0:12} (guarded internal paths only)" ;;
      contribution|empty)
        REPLAY+=("$c") ;;
      mixed)
        die "commit ${c:0:12} mixes A-CX-internal paths with contribution code -- split it per the 'Commit Separation on acx/dev' rule in rules/common/git-workflow.md, then retry (nothing pushed)" ;;
      *)
        die "could not classify commit ${c:0:12} ('$verdict') -- aborting rather than risk leaking internal content (nothing pushed)" ;;
    esac
  done < <(printf '%s\n' "$CANDIDATES")
  [ "${#REPLAY[@]}" -gt 0 ] \
    || die "no contribution commits remain after excluding config-only commits (nothing to PR)"
else
  note "WARNING: classify tool not found (.a-cx/bin/check_upstream_leak.py) -- config-only commits are NOT excluded; run /setup-repo. The pre-push guard still backstops."
  # Replay the non-merge candidates individually (never a range), so a merge
  # commit can never trip the cherry-pick even on this degraded path.
  while IFS= read -r c; do [ -n "$c" ] && REPLAY+=("$c"); done < <(printf '%s\n' "$CANDIDATES")
fi

# Visibility (#456): always list what will be replayed, and warn on a wide range,
# so a drifted fork's sweep is never silent.
if [ "$SINGLE" -eq 1 ]; then
  note "replaying ${#REPLAY[@]} commit(s) via --single onto a clean branch from $BASE (fork is $FULL_COUNT commit(s) ahead of upstream):"
else
  note "replaying ${#REPLAY[@]} commit(s) onto a clean branch from $BASE (range has $FULL_COUNT commit(s), $MERGE_COUNT merge(s)):"
fi
for c in "${REPLAY[@]}"; do note "  - $(git log -1 --pretty='%h %s' "$c")"; done
if [ "$SINGLE" -eq 0 ] && [ "${#REPLAY[@]}" -gt "$WIDE_RANGE_WARN" ]; then
  note "WARNING: ${#REPLAY[@]} commits is a wide contribution range -- the fork may have drifted from upstream. If you meant to contribute a single fix, re-run with: --single --source <that-commit>."
fi

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
  # Cherry-pick the replay set as individual SHAs, oldest first. REPLAY is always
  # a list of non-merge commits (both the classify and the degraded fallback
  # paths), never a raw `A..B` range -- so a merge commit in the range can never
  # reach `git cherry-pick` and crash it (#456).
  if ! git cherry-pick "${REPLAY[@]}"; then
    git cherry-pick --abort 2>/dev/null || true
    git checkout - >/dev/null 2>&1 || true
    git branch -D "$BRANCH" >/dev/null 2>&1 || true
    die "cherry-pick onto '$BRANCH' hit a conflict -- rebase your source onto $UPSTREAM_REMOTE/$BASE and retry"
  fi
fi

# --- 5. Pre-push guard backstop (respect it; never bypass) ------------------
# GUARD_SNAPSHOT was captured in the preconditions, before the branch existed --
# the working-tree guard path does not exist on this clean checkout (#456).
if [ "$DRY_RUN" -eq 0 ]; then
  BRANCH_SHA="$(git rev-parse HEAD)"
  BASE_SHA="$(git rev-parse "$UPSTREAM_REMOTE/$BASE")"
  UPSTREAM_URL="$(git remote get-url "$UPSTREAM_REMOTE")"
  note "checking the upstream-bound diff against the pre-push guard ($GUARD)..."
  # Invoke the guard SNAPSHOT exactly as git would: `<hook> <remote-name>
  # <remote-url>` with the ref update on stdin. Run it directly when executable
  # (its shebang picks the interpreter -- works for the Python A-CX guards and for
  # a shell hooks-manager pre-push); fall back to python3 for a non-executable copy.
  if [ -x "$GUARD_SNAPSHOT" ]; then
    guard_cmd=( "$GUARD_SNAPSHOT" "$UPSTREAM_REMOTE" "$UPSTREAM_URL" )
  else
    guard_cmd=( python3 "$GUARD_SNAPSHOT" "$UPSTREAM_REMOTE" "$UPSTREAM_URL" )
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
if [ "$DRY_RUN" -eq 1 ]; then
  # Print the plan (create + the reviewer assignment) but change nothing.
  printf '  [dry-run]' >&2
  printf ' %q' gh pr create --repo "$UPSTREAM_SLUG" --base "$BASE" \
    --head "$FORK_OWNER:$BRANCH" --title "$TITLE" --body "$BODY" >&2
  printf '\n' >&2
  printf '  [dry-run]' >&2
  printf ' %q' gh pr edit '<new-pr-url>' --add-reviewer "$REVIEWER" >&2
  printf '\n' >&2
  note "done (dry-run): would open the PR and request '$REVIEWER' as reviewer."
  exit 0
fi

# Open the PR and capture its URL so the reviewer can be assigned on it (and so a
# failed assignment can point at the exact PR).
PR_URL="$(gh pr create --repo "$UPSTREAM_SLUG" --base "$BASE" \
  --head "$FORK_OWNER:$BRANCH" --title "$TITLE" --body "$BODY")" \
  || die "gh pr create failed (branch pushed to '$FORK_REMOTE'; open the PR manually)"
note "opened PR: $PR_URL"

# Assign the reviewer on the UPSTREAM PR. This fires GitHub's native
# review-request notification to the reviewer. GitHub rejects a non-collaborator
# or unknown login; on rejection, fail loud (non-zero) with the PR URL and the
# exact command to run by hand -- never leave the PR silently review-less.
note "requesting reviewer '$REVIEWER' on the upstream PR..."
if ! gh pr edit "$PR_URL" --add-reviewer "$REVIEWER"; then
  die "PR opened at $PR_URL but requesting reviewer '$REVIEWER' failed -- likely not a collaborator on $UPSTREAM_SLUG, or an unknown login. Assign an eligible reviewer manually:
    gh pr edit $PR_URL --add-reviewer <login>"
fi
note "reviewer '$REVIEWER' requested on $PR_URL"
note "done."
