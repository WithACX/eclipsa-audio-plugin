# Fork workflow (A-CX internal)

Internal A-CX process doc. Lives under `.a-cx/` so the pre-push guard and the
branch topology keep it out of any upstream-bound diff. Never move it out of
`.a-cx/` on this fork.

This repo is a fork of `google/eclipsa-audio-plugin`. A-CX develops here and
contributes selected changes back upstream. The rules below keep A-CX tooling
(`.a-cx/`, `.claude/`, `AGENTS.md`, `CLAUDE.md`) out of anything Google sees.

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
