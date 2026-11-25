---
id: tick-acceptance-boxes-before-closing-merge
trigger: "when a PR body carries a Closes reference to an issue whose acceptance criteria are checkboxes and the PR is about to be merged"
confidence: 0.4
domain: process
scope: project
date: 2026-11-25
---
# Tick Verified Acceptance Boxes Before the Closing Merge

## Action
Tick each acceptance checkbox you have verified on the issue before merging the PR that closes it, because merging closes the issue and boxes ticked after that are not part of the completed record.

## Evidence
- Observed 3 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599 (2026-11-25)
- Pattern: Unit #65 merged its PR and closed WithACX/eclipsa-audio-planning#65 with two verified acceptance boxes still unticked, and the unit stopped with "2 acceptance boxes verified but not ticked after close". The unit's boxes were then ticked by hand after the fact, and #107 had its boxes ticked the same way. The next unit (#52) was dispatched with an explicit instruction to tick verified boxes BEFORE merging.
