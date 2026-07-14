# Fork workflow (A-CX internal)

Internal A-CX process doc. Lives under `.a-cx/` so the pre-push guard and the
branch topology keep it out of any upstream-bound diff. Never move it out of
`.a-cx/` on this fork.

This repo is a fork of `google/eclipsa-audio-plugin`. A-CX develops here and
contributes selected changes back upstream. The rules below keep A-CX tooling
(`.a-cx/`, `.claude/`, `AGENTS.md`, `CLAUDE.md`) out of anything Google sees.

## Local setup (do this once per machine)

This fork has the **same repo name** as the Google upstream
(`eclipsa-audio-plugin`). To avoid confusing your A-CX clone with a plain clone
of Google's repo, put the A-CX clone under a `withACX/` parent folder:

```bash
mkdir -p ~/dev/withACX && cd ~/dev/withACX
git clone https://github.com/WithACX/eclipsa-audio-plugin.git
cd eclipsa-audio-plugin

git remote add upstream https://github.com/google/eclipsa-audio-plugin.git  # if not present
git lfs install                     # this repo uses Git LFS
git checkout acx/dev                # REQUIRED: the working branch (see below)
```

**Important -- `main` is intentionally bare.** It mirrors the Google upstream
and carries none of the A-CX tooling, so a fresh clone left on `main` looks like
there is nothing A-CX here. That is expected. All day-to-day work and all
tooling live on **`acx/dev`** -- always switch to it after cloning. If the
repo's default branch has been set to `acx/dev`, clones land there
automatically; otherwise run the `git checkout acx/dev` above.

Then, inside the clone, run `/setup-repo` in Claude Code once. It installs the
local `pre-push` guard (not committed, so every fresh clone needs this) and the
`/upstream-pr` helper. Confirm remotes: `origin` = the fork, `upstream` = Google.

## Where to run the delivery commands

The project splits code (this fork) from planning (a separate repo). Run each
command where its work lands:

| Command | Run it in | Why |
|---|---|---|
| `/deliver-plan WithACX/eclipsa-audio-planning ...` | anywhere (takes the repo as an argument) | Writes objectives/phases/backlog and files issues in the planning repo. |
| `/deliver-code` (and `/orchestrate`, `/review`, build/test) | **this fork's clone** | Code changes, commits, and PRs happen here. It reads issues, the board, and plan docs from the planning repo via `tracker.repo` in `.a-cx/github.yaml`. |

So: plan in the planning repo, build here. `/deliver-code` never needs the
planning repo checked out -- the `tracker.repo` setting points it there over the
API.

## Releases and versions

Official releases and version tags (e.g. `v1.4.4`) live on the **Google
upstream** as GitHub Releases. This fork only mirrors those tags via git; A-CX
does not cut its own releases here.

The installer repo (`WithACX/eclipsa-audio-plugin-installers`) builds the plugin
from a git submodule pinned to **this fork's `main`** (auto-bumped by its
`update-submodule` workflow). Because `main` mirrors the upstream, the installer
tracks upstreamed/released code: a plugin change reaches a new installer build
once it is in this fork's `main`.

## Branch model

| Branch | Purpose | A-CX tooling? |
|---|---|---|
| `main` | Clean mirror of `upstream/main`. Never diverges except while syncing. | Never |
| `acx/dev` | The team's working branch. Carries the A-CX tooling as clean commits. Day-to-day work happens here (or on feature branches cut from it). | Yes |
| feature branches | Cut from `acx/dev` for internal work; PR back into `acx/dev`. | Inherit acx/dev |

Hard rule: never commit A-CX tooling to `main`, and never open an
upstream-bound PR from `acx/dev`.

## Remotes

- `origin` -> `WithACX/eclipsa-audio-plugin` (our fork). All internal work pushes here.
- `upstream` -> `google/eclipsa-audio-plugin`. Only clean, tooling-free PRs go here.

## Day-to-day internal work

```bash
git checkout acx/dev
git pull origin acx/dev
git checkout -b feature/my-change   # optional; or work on acx/dev directly
# ... commit ...
git push origin feature/my-change   # open a PR into acx/dev
```

`/deliver-code` and `/orchestrate` deliver to `origin` only; they never touch
`upstream`.

## Syncing new upstream changes into the fork

Do this when Google has merged new work you want:

```bash
git fetch upstream
git checkout main
git merge --ff-only upstream/main     # keep main a clean mirror
git push origin main
git checkout acx/dev
git rebase main                       # replay the tooling on top of new upstream
git push --force-with-lease origin acx/dev
```

If the rebase touches a conflict, resolve it in the tooling commits (product
code came from upstream and should win).

## Contributing a change back to upstream

Use the deliberate, opt-in command. It is never run automatically.

```
/upstream-pr --branch <name>                 # contribute HEAD's commits
/upstream-pr --branch <name> --source <ref>  # contribute a specific ref
/upstream-pr --branch <name> --dry-run        # print the plan, change nothing
```

It runs `.a-cx/bin/acx-upstream-pr.sh`, which: fast-forwards `main` from
`upstream`, cuts a fresh branch from `main`, replays your product commits onto
it, runs the pre-push guard as a backstop, pushes to `origin`, and opens the PR
against `upstream`. It fails closed (pushes nothing) on a dirty tree, a
conflict, or a guarded A-CX path in the diff. Do not work around a block with
`--no-verify` or a manual push to `upstream`.

## How A-CX tooling is kept out of the upstream diff

Two controls, primary first:

1. **Clean branch topology (primary).** Upstream PRs are cut from `main`, which
   never contains tooling commits. Those commits live only on `acx/dev`, so they
   are structurally absent from any diff cut off `main`. This is what actually
   keeps the diff clean; it is not a filter applied at PR time.
2. **Pre-push guard (backstop).** A local `.git/hooks/pre-push` hook blocks a
   push to the `upstream` remote whose diff touches a guarded A-CX path
   (`.a-cx/`, `.claude/`, `.github/ISSUE_TEMPLATE/`, `AGENTS.md`, `CLAUDE.md`,
   and named A-CX workflow files). It is passive on pushes to `origin`. On this
   repo it is chained with the existing Git LFS pre-push hook; the original LFS
   hook is backed up at `.git/hooks/pre-push.lfs-backup`.

The guard is a client-side speed bump only: `--no-verify`, a push from CI, or a
merge via the GitHub UI all bypass it. It is not a secrets scanner. Normal
secrets hygiene still applies. The real boundary is control 1.

The guard is local and never committed (it lives in `.git/hooks/`). New clones
get it by running `/setup-repo`, which installs the hook and the
`acx-upstream-pr.sh` script.

## Where issues and planning live

Internal issues, backlog, and the board are in `WithACX/eclipsa-audio-planning`
(the org "Groove" board). Do not open A-CX planning issues on this public fork.
See `.a-cx/docs/board-workflow.md` for the board's status flow.
