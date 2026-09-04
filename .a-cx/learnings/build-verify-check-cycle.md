---
id: build-verify-check-cycle
trigger: "when verifying code changes on a C++ project with CMake and ctest"
confidence: 0.85
domain: testing
scope: project
date: 2026-09-03
status: observed
---

# Build-Verify-Check Cycle Workflow

## Action
Verify code changes by repeating: build with cmake, run full ctest suite, poll results until completion. Log output to scratchpad for later inspection. Do not rely on console output for verification -- grep key patterns from saved logs.

## Evidence
Observed 4 times in session 3fad9120-e811-45f1-925f-06502d2a6c54:
- Initial build check: all 332 tests pass (events 1-2)
- Background rebuild with logging to cmake_build_59c.log and ctest_59c.log (events 7-8)
- Polling for status by grepping CTEST_EXIT from log file (events 11-12)
- Final verification: confirmed 100% pass rate (event 30)

## Pattern
The project has a large test suite (332 tests). Backgrounding the build allows parallel work. Saving logs to scratchpad prevents terminal scrollback loss. Grepping for `CTEST_EXIT=0` and `tests passed` is reliable across reruns.

## Reuse Pattern
```bash
mkdir -p "$SP"
cmake --build build --config Release -j 4 > "$SP/cmake_build.log" 2>&1
BE=$?
if [ $BE -eq 0 ]; then
  (cd build && ctest -C Release > "$SP/ctest.log" 2>&1; echo "CTEST_EXIT=$?" >> "$SP/ctest.log")
fi
```

Verify with:
```bash
grep -E "CTEST_EXIT=0|tests passed" "$SP/ctest.log"
```
