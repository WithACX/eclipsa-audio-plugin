---
id: blocking-wait-on-background-unit-output
trigger: "when a delivery unit runs as a background agent and the orchestrator must wait for its outcome line"
confidence: 0.6
domain: workflow
scope: project
date: 2026-11-25
---
# Block on a Background Unit's Output File Instead of Re-Prompting

## Action
Wait for a background delivery unit by running a blocking wait on its output file with a long timeout, and check PR state on each wake.

## Evidence
- Observed 11 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599 (2026-11-25)
- Pattern: After launching a unit agent with `run_in_background` semantics, the orchestrator repeatedly ran a wait script on the agent's output file (600000 ms timeout). Each call returned `WAITING age=N` until the unit finished. Two of those wake-ups also listed open PRs with `gh pr list`.
