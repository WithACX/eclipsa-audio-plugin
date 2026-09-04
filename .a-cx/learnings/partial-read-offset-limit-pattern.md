---
id: partial-read-offset-limit-pattern
trigger: "when reading specific context from large test or implementation files"
confidence: 0.75
domain: code-style
scope: project
date: 2026-08-14
---

# Use Read offset/limit for efficient partial file reads

## Action

Use explicit `offset` and `limit` parameters on Read calls to fetch only the context range you need, avoiding full-file reads when exploring large test or implementation files.

## Evidence

- Observed 3 times in session 7ec26130 (height-indicator feature)
- Pattern: Read(offset=44, limit=28) to grab a specific function block
- Applied to: HeightIndicator_test.cpp, PerspectiveRoomViews.cpp
- Benefit: Speeds up context exploration without loading entire 200+ line files

## How to apply

When you've located a function or test with grep/sed and only need to see that section:

```bash
# Grep to find the line number
grep -n "TEST.*SplitAtElevation" file.cpp

# Then Read with offset/limit instead of the whole file
Read(file_path, offset=123, limit=15)  # Read 15 lines starting at line 123
```

This is faster than full-file reads and keeps the focus narrow during code exploration.
