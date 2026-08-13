---
id: multilayer-process-diagnosis
trigger: "when a long-running build or CMake configure appears slow or stuck"
confidence: 0.80
domain: workflow
scope: project
date: 2026-07-21
---

# Multi-Layered Process Health Diagnosis

## Action
When a process seems hung or suspiciously slow, run a structured diagnostic: disk growth (`du`), process runtime + CPU (`ps -p` with etime/time/%cpu), network state (`lsof`), and log tail together—never assume hung without checking all layers.

## Evidence
- Observed 3 times in session 0932d151
- Event 15: `du -sh build/`, `ps`, `lsof -p <pid>` to check if cmake configure was actually stuck vs. just slow
- Event 19: repeated the layered check (du, ps with metrics, tail log)
- Event 21: diagnosis revealed LibIAMF fetch in CLOSE_WAIT network state (hung connection)
- Result: Correctly identified network hang vs. local computation hang, enabling targeted fix (kill + clear partial download, retry)
