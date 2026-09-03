---
id: diagnostic-log-extraction-grep-pipeline
trigger: "when polling background job logs during builds, tests, or compile stages"
confidence: 0.85
domain: workflow
scope: project
date: 2026-08-14
---

# Diagnostic Log Extraction via Grep Pipeline

## Action
Extract diagnostic data (errors, test counts, warnings) from background job logs using `grep | head/tail` rather than reading full logs. Pattern: `grep -iE "<pattern>" /tmp/*.log | head -N`.

## Evidence
- Observed 5 times in session 7ec26130 (events #1, #7, #9, #17, #19)
- Event #1: `grep -iE "error:" /tmp/build_split.log | head -10` → extract build errors
- Event #7: `grep -c "Passed" /tmp/ctest_split.log` → count test passes
- Event #9: `grep -c "Passed" /tmp/ctest_split.log` → recheck test count
- Event #17: `grep -iE "error:" /tmp/vst3_split.log | head -5` → extract linker warnings
- Event #19: `grep -iE "error:" /tmp/vst3_split.log | head -5` → verify no compilation errors

## Pattern
Consistent structure across background polling cycles:
1. Start task with `run_in_background: true` → logs to `/tmp/<stage>_split.log`
2. Wait N seconds for task to progress
3. Poll with targeted grep: `grep -iE "<error|pattern>" /tmp/*.log | head -5` or `grep -c "<success_marker>"`
4. Combine with tail: `tail -N /tmp/*.log` to see progress tail
5. Repeat polling until task completes

Advantages over full log read:
- Fast when logs grow large (eclipsa_tests runs 181+ sec)
- Surfaces only actionable lines (errors, counts) not noise
- Limit output with head -N to avoid log spam

Used in: build verification, ctest polling, VST3 build verification, CI check polling.
