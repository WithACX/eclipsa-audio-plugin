---
id: python-str-replace-precision-edits
trigger: "when making multi-line edits to C++ source files"
confidence: 0.85
domain: code-style
scope: project
date: 2026-09-04
---

# Use Python pathlib + assert for reversible precision edits

## Action
Use Python string replacement with `pathlib.Path` and assertions to make multi-line edits to source files, verifying each step reversible and exact.

## Evidence
- Observed 5+ times in session 61 (lines 1–13, 20, 45, 61)
- Pattern: read text → assert old pattern exists → replace → write back → verify with grep
- Applied to: PerspectiveRoomView.h/cpp (4 edits), PannerInput.h (1 edit), PerspectiveRoomViews.h/cpp (2 edits), PannerInput_test.cpp (1 edit)
- Each edit embedded the old text as an assertion before replacement, catching silent mismatches

## Why
- Assertion prevents silent mismatches when file structure changes
- Multi-line string literals preserve whitespace and comments exactly
- Grep verification after replacement confirms the change landed
- Bash + Python keeps the edit atomic and reads cleanly in logs
