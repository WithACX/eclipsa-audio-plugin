---
id: worktree-vst3-build-changed-saf-checksum
trigger: "when forcing SAF_PERFORMANCE_LIB (or any other toolchain-owned cache variable) directly via -D to patch around a missing platform toolchain, rather than reconfiguring from a fresh build directory"
confidence: 0.5
domain: testing
scope: project
date: 2026-07-16
status: issue-raised
issue: "GH-46"
---
# Forcing SAF's Accelerate backend onto an existing build recompiles it and can break exact-checksum tests

## Action
`SAF_PERFORMANCE_LIB` controls which math backend (Intel MKL vs Apple
Accelerate vs others) the Spatial_Audio_Framework third-party library
compiles against, and it is a compile definition on that library, not just a
link choice. Changing it on an existing build directory forces a real
recompile of SAF and anything statically linked to it (all plugin formats
share one `SharedCode` static library), which can change the exact
floating-point output of the audio/export pipeline -- enough to break a test
that asserts an exact SHA-256 checksum of exported file bytes
(`FileOutputTests.validate_file_checksum`). Don't assume patching a single
toolchain-owned cache variable is a no-op; if an exact-output test starts
failing right after such a patch, suspect the numeric backend change first,
not the code under test.

## Evidence
- Observed during the #41 delivery run (worktree
  `.a-cx/worktrees/41-export-fails-silently`, see
  [[worktree-build-missing-toolchain]]): forcing
  `-DSAF_PERFORMANCE_LIB=SAF_USE_APPLE_ACCELERATE` (the toolchain's intended
  value, needed to get `RendererPlugin_VST3` to configure without an Intel-MKL
  "mkl.h not found" error) triggered a real SAF recompile. Immediately after,
  `FileOutputTests.validate_file_checksum` failed on a checksum mismatch, and
  `LoggerTest.LoggerInitMultipleCalls` also failed -- both verified via a
  direct, isolated `--gtest_filter` re-run (not just noise from running the
  whole suite in one process). Confirmed via the same run's earlier
  checkpoints that all 244 tests, including this exact checksum test, passed
  cleanly before this specific cache change. Filed as
  WithACX/eclipsa-audio-planning#46 to investigate whether the reference
  checksum needs updating for the intended (Accelerate) backend, or whether
  something else about the ad-hoc cache patch diverges from what the real
  toolchain file would produce.
