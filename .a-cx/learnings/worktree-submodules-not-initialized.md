---
id: worktree-submodules-not-initialized
trigger: "when running `cmake -B ./build` for the first time in a freshly created delivery worktree in this repo"
confidence: 0.6
domain: process
scope: project
date: 2026-07-21
---
# `git worktree add` does not initialize this repo's submodules

## Action
Before the first CMake configure in a new worktree, run
`git submodule status` and check for a `-` prefix (uninitialized). This repo
vendors `third_party/JUCE` and `third_party/Spatial_Audio_Framework` as git
submodules; `git worktree add` checks out the worktree's tree but does not
run `git submodule update --init --recursive` the way a fresh `git clone`
(with `--recurse-submodules`) or an existing checkout's `git pull` would. A
configure against uninitialized submodules does not fail immediately -- it
gets partway through (fetching unrelated FetchContent deps, resolving other
third_party targets) before failing on `add_subdirectory(...)` finding no
`CMakeLists.txt`, or on a missing CMake command like `juce_add_modules`
because JUCE's own CMake modules were never loaded. Run
`git submodule update --init --recursive` immediately after `git worktree
add`, before the first configure, rather than only reacting to the
downstream error.

## Evidence
- Observed during the #38 delivery run (worktree
  `.a-cx/worktrees/38-audio-video-duration-mismatch-warning`): a fresh
  worktree's first `cmake -B ./build` configure got through most of
  `third_party/CMakeLists.txt` (FetchContent for zeromq, libiamf, etc. all
  fetched fine) before failing with `add_subdirectory` unable to find
  `third_party/Spatial_Audio_Framework/CMakeLists.txt`, followed by
  `Unknown CMake command "juce_add_modules"` from `common/CMakeLists.txt`.
  `git submodule status` showed both `third_party/JUCE` and
  `third_party/Spatial_Audio_Framework` prefixed with `-` (never
  initialized in that worktree checkout). Running
  `git submodule update --init --recursive` and reconfiguring resolved both
  errors.
