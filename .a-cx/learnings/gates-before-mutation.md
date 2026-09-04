---
id: gates-before-mutation
trigger: "before closing an issue or mutating parent/sibling issue state"
confidence: 0.87
domain: process
scope: project
date: 2026-09-04
---
# Run Sequential Verification Gates Before State Mutations

## Action
Before closing an issue or updating shared state (parent/sibling issues, board), run a sequence of verification gates to confirm preconditions. Gates should check: open blockers, sub-issues, assignment, criteria completion, precedent (how siblings were closed), and board readiness.

## Evidence
- Observed 4 gate sequences in session 3fad9120-e811-45f1-925f-06502d2a6c54
- Pattern: Each gate validates a precondition; gates are cumulative and blocking
  - Event 7: Pre-close gates: sub-issues, blockers, labels, criteria state (5 checks)
  - Event 17: Closure precedent on siblings #55, #56, #57 (all CLOSED → Done)
  - Event 19: Close-assignment gate (verify assignee present before close)
  - Event 21-22: Full closure sequence: assignment → comment → close → board update → verification
- Gate order: Non-destructive checks first (state queries), assignment verification, then mutations
- Benefit: Prevents closing issues with open work, and keeps closures consistent with
  how sibling tasks were closed. Note these gates do NOT protect against a concurrent
  writer: each is a read taken before a separate write, so state can change in between.
