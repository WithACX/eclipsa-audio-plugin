---
id: file-output-tests-not-parallel-safe
trigger: "when running ctest on this project"
confidence: 0.85
domain: testing
scope: project
date: 2026-08-06
---

# FileOutputTests are not parallel-safe

## Action
Always run `ctest` without the `-j` parallelization flag; run serially with `ctest --output-on-failure` or plain `ctest`.

## Evidence

- Observed 2 consecutive runs in session 4082aef7-b80e-4d0b-b369-b342ae66b3a9:
  - `ctest -j 4` → **14 test failures** in FileOutputTests and MP4IAMFDemuxerTest (line 23–24, 25–26 partial)
  - `ctest` (serial) → **all 267 tests pass** (line 25–26 final)
- PR #19 commit body (line 36) explicitly documents: "this suite's `FileOutputTests` share working-directory files and are not parallel-safe -- run without `-j`"
- Failures observed include segfault and fixture/output-file conflicts typical of concurrent test isolation issues

## Why

FileOutputTests write to shared temporary files (audio/video/muxed output artifacts) that interfere when tests run concurrently. The codebase does not isolate per-test working directories.

## How to apply

**Before committing or submitting a PR**, run the full test suite as:
```bash
ctest --output-on-failure  # or: cd build && ctest --output-on-failure
```
Do NOT use `-j` or `--parallel` flags. A serial run takes ~4–5 minutes but guarantees real pass/fail status.

If you run `ctest -j N` and see spurious failures in FileOutput or MP4IAMF tests, re-run serially to confirm they are real before filing a regression.
