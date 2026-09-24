---
id: acceptance-criteria-accuracy
trigger: "when close_predicate.py holds a closure due to unchecked acceptance criteria"
confidence: 0.80
domain: process
scope: project
date: 2026-09-04
---
# Never Tick Acceptance Criteria Without Evidence

## Action
When a close predicate hold names unchecked acceptance criteria, do not tick them to make the closure pass. Instead, move misfiled criteria to the child issues that actually own them, document the delegation, and close with the corrected set.

## Evidence
- Observed 3+ times in session 736b1387-4c27
- Events: close_predicate hold diagnosed (event 10, names 3 unchecked boxes), user decision point (event 18, chooses not to falsify), implementation (events 22-24, posts close comment explaining decision, notes the carry-forward to #62)
- Pattern: When #60's close_predicate held on criteria 6, 7, 8 (automation recording, dial tracking, per-host input delivery), user explicitly refused to tick them because they were not observed. Instead, moved criterion 6 (per-host input delivery) to #62 because REAPER doesn't deliver arrow keys, making verification impossible in that task's chosen host.
- Outcome: Close comment documents why criteria were unticked; #62 receives a note that three checks were carried forward from #60 with reasoning; close_predicate passes on the corrected set; parent #54 correctly computes done-status based on accurate closure record

## How to apply
1. If close_predicate holds on unchecked criteria, examine each one
2. For each unverified criterion, ask: "Did I actually observe this in testing?"
3. If no: do not tick it
4. Instead: if the criterion belongs in a child task (cross-DAW verification, edge case testing, etc.), move it there with a comment on both issues
5. Re-run close_predicate on the cleaned set
6. Document the decision in the close comment so future readers know why the criterion set changed
