---
id: fresh_worktree_submodule_state_recovery
trigger: "when a fresh git worktree's submodules have staged deletions or missing content"
confidence: 0.80
domain: git
scope: project
date: 2026-08-06
---
# Reset submodule state and re-initialize when worktree shows staged deletions

## Action
After creating a fresh git worktree and running `git submodule update --init --recursive`, if `git submodule status` shows missing/empty directories (e.g., `third_party/JUCE`) or if `git status` shows staged deletions in a submodule (e.g., `deleted: .clang-tidy`, `deleted: .github/...`), run `git reset --hard HEAD` inside the submodule directory, then retry `git submodule update --init --recursive` from the worktree root. This cleans up orphaned/partial checkout state.

## Evidence
- Observed 2026-08-06 in session 4082aef7 (lines 40-66):
  - After `git worktree add` and initial `git submodule update --init --recursive` (line 40), `git submodule status` showed negative commit hashes (` -2c75900...`, ` -018e06e...`) indicating uninitialized/detached state
  - Submodule checkout then failed: `git lfs pull` timed out; second attempt failed with "network unreachable"
  - Third attempt (line 45-46) succeeded when `git submodule update --init --recursive` was retried
  - Later inspection (line 64-65) found submodule `third_party/JUCE` had staged deletions: `deleted: .clang-tidy`, `.github/ISSUE_TEMPLATE/...`
  - Running `git reset --hard HEAD` in the submodule (line 66), then retrying the submodule init (line 66) finally resolved the state
  - After reset, `third_party/JUCE/extras` directory appeared, which CMake then found successfully (line 69)
- Pattern: partial/stale submodule checkouts cause CMake configure to fail with "Unknown CMake command juce_add_modules" until the full contents are present

## Related
- [[eclipsa_fresh_worktree_build_prereqs]] — the full three-step fresh worktree setup (submodules, toolchain, LFS)
- [[lfs_pointer_binary_mismatch_recovery]] — LFS file state issues that arise alongside submodule problems
