---
id: lfs-pointer-file-changes-in-worktree
trigger: "when git status shows modified binary files from third_party/"
confidence: 0.75
domain: git
scope: project
date: 2026-07-22
---
# LFS-tracked binary files show as modified after branch merge

## Action
Ignore LFS pointer file changes in git status (files like `third_party/*/lib/*.dll`, `.dylib`, `.a`). They are expected when merging branches with different LFS snapshots. Do NOT stage or commit these changes — they are working-directory artifacts, not source code. Run `git checkout -- <file>` to restore them if needed before committing.

## Evidence
- Observed in PR #17: After merging `acx/dev` into `38-audio-video-duration-mismatch-warning`, git status showed 7 modified binary files in third_party/
- Files: third_party/libiamf/lib/Windows/Debug/iamf_tools.dll, third_party/libiamf/lib/macos/libFLAC.a, libfdk-aac.a, libogg.a, libopus.a, third_party/obr/lib/Windows/Release/obr.dll, third_party/obr/lib/obr.dylib
- Each file is marked with `filter: lfs` (confirmed via git check-attr)
- CI logs show \\\"Encountered 16 files that should have been pointers, but weren't\\\" during checkout — standard LFS warning for pointer files
- Pattern: The differences are size changes from actual binary (e.g., 4.6MB → 132 bytes for obr.dylib), indicating LFS pointers not actual blobs

## Related
- [[always_use_a_worktree]] — LFS state is isolated per worktree; this pattern is safe as-is
