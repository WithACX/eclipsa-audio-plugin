---
id: background-task-status-polling-workflow
trigger: "when spawning long-running builds or tests in the background during code work"
confidence: 0.80
domain: workflow
scope: project
date: 2026-08-14
---

# Background Task Status Polling During Intervening Work

## Action
Spawn long-running build/test tasks with `run_in_background: true` and redirect output to a persistent log file. Continue with other work (code changes, issue management, formatting checks). Periodically poll the log via `tail` to check progress without blocking the main workflow.

## Evidence

- Observed 4 times in session 7ec26130-d45c-4ca6-8362-40a98c8cc959 (2026-08-14)
- Build spawn (event 25): `cmake --build ... > /tmp/build_57_1px.log 2>&1` with `run_in_background: true`
- Status checks (events 20, 29): `tail /tmp/build_57_1px.log` while doing formatting/issue work
- Ctest spawn (event 31): `ctest ... > /tmp/ctest_57_1px.log 2>&1` with `run_in_background: true`
- Status checks (events 43, 48): `tail /tmp/ctest_57_1px.log` while doing other work

## Pattern Details

1. **Spawn phase**: Launch build or ctest with output redirected to a timestamped log file
2. **Work phase**: Continue with code edits, formatting checks, issue management, etc. while tasks run
3. **Poll phase**: Periodically `tail -N /tmp/<task>.log` to check progress
4. **No blocking**: Main workflow is not interrupted; checks are opportunistic

## Why It Works

- Builds and ctest can take 5–10 minutes; spawning in background avoids idle time
- Log file is persistent and tailable from any shell context
- User can review code changes, run formatting checks, and manage issues while waiting
- Periodic polls (via tail) provide visibility without polling overhead

## Related Learnings

- [[build_plugin_for_manual_testing]] -- proactive VST3 build for DAW testing
- [[unattended_runs_need_caffeinate]] -- machine sleep management for unattended runs
- [[build_test_before_pr]] -- the broader build-test-format cycle
