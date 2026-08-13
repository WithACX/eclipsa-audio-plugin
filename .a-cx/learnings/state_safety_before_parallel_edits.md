---
id: state-safety-before-parallel-edits
trigger: "when about to make parallel changes to multiple files, snapshot git state before and after"
confidence: 0.85
domain: git
scope: project
date: 2026-07-27
---
# State Safety Snapshots for Parallel File Edits

## Action
Before executing parallel Edit operations on multiple files (especially across different directories), explicitly check `git status` and `git worktree list` to confirm isolation and capture baseline state. After edits complete, re-run git status to verify no hidden mutations occurred.

## Evidence
- PR #17 session made parallel edits to 4 different files:
  - WavFileOutputProcessor.h:124-125 (widening fields)
  - WavFileOutputProcessor.cpp:87 (widening local variable)
  - LoudnessExportProcessor.h:81-83 (widening fields)
  - LoudnessExportProcessor.cpp:268-270 (widening local variables)
- Event 9: Checked git worktree list to confirm 7 isolated worktrees before edits
- Event 11: Re-ran git status in the worktree to capture baseline (7 modified binary LFS files, expected)
- Pattern: Parallel edits are safe ONLY when git state is isolated and snapshot shows no surprises

## How to apply
1. Before parallel Edit calls: run `git worktree list` and `git status --porcelain=v1 --untracked-files=all`
2. Expected state: current worktree, no uncommitted non-binary changes (LFS binaries OK)
3. After edits: re-run git status to confirm no ghost mutations from race conditions
4. Document unexpected files in memory before editing (see [[lfs_pointer_files_in_worktree]])

## Related
- [[feedback_worktree_isolation_always]] — always use a worktree, even lightweight fixes
- [[lfs_pointer_files_in_worktree]] — LFS pointer changes are expected and safe in worktrees
