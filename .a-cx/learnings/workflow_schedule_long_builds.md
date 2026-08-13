---
id: workflow-schedule-long-builds
trigger: "when a CMake configure or build will take >5 minutes"
confidence: 0.75
domain: workflow
scope: project
date: 2026-07-22
---

# Use ScheduleWakeup for long build waits instead of polling

## Action
When a CMake configure, dependency fetch, or multi-target build is running and will likely take 5+ minutes, schedule a long-interval wakeup (300–900 seconds) to check progress asynchronously rather than spinning on tight status polls.

## Evidence
- Observed 3 times in session 0932d151 (events 7, 27, 31)
- Pattern: CMake configure → ScheduleWakeup with 300–900s delay + clear prompt for next steps → returns to check progress after scheduled interval
- Example: after fixing toolchain file and re-running configure, scheduled 900s wakeup ("will need to refetch deps, checking back in 15 min") rather than polling every 10–30s
- Result: cleaner session flow, less noise, respects async completion of long tasks

## How to apply
When triggering a long build/configure on this project, use ScheduleWakeup with:
- **300s** for intermediate stages (e.g., first CMake configure attempt)
- **900s** for full reconfigures with dependency fetches
- Include a clear next-step prompt so context is fresh when wakeup fires
