---
id: lsp-diagnostics-vs-compile-truth
trigger: "when LSP reports undefined symbols or missing declarations in room_views files"
confidence: 0.82
domain: testing
scope: project
date: 2026-08-13
---

# LSP Diagnostics vs. Compile Truth: Room Views Protobuf Headers

## Action
When the editor LSP reports undeclared symbols in room_views code, dismiss the diagnostic if local compile and ctest both succeed. Do not block PR or add defensive includes based on LSP alone.

## Evidence
- Observed 2 times in same project session (aut-20260813-56 and aut-20260813-55):
  - LSP flagged undeclared IconStore, facingSurface, anchorsToPath in PerspectiveRoomViews.cpp
  - Unit's own compile: `eclipsa_tests` target built exit 0, 290/290 tests passed (+17 new tests from this branch)
  - Root cause: generated protobuf headers absent from the LSP index but present at compile time
  - Pattern identical in both runs; classified as "clangd cascade"
- Verification: grep confirmed anchorsToPath is declared in ElevationSurfaces.h (included line 20 of .cpp)
- Not a code defect; LSP index lag

## Why
This codebase uses protobuf code generation and unity-build compilation (components.cpp includes 20+ sources). CMake generates protobuf headers at configure time, but the LSP server (clangd) may not have rebuilt its index if:
- The build/ directory was cleaned between configuration and editor start
- A submodule was updated and `git lfs pull` was not run after `git submodule update`
- The LSP session started before CMake finished code generation

The compile-time environment has the real headers; LSP is stale.

## How to apply
1. When LSP reports "undeclared identifier" in a room_views or components file:
   - Do NOT add includes based on the LSP error alone
   - Run `ctest` in the worktree: if all tests pass, LSP is stale
   - Run `cmake --build build --target <component>` for the affected module
   - If compile succeeds, confirm the symbol is declared in the codebase via grep:
     ```bash
     grep -n "anchorsToPath" common/components/src/room_views/*.h
     ```
2. If grep finds the symbol and compile succeeds, add a comment in the PR body:
   ```
   LSP false positive: undeclared anchorsToPath.
   Root cause: clangd index stale; symbol is in ElevationSurfaces.h line NN.
   Compile verified: eclipsa_tests exit 0, 290/290 tests pass.
   ```
3. Close the diagnostic by restarting the editor's LSP server (usually Cmd+Shift+P → "LSP: Restart" in VS Code)

## Related
- [[empirical-verification-refutes-theoretical-bugs]] (parent learning: use testing to validate claims)
- [[Fresh worktree build prereqs]] (setup: submodule init + lfs pull prevents this root cause)
