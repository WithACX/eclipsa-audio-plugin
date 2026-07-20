---
id: worktree-missing-submodules
trigger: "immediately after `git worktree add` in this repo, before the first cmake configure"
confidence: 0.6
domain: process
scope: project
date: 2026-07-20
---
# A fresh `git worktree add` does not initialize this repo's submodules

## Action
Run `git submodule update --init --recursive` immediately after `git worktree
add` and before the first `cmake` configure in that worktree. `git worktree
add` checks out the working tree but does not populate submodules (`JUCE`,
`Spatial_Audio_Framework`, `protobuf`), so a bare `cmake -B ./build ...` in a
brand-new worktree fails hard: `third_party/CMakeLists.txt`'s
`add_subdirectory(Spatial_Audio_Framework)` errors because that directory has
no `CMakeLists.txt`, and `common/CMakeLists.txt`'s `juce_add_modules` is an
"Unknown CMake command" because JUCE's own CMake modules were never loaded.
Both surface together, before dependency fetching even reaches the
`third_party` layer, so they're the first configure errors you'll see in a
fresh worktree, not something introduced by later work in that worktree.

## Evidence
- Observed during the #43 delivery run (worktree
  `.a-cx/worktrees/43-smpte-dolby-speaker-labels`): a fresh
  `cmake -B ./build -DCMAKE_BUILD_TYPE=Debug -DCI_TEST=ON` failed with
  `CMake Error at third_party/CMakeLists.txt:71 (add_subdirectory): The
  source directory .../third_party/Spatial_Audio_Framework does not contain a
  CMakeLists.txt file` immediately followed by `CMake Error at
  common/CMakeLists.txt:16 (juce_add_modules): Unknown CMake command`.
  `git submodule status` showed both `third_party/JUCE` and
  `third_party/Spatial_Audio_Framework` prefixed with `-` (uninitialized).
  Running `git submodule update --init --recursive` (took ~50s, cloning both
  submodules fresh since the worktree's `.git` submodule state isn't shared
  with the primary tree's already-initialized ones) resolved both errors on
  the next configure attempt.
