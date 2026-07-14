---
id: worktree-lfs-false-positive-diffs
trigger: "when running `git worktree add` in this repo and then `git status`/`git diff` before staging changes"
confidence: 0.5
domain: git
scope: project
date: 2026-07-14
---
# git worktree checkouts can show LFS-tracked binaries as falsely modified

## Action
After `git worktree add`, do not `git add -A` or trust a bare `git status`
listing at face value. Several LFS-tracked binaries under `third_party/`
(`*.a`, `*.dll`, `*.dylib`) showed up as modified in the new worktree even
though no one touched them. `git lfs status` confirmed the Git object hash
and the working-file hash were identical ("Git: X -> File: X"), so the
content was unchanged -- the LFS smudge/clean filter just produced a
different byte-for-byte re-serialization on that checkout. Stage only the
files you actually intended to change (`git add <path> <path> ...`), and
cross-check any unexpected binary diff with `git lfs status` before
including it in a commit.

## Evidence
- Observed once during the #44 delivery run (worktree
  `.a-cx/worktrees/44-wav-checkbox-default-unchecked`): 6 third_party
  binaries appeared modified after `git worktree add`; `git lfs status`
  showed matching hashes for all of them.
