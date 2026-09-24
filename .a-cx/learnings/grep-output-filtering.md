---
id: grep-output-filtering
trigger: "when analyzing build or test output from cmake/ctest"
confidence: 0.85
domain: workflow
scope: project
date: 2026-09-21
---

# Filter build and test output with grep

## Action
Pipe cmake/ctest output through `grep -E` with patterns for errors, failures, and test status to isolate relevant lines rather than reading full logs.

## Evidence
- Observed 10+ times in session fc88928f-8d1c-464f-a393-6659bdfcf77c
- Pattern: After every cmake build, user runs `grep -E "error:|FAILED"` on build logs
- After ctest runs, user filters with patterns like `grep -E "\*\*\*Failed|\*\*\*Exception|tests passed|tests failed"`
- Example: `cmake --build ./build --target eclipsa_tests -j 8 > build_tests.log 2>&1; grep -E "error:|FAILED" build_tests.log | head -20`
- Used for: quickly identifying which tests failed, whether build succeeded, finding specific error lines in verbose output
