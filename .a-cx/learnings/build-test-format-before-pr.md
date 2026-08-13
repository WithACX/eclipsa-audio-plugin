---
id: build-test-format-before-pr
trigger: "before opening a PR for any code changes"
confidence: 0.95
domain: process
scope: project
date: 2026-07-21
---
# Build, Test, Format Before PR

## Action
Always run: clang-format on all changed .cpp/.h files, a full cmake configure with -DCI_TEST=ON, cmake build, and ctest — before opening a PR.

## Evidence
- Documented in AGENTS.md lines 130–138 (upstream-bound PR requirements)
- Observed repeatedly in session e5d2fc75-c97e-4f3d-b9fd-992653b0f32c:
  - Line 133: clang-format --style=file:.clang-format -i on changed files
  - Line 135: cmake -B ./build -DCMAKE_BUILD_TYPE=Debug -DCI_TEST=ON
  - Line 141–145: Full cmake build + ctest pipeline for issue #43 worktree
- Pattern repeated for worktrees 42, 43, 47
- AGENTS.md explicitly states: "an upstream-bound PR that only ran ctest locally and skipped formatting"—flagged as a prior failure mode

## Why
Skipping any step (clang-format, build, or ctest) allows trivial failures into the PR. CI enforces clang-format with -Werror, so violations block the PR. Incomplete local testing wastes review time on obvious bugs. The build/_deps tree is large (6–9GB) and reused across worktrees; a fresh configure+build in a worktree is the right pattern, not a shortcut skip.

## How to apply
Before `git push` for any PR: (1) run clang-format on changed files; (2) if in a worktree, cmake configure with -DCI_TEST=ON; (3) cmake --build; (4) ctest (or just the affected test subset if full suite is very long, but note that in CMake log).
