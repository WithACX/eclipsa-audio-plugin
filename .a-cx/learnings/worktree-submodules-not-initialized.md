---
id: worktree-submodules-not-initialized
trigger: "when running `git worktree add` in this repo and then configuring CMake in the new worktree"
confidence: 0.5
domain: git
scope: project
date: 2026-07-15
---
# `git worktree add` does not initialize submodules

## Action
After `git worktree add -b <branch> <path>`, run `git submodule update --init
--recursive` in the new worktree BEFORE running `cmake -B ./build ...`.
`third_party/JUCE` and `third_party/Spatial_Audio_Framework` (and likely other
submodules) come up uninitialized in a fresh worktree even though they're
already checked out in the primary tree. CMake configure fails on this with
misleading errors that don't mention submodules at all: `Unknown CMake command
"juce_add_modules"` and `add_subdirectory` given a source directory with no
`CMakeLists.txt` for SAF. `git submodule status` shows a `-` prefix on affected
submodules when this is the cause.

## Evidence
- Observed once during the #42 delivery run (worktree
  `.a-cx/worktrees/42-general-ui-cleanup`): fresh `git worktree add` left both
  JUCE and Spatial_Audio_Framework uninitialized; `git submodule status`
  showed `-<sha> third_party/JUCE` / `-<sha> third_party/Spatial_Audio_Framework`
  until `git submodule update --init --recursive` was run.
