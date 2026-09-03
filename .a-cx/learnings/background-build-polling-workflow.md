---
id: background-build-polling-workflow
trigger: "when making multiple edits before checking build success"
confidence: 0.82
domain: workflow
scope: project
date: 2026-08-14
---

# Launch Build in Background, Continue Editing, Poll Progress

## Action
After formatting, launch `cmake --build` with `run_in_background: true`, then continue editing while monitoring the log with `grep | tail` to detect errors early without blocking.

## Evidence
- Observed 2 times in session 7ec26130 (height-indicator branch)
- Pattern: format → build-in-background → edit → poll
  - Line 25: clang-format all 4 files, then `cmake --build ./build --target eclipsa_tests -j $(sysctl -n hw.ncpu) > /tmp/build_conn.log 2>&1` with `run_in_background: true`
  - Line 31: continued editing while build ran; checked progress with `grep -iE "error:" /tmp/build_conn.log | head -10; tail -2 /tmp/build_conn.log`
- Workflow optimized for rapid iteration: compilation happens in parallel with doc refinements, test appends, comment updates
- Falls back to `|| echo "still building"` when log file not yet created, avoiding spurious errors

## Rationale
- Compilation is slow (multi-target build takes minutes); blocking on it halts all other work
- Early error detection via grep prevents cascading downstream fixes
- Tail output confirms the build is still running or shows final status
- Matches memory: "Background task status polling workflow" + "Diagnostic log extraction via grep pipeline"

## When to Apply
- Any multi-edit session where build time exceeds a few seconds
- When follow-on edits (docs, tests, comments) don't depend on successful compilation
- Before opening a PR, run one final foreground build to catch any last-minute errors

## Caution
- Build may fail silently if backgroundTaskId dies; periodically check the actual log, not just tails
- Grep -i catches case-insensitive errors but can miss warnings that fail CI
