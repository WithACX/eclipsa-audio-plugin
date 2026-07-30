---
id: gating-behavior-change-invalidates-test-comments
trigger: "when adding a guard/condition around a call that a regression test's comment describes as running unconditionally"
confidence: 0.6
domain: testing
scope: project
date: 2026-07-30
---
# Gating a previously-unconditional call can leave a passing test's comment describing stale behavior

## Action
When a review finding leads to gating a call that was previously run
unconditionally (e.g. adding `if (successFlag) { helperThatParsesUntrustedInput(); }`
around a call a comment says "runs unconditionally after X"), search for and
update any test comment that documents the old unconditional behavior in the
same commit -- even if the test's assertions still pass unchanged. The test
outcome (e.g. "the higher-priority error wins") can remain correct under both
the old and new code paths, so CI gives no signal that the comment is now
describing a mechanism the code no longer uses.

## Evidence
- `FileOutputTests.mux_failure_takes_priority_over_duration_mismatch` in
  `common/processors/tests/FileOutputProcessor_test.cpp` asserted
  `kMuxFailed` wins over a genuine duration mismatch, with a comment
  attributing this to "checkAudioVideoDurationMismatch runs unconditionally
  ... relies on first-recorded-error-wins semantics." Fixing PR #17's MEDIUM
  finding ("duration check expands untrusted-media parsing surface") changed
  `FileOutputProcessor::closeFileExport` to skip the call entirely on mux
  failure -- the test still passed with zero code changes needed, but the
  comment would have shipped describing a mechanism that no longer existed.

## Related
- [[regression_test_capture_from_review]]
