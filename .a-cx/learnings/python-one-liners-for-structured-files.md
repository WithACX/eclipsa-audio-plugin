---
id: python-one-liners-for-structured-files
trigger: "when editing JSON, Markdown frontmatter, or large config files in a Bash script"
confidence: 0.85
domain: tooling
scope: project
date: 2026-09-03
---

# Python One-Liners for Structured File Editing

## Action

Use Python's `pathlib` and string methods in a Bash heredoc to edit JSON, Markdown, and config files programmatically. This avoids sed escaping hell and makes multi-line replacements legible.

## Pattern

Bash here-document with Python makes multi-step edits readable:

```bash
PY="$(command -v python3 || command -v python)"
"$PY" << 'PYEOF'
import pathlib, json

# Read, modify, write back (common on this repo):
p = pathlib.Path(".a-cx/runs/ISSUE.json")
r = json.loads(p.read_text())
r["field"]["subfield"] = "new value"
r["criteria_result"] = "Ticked 5, left unticked 5"
p.write_text(json.dumps(r, indent=2) + "\n")

# Or for Markdown: simple string replace
p = pathlib.Path(".a-cx/learnings/file.md")
s = p.read_text()
s = s.replace("old text", "new text", 1)  # count=1 for first occurrence only
p.write_text(s)

print("file updated")
PYEOF
```

Advantages over sed:
- Readable multi-line replacements (no escaping `\/` and `\n`)
- Easy to do `json.loads()` → modify fields → `json.dumps()` → write back
- Works identically on macOS and Linux (no sed `-i ''` vs `-i` differences)
- Can do conditional checks: `if "pattern" in s: s = s.replace(...)`

Common edits on this project:
1. **JSON unit records** (`.a-cx/runs/{ISSUE}.json`): Update verdicts, evidence, state
2. **PR body files** (scratchpad): Update closing links, chore count, test evidence
3. **Learnings files**: Fix em-dashes, correct factual errors, update frontmatter

## Evidence

Observed 3+ times in session 3fad9120:
- Event 6: Python reads 3 Markdown files, replaces multi-line blocks to fix gate-sequence order and board-guard scope
- Event 12: Python reads PR body, updates chore commit count and adds explanations of corrections
- Event 20: Python loads `.a-cx/runs/59.json`, writes multiple new fields (report details, verdicts, artifacts), and writes back formatted

No sed call appears in the session; all file edits use Python for clarity and robustness.

