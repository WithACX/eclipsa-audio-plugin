---
id: review-findings-empirical-verification-workflow
trigger: "when addressing review findings for production code quality issues"
confidence: 0.8
domain: code-style
scope: project
date: 2026-07-30
---
# Fix Review Findings Through Empirical Verification

## Action
When addressing review findings about stale comments, unreachable code, or inconsistent patterns, verify the actual behavior with regression tests and grep analysis before accepting the finding at face value.

## Evidence
- Session 8797d38c: PR #17 review fixes (3+ separate findings corrected this way)
  - **Stale comment claim**: Review said `framesWritten_ == 0` would flag `kVideoLongerThanAudio`. Developer added regression test (`mux_zero_frame_export_fails_mux_not_silently_skips`) that proved zero-frame exports always fail the mux outright with `kMuxFailed` — the function can never be reached with that condition. Fixed by rewording comment as a hypothetical defense-in-depth note.
  - **Orphaned function**: Review flagged `getMediaDurationSeconds()` as test-only with zero production callers. Developer verified with grep that it was only called by its own test, then refactored to thread the video duration through the `muxIAMF()` return value instead.
  - **Inconsistent logging**: Review found the mismatch-check function was the only `recordExportErrorIfUnset` site without a corresponding `LOG_WARNING`/`LOG_ERROR`. Developer added logging to match the pattern used everywhere else in the file.

## Pattern
When a review finding claims a code path is unreachable, a function is unused, or a comment doesn't match behavior, the fix is not just to reword — it's to verify empirically: 
- Use grep to confirm reference count across the codebase
- Add a regression test that exercises the claimed scenario and observe what actually happens
- Let the test output (pass/fail, expected vs. actual) guide whether the comment is just stale or whether the code path is truly unreachable

A finding that seems like "impossible to reach" often turns out to be genuinely unreachable once you add the test and see the assertion fail.
