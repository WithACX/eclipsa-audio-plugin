---
id: verify-mutations-immediately
trigger: "after any GitHub state mutation (close, assign, comment, board update)"
confidence: 0.85
domain: process
scope: org-wide
date: 2026-09-04
---
# Verify State Mutations Immediately After Completion

## Action
After every GitHub state mutation, immediately readback and verify the operation succeeded before proceeding to dependent operations. Do not assume the mutation succeeded; GitHub API can fail silently or partially.

## Evidence
- Observed 5 verification patterns in session 3fad9120-e811-45f1-925f-06502d2a6c54
- Pattern: Every mutation is followed by a readback check
  - Event 6: After merge verification, checked ancestry with `git merge-base --is-ancestor` on three commits
  - Event 16: After ticking criteria, grepped the result to confirm `ticked: 10 unticked: 0`
  - Event 20: After assign, verified `$me` was in the assignees list
  - Event 22: After close + board update, verified state and board status in readback
  - Event 24: After close, re-checked parent close predicate and sibling blocker counts
- Catch rate: Prevents silent failures where the API returns 200 but the mutation didn't land (e.g., permission, race condition, or validation edge cases)
