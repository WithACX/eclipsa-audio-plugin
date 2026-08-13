---
id: lfs_pointer_binary_mismatch_recovery
trigger: "when a git worktree shows modified LFS-tracked binary files after creation"
confidence: 0.85
domain: git
scope: project
date: 2026-07-30
---
# LFS pointer vs. actual content mismatch in worktrees requires manual recovery

## Action
When a freshly-created worktree shows modified LFS-tracked files (e.g., third-party binaries in `third_party/`) as dirty despite no local changes, the smudge filter and working-tree content are out of sync. Run `git lfs pull` in the worktree to fetch the real content, then restore exact HEAD blob content for all mismatched files via `git cat-file -p HEAD:<path> > <path>` (bypasses the smudge filter). Finally, mark them `assume-unchanged` to hide the filter/content divergence (mirrors the primary tree's clean state).

## Evidence
- Observed 2026-07-30 when creating upstream-pr-17-dev worktree to contribute PR #17 upstream: `git status --porcelain` showed `M` on 8 third-party LFS files (`libear.a`, multiple libiamf libs, obr.dll, obr.dylib). Files were actually unchanged on disk; mismatch was pointer-file-to-binary divergence after worktree creation.
- Root cause: the `.git/lfs/objects/` media store and `.git/worktrees/upstream-pr-17-dev/.git/lfs/` or smudge-filter state diverged when the worktree was set up without synchronizing LFS content.
- Recovery: `git lfs pull 2>&1` fetched actual blobs, then `git cat-file -p HEAD:<path>` restored each modified file's exact blob hash (12a266625de4... in this case), bypassing any filter misapply. `git update-index --assume-unchanged` on each finally cleared the status.
- Alternative (if lfs pull times out): `git cat-file --batch` over the full range, then `git update-index --assume-unchanged` for all affected files, is faster.
