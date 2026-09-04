---
id: surgical-read-offset-before-edit
trigger: "when targeting a specific section of a large file for editing"
confidence: 0.75
domain: workflow
scope: project
date: 2026-08-14
---

# Use Read with offset/limit to Get Context Before Surgical Edits

## Action
When editing a large file, use `Read(offset=LINE, limit=LINES)` to fetch only the section you need, then perform the edit. Reduces context noise and keeps mental model tight.

## Evidence
- Observed 3 times in session 7ec26130 (height-indicator branch)
- Pattern: grep → read offset/limit → edit
  - Line 1: `grep | head` to understand splitAtElevation's docstring location, then `Read offset=100 limit=78` fetches just the docstring + function signature (11 lines of code + 67 lines of docs)
  - Line 17: `grep -n "paintHeightIndicatorConnectors"` found at line 273, then `Read offset=271 limit=32` fetches the function definition + comments in one call
  - Line 31: Used `grep | tail` to check build progress from log file without reading whole log
- More efficient than full-file reads for files >200 lines
- Grep output tells you the line number; pass offset=lineNumber-2 to capture surrounding context

## Rationale
- Header files like HeightIndicator.h contain 200+ lines of geometry and docs; reading the whole file loads excessive context
- Each targeted edit (docstring update, function signature change, comment refinement) affects 10-30 lines
- Offset/limit keeps the read to just the section, reducing token cost and noise

## When to Apply
- Files >150 lines where you know the exact section to modify
- When grep has already located the section and you just need surrounding context
- Large test files where you're appending a specific test or modifying one handler

## Maturity Note
This pattern overlaps with `Partial read with offset/limit` in existing memory. Confirmed as a consistent practice across multiple editing sessions on this codebase.
