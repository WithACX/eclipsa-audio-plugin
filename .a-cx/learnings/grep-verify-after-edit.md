---
id: grep-verify-after-edit
trigger: "after making programmatic edits to source files"
confidence: 0.82
domain: process
scope: project
date: 2026-09-04
---

# Always grep the edit to verify it landed

## Action
After each Python/sed-based file edit, immediately grep or inspect the modified file to confirm the change is present and syntactically correct.

## Evidence
- Observed 4+ times in session 61 (lines 3–4, 5–6, 11–12, 35–36)
- Pattern: Python edit → grep -n keyword or sed inspection
- Catches: typos in search strings, file not found, wrong indentation, incomplete replacements

## Why
- Assertions in the edit script catch missing old patterns, but not what was actually written
- Immediate verification catches copy-paste errors in `replace_all` strings early
- Verifying line numbers ensures edits landed in the expected location
- Compounds with the Python assertion pattern to give double confidence
