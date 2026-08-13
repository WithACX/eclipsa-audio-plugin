---
id: boundary_test_pair_for_numeric_constants
trigger: "when widening or changing a numeric tolerance/threshold constant in code"
confidence: 0.82
domain: testing
scope: project
date: 2026-08-06
---
# Add paired boundary tests when changing numeric constants

## Action
When changing a numeric constant (e.g., a tolerance, threshold, or bound), add at least two new boundary tests: one that sits just-inside the new bound (to prove the old value would fail here), and one just-outside (to prove the constant still enforces the limit at the edge). This guards against silent reversions and documents the intent of the constant.

## Evidence
- Observed 2026-08-06 in PR #18 / branch acx/pin-duration-mismatch-tolerance-boundary:
  - `kDurationMismatchToleranceSec` was widened from 0.05s to 0.5s (line 49-50 of observer log)
  - Added `mux_no_mismatch_within_widened_tolerance` (just-inside: 0.3s drift, inside new 0.5s tolerance, outside old 0.05s tolerance)
  - Added `mux_flags_mismatch_just_beyond_widened_tolerance` (just-outside: 0.7s drift, well beyond new 0.5s tolerance)
  - Test comment explicitly states: "so this test would fail if the constant were ever reverted to 0.05"
- Pattern: the test name and assertion clearly documents WHY the boundary matters and what code change it would catch

## Related
- [[feedback_build_test_before_pr]] — validate tests pass locally and in tree-wide format check before PR
- [[comment_each_unit_test]] — document what each test verifies, especially for boundary cases
