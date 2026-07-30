---
id: dead-code-finding-document-dont-default-remove
trigger: "when a review flags a function as dead/unused production code that duplicates a test-only helper"
confidence: 0.6
domain: code-style
scope: project
date: 2026-07-30
---
# Dead-code review findings: check test coverage before defaulting to removal

## Action
When a MEDIUM/LOW review finding calls out a production function with zero
production callers (often phrased as "wire a real caller, remove it, or
document it explicitly"), don't reflexively remove it. First check whether
its own regression test covers behavior (e.g. an edge case, an algorithm
choice) not exercised anywhere else. If removing the function would also
remove the only test coverage for that behavior, and re-wiring a caller
would reintroduce a problem a prior commit deliberately fixed (e.g. a
redundant second file parse), the correct fix is the third option: document
in the header/declaration comment, explicitly and specifically, why the
function has no production caller by design and why it's still kept/tested.
Cross-reference from the sole caller (the test) back to that doc comment so
a future reader doesn't rediscover the same "looks like dead code" concern.

## Evidence
- PR #17 (`eclipsa-audio-plugin`), self-review MEDIUM finding on
  `IAMFExportHelper::getMediaDurationSeconds()`: a prior commit (b1f2cd3)
  had already moved the feature's real duration source to
  `muxIAMF()`'s `outVideoDurationSec` out-param specifically to avoid a
  second, independent GPAC parse of the video file -- re-wiring
  `getMediaDurationSeconds()` as a caller would have undone that exact
  optimization. Its own test (`get_media_duration_uses_video_track_not_container`)
  was the only coverage of the video-track-vs-container-duration
  preference logic, so removing the function would have silently dropped
  that edge-case coverage. Documented it as an intentional, tested,
  general-purpose utility instead.

## Related
- [[empirical_verification_refutes_theoretical_bugs]] -- same spirit: check
  the actual test/behavior evidence before acting on a review finding's
  suggested fix.
