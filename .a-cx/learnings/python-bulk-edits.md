---
id: python-bulk-edits
trigger: "when making multiple complex string replacements in a single file"
confidence: 0.75
domain: code-style
scope: project
date: 2026-09-21
---

# Use Python inline scripts for bulk file edits with assertions

## Action
Write Python scripts (via `python3 - <<'PY'`) that read a file, perform multiple chained `.replace()` calls with assertions, and write the file back. Chain replacements so failures surface immediately via assertion errors.

## Evidence
- Observed 3 times in session fc88928f-8d1c-464f-a393-6659bdfcf77c (events 13, 15, 25)
- Pattern: `python3` with heredoc, load file, chain multiple `s.replace(old, new)` with `assert old in s` before each
- Example: Updating test assertions across multiple lines with backup safety
  - Event 13: 6 separate multiline replacements in one script
  - Event 15: Additional replacements with assertion check
  - Event 25: Updating PR description body with large multiline changes
- Used for: test fixture updates, refactoring where manual editing risks missing cases, coordinated changes across a file
- Advantage: assertions catch malformed old_string early, chainable operations are clear to read
