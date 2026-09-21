---
id: cmake-ctest-cycle
trigger: "when validating changes or investigating test failures in C++ code"
confidence: 0.80
domain: workflow
scope: project
date: 2026-09-21
---

# Build incrementally with cmake --target, then run filtered ctest

## Action
Use `cmake --build ./build --target <test_target> -j 8` to compile only affected tests quickly, then run `ctest -R "<pattern>"` to run only relevant test suites rather than the full test suite.

## Evidence
- Observed 5 times in session fc88928f-8d1c-464f-a393-6659bdfcf77c (events 1, 7, 17, 21, 23)
- Pattern: Incremental build with specific target (`eclipsa_tests`) followed by filtered ctest with regex patterns
- Example flow:
  1. `cmake --build ./build --target eclipsa_tests -j 8`
  2. `ctest -R "ElevationClamp|PannerInput|ElevationGeometry" --output-on-failure`
- Used for: rapid iteration when fixing defects, avoiding full test suite overhead, validating specific subsystems
- Enables fast feedback loop: make change → targeted build → targeted test → analyze
