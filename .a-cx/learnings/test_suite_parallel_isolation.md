---
id: test-suite-parallel-isolation
trigger: "when running ctest with -j4 or -j8 flags"
confidence: 0.75
domain: testing
scope: project
date: 2026-07-22
---
# Test Suite Parallel Isolation Issue

## Action
Run the test suite with `ctest` (serial, no -j flag) instead of parallel variants. Parallel runs produce ~20+ false failures from fixed test-output filenames colliding across processes; serial runs pass cleanly (257/257 in this session).

## Evidence
- Session 0932d151: `ctest -j4` produced 25 FAILED tests (FileOutputTests, IAMFBufferedReaderTests, EBU128 tests) in lines 89-94
- Same session: serial `ctest` (no -j flag) achieved 100% pass rate with 257/257 tests passing (line 106)
- Pattern: All parallel failures were pre-existing test-isolation flakes, not regressions from code changes
- Noted as a known issue but not critical for pre-PR validation (serial run is the authoritative signal)
