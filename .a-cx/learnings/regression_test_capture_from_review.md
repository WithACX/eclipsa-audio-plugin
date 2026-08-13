---
id: regression-test-capture-from-review
trigger: "when code review identifies missing test coverage for a scenario, add a regression test to the same diff"
confidence: 0.7
domain: testing
scope: project
date: 2026-07-27
---
# Regression Test Capture: Review Findings → Test Cases

## Action
When code review or verification reveals a scenario that \"nothing previously exercised,\" immediately add a regression test for that scenario to prevent the same gap from reappearing. Include a comment explaining what the test guards against.

## Evidence
- PR #17 added checkAudioVideoDurationMismatch() which runs after muxing succeeds/fails
- Code comment documented: \"runs unconditionally... relies on FileExport::recordExportErrorIfUnset's first-recorded-error-wins semantics\"
- Review finding: No test existed covering case where mux failure AND genuine duration mismatch both fire — unclear which error would be reported
- Event 54: Added `TEST_F(FileOutputTests, mux_failure_takes_priority_over_duration_mismatch)` with detailed comment explaining what it guards against
- Test pattern: Invalid video output path (forces mux failure) + short audio render (would trigger duration mismatch alone) → confirm mux error wins

## How to apply
1. After code review, scan findings for phrases like \"nothing previously exercised\" or \"no test exists for\"
2. Design a minimal test case that reproduces that exact scenario
3. Document in the test comment WHY it matters (what would break if error priority changed)
4. Include the finding's own language in the comment (helps future reviewers understand context)

## Related
- [[feedback_comment_each_unit_test]] — one-line comment above every test explaining what it verifies
- [[test_suite_parallel_isolation]] — tests in this project use isolated worktrees
