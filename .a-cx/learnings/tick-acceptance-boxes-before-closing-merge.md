---
id: tick-acceptance-boxes-before-closing-merge
trigger: "when merging a PR whose body closes an issue in another repo, such as Closes WithACX/eclipsa-audio-planning#N"
confidence: 0.5
domain: process
scope: project
date: 2026-11-26
---
# Tick Verified Acceptance Boxes Before Merging the Closing PR

## Action
Tick each acceptance box you have verified on the PR head before merging, because a merge that carries `Closes <owner/repo>#<issue>` closes the issue immediately and the boxes are then read as they stand.

## Evidence
- Observed 4 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599 (2026-11-25 to 2026-11-26)
- Pattern: Both unit dispatch prompts required every PR body to carry a cross-repo `Closes` line and required ticking boxes before merge. The issue readback after each merge showed all acceptance boxes ticked and the issue CLOSED. A learning PR (#67) for this same rule was opened in the session.
