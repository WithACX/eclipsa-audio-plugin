---
id: worktree-fetchcontent-isolation
trigger: "when configuring a worktree with shared FetchContent deps from primary tree"
confidence: 0.80
domain: process
scope: project
date: 2026-07-22
---
# Worktree FetchContent Isolation: Use Source Override, Not Base Dir

## Action
When reusing already-fetched dependencies in a worktree to avoid slow network downloads, use `-DFETCHCONTENT_SOURCE_DIR_<DEP>=<PATH>` for specific dependencies (e.g., LibIAMF) instead of `-DFETCHCONTENT_BASE_DIR=<PATH>` to reuse the entire cache. The base-dir approach causes build-dir contamination: generated files (like `config.h`) are referenced from the primary tree's build/_deps, not the worktree's local build.

## Evidence
- Session 0932d151, lines 33-34: attempted `-DFETCHCONTENT_BASE_DIR=/primary/build/_deps` in worktree #38 configure
- Line 66-68: resulted in fatal error `sofa_hrtf.h:4: #include "config.h" not found` — config.h was generated in primary tree's `libspatialaudio-build/config.h`, not in worktree's build dir
- Lines 69-70: fixed by narrowing to only `-DFETCHCONTENT_SOURCE_DIR_LIBIAMF=<PATH>`, letting other deps build fresh locally
- Result: worktree configure completed successfully (line 73)
- Why: CMake's FetchContent generates source-dep build outputs (config.h, etc.) in the consuming project's build dir, not shared across trees

## How to apply
When a worktree needs to skip slow network downloads of a large dependency (LibIAMF, ~200MB), find the existing source in a sibling worktree's `build/_deps/libiamf-src` and pass only that via `-DFETCHCONTENT_SOURCE_DIR_LIBIAMF=<path>` — let all other deps fetch fresh with FetchContent's git-clone mechanism (fast and safe).
