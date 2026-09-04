---
id: python-pathlib-for-bulk-text-edits
trigger: "when refactoring comments or performing bulk text replacements across multiple code files"
confidence: 0.85
domain: workflow
scope: project
date: 2026-09-04
---

# Use Python pathlib for Bulk Text Replacements, Not sed/awk

## Action
When refactoring comments or applying text changes to multiple .h/.cpp files, use Python pathlib with a list of (old, new) tuples rather than complex sed/awk pipelines.

## Evidence
- Observed 6 times in session 3fad9120-e811-45f1-925f-06502d2a6c54 (events 3, 8, 10, 12, 14, 20)
- Pattern: `import pathlib; p = pathlib.Path(...); s = p.read_text(); reps = [(old, new), ...]; for old, new in reps: s = s.replace(old, new); p.write_text(s)`
- Used consistently for comment refactoring across Coordinates.h, Coordinates.cpp, PerspectiveRoomViews.h, PerspectiveRoomViews.cpp, RoomViewScreen.cpp, and Coordinates_test.cpp
- Each multi-edit job was self-contained in one Python block, making verification and atomicity simpler

## Why
Python pathlib is clearer than sed piping, handles multi-line replacements without escaping nightmares, and the assertion pattern (`assert s.count(old) == 1`) catches duplicate or missing matches immediately. This matters when every replacement must land exactly once.

## How to apply
Write a Python heredoc in Bash when you have 2+ text replacements to make in a single file. Collect them as tuples, iterate once, and assert each match count. If you have multiple files, chain them in the same script so one failure stops all.
