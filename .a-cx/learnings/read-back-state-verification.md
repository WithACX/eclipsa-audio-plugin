---
id: read-back-state-verification
trigger: "after any GitHub issue state change (close, board status, assignment, label)"
confidence: 0.85
domain: process
scope: project
date: 2026-09-04
---
# Always Verify State Changes by Reading Back

## Action
After updating an issue's state (close, board Status, assignment, labels), immediately read it back from the API and confirm the new value matches what was requested.

## Evidence
- Observed 5+ times in session 736b1387-4c27
- Events: close-assignment gate confirmation (event 21: `assignees` re-read), board Status after merge (event 15), board Status after set (event 26, read-back confirms `Done`), parent state verification after close (event 28-29 confirms #54 still `OPEN` and `In Progress`), registry release and unit record removal (event 33)
- Pattern: Every single state write includes a subsequent read via `gh issue view`, `board_cli.py field-value`, or `close_predicate.py` to confirm the written state persisted
- Outcome: Detected that GitHub's built-in automations sometimes flip board Status incorrectly (reason for the explicit read-back); the read-back catches mismatches before they become data corruption

## How to apply
After each state-changing operation:
1. Immediately query the current state using the same tool path (e.g., `gh issue view <n> --json state`)
2. Compare returned value to what was requested
3. If mismatch, log it and decide whether to retry, fix manually, or escalate
4. Never assume a write succeeded; GitHub automations can override state changes

Example:
```bash
gh issue close 60 --reason completed
back=$(gh issue view 60 --json state --jq '.state')
[ "$back" = "CLOSED" ] && echo "closed" || echo "ERROR: still $back"
```
