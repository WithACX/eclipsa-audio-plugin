---
id: scheduled-waits-for-long-builds
trigger: "when starting long-running CMake configures or multi-minute builds"
confidence: 0.85
domain: workflow
scope: project
date: 2026-07-21
---

# Scheduled Waits Instead of Tight Polling for Long Builds

## Action
Use ScheduleWakeup to check on long-running builds at reasonable intervals (600–900s) rather than polling every few minutes—matches actual build duration and keeps the agent loop clean.

## Evidence
- Observed 4 times in session 0932d151
- Events 5, 11, 17, 29: ScheduleWakeup for CMake configure checks
- Delays picked to match expected build phase duration (10–15 min for full dep tree fetch)
- Pattern description: "checking again in N min rather than tight polling"
- Result: Allowed multi-GB dependency fetch to complete without unnecessary context overhead
