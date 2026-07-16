---
id: fresh-worktree-full-deps-build-time
trigger: "when starting the first build in a newly created delivery worktree in this repo"
confidence: 0.5
domain: process
scope: project
date: 2026-07-15
---
# A new worktree's first build still compiles the full _deps tree, even with a cloned cache

## Action
Budget roughly 15-20 minutes (observed) for the first `cmake --build` in a
freshly created delivery worktree, even when an implementing agent seeds
`build/_deps` from an existing configured build (primary tree or a sibling
worktree). CMake's generated build files embed absolute, worktree-specific
paths, so a cloned `_deps` cache does not make the configure/build a no-op --
large third-party targets (protobuf, abseil, upb) still recompile from
source. Don't assume a build is hung because it's taking a while on a fresh
worktree; check `ps aux` for active compiler processes before treating it as
stuck.

## Evidence
- Observed during the #41 delivery run (worktree
  `.a-cx/worktrees/41-export-fails-silently`): the implementing subagent
  cloned `build/_deps` (~7.9GB) from the primary tree's configured build, but
  the subsequent `cmake --build build --target eclipsa_tests -j8` still
  actively compiled protobuf/upb/abseil sources for several minutes before
  reaching the repo's own targets.
