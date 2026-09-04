---
id: close-predicate-gate-workflow
trigger: "when closing a parent issue with open sub-issues or unchecked acceptance criteria"
confidence: 0.82
domain: process
scope: project
date: 2026-09-04
---
# Close Predicate as an Enforced Completion Gate

## Action
Run `close_predicate.py --repo <repo> --parent <issue>` before closing a parent to verify all five conditions hold; never force-close while a hold is present.

## Evidence
- Observed 8 times in session 736b1387-4c27
- Events: conditions checked on #60 (events 6, 10), #54 (events 14, 28), referenced in planning (events 16, 19, 26, 28)
- Pattern: The tool surfaces why a parent cannot close (open sub-issues, unchecked criteria, open PRs referencing it, blocked-by dependencies, needs-answer markers)
- Outcome: In #60 case, close predicate held on 3 unchecked criteria; user delegated them to #62 with documentation rather than ticking false positives, then re-ran predicate (passed on final attempt with criteria moved)

## How to apply
Before closing any parent issue:
1. Run `close_predicate.py --repo WithACX/eclipsa-audio-planning --parent <issue>` 
2. Read all listed failures -- each one represents a real blocker
3. If criteria are unchecked, either verify them or move them to a child issue with inline documentation
4. Never tick acceptance criteria without evidence observed in testing
5. Re-run the predicate to confirm pass before closing
