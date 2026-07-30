---
id: ctest-parallel-shared-fixed-filenames
trigger: "when running the full test suite in this repo before opening a PR"
confidence: 0.6
domain: testing
scope: project
date: 2026-07-21
---
# Running `ctest -jN` produces false failures from shared fixed test-output filenames

## Action
Run `ctest` from `build/` with NO `-j` flag when validating a change in this
repo, matching the invocation AGENTS.md documents -- do not add parallelism
as an optimization. Every gtest test in this repo compiles into one combined
`eclipsa_tests` binary (`cmake/eclipsa_build_tests.cmake`); `gtest_discover_tests`
then registers each `TEST`/`TEST_F` as its own ctest entry, so `ctest -jN`
runs N of these as separate OS processes concurrently. Several fixtures
(`FileOutputTests` at minimum -- `iamfOutPath`/`videoOutPath` are
`std::filesystem::current_path() / "test.iamf"` / `"test.mp4"`, a fixed name
with no per-test uniqueness) read and write files at a FIXED path under the
shared build directory. Two tests from that fixture racing in parallel step
on each other's output files, producing failures (`readFrame` returning 0,
wrong duration, etc.) that look like real regressions in whichever code the
failing tests touch -- including tests that pass cleanly in isolation. A
`-j4` full run surfaced ~30 failures spanning `FileOutputTests`,
`MP4IAMFDemuxerTest`, `IAMFFileReaderTest`, and even an unrelated
`LoggerTest`; the same suite run serially (`ctest` with no flag) passed
100% (257/257). Don't trust a parallel ctest failure list without first
re-running serially to rule this out.

## Evidence
- Observed during the #38 delivery run (worktree
  `.a-cx/worktrees/38-audio-video-duration-mismatch-warning`): `ctest -j4
  --output-on-failure` reported ~30 failed tests, including this session's
  own new `FileOutputTests.mux_no_mismatch_when_audio_covers_video_duration`
  and several tests already passing individually
  (`FileOutputTests.iamf_lpc_1ae_1mp`, `.iamf_lpc_1ae_1mp_expl`). Re-running
  the identical build with plain `ctest --output-on-failure` (no `-j`)
  passed 257/257 with zero failures, confirming the parallel run's failures
  were a test-isolation artifact rather than a code regression.
