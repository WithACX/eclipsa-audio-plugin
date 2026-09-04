---
id: em-dash-normalization-requirement
trigger: "when editing learnings files or internal documentation in this project"
confidence: 0.95
domain: brand
scope: project
date: 2026-09-03
---

# Em-Dash Normalization Requirement

## Action

Before committing any `.a-cx/learnings/` file, ensure it contains NO em-dashes (U+2014 ` -- `). Replace all em-dashes with ASCII double-hyphens ` -- ` (space, two hyphens, space).

A-CX brand guidelines require ASCII-only dashes in all code and documentation. The `capture_internal_artifacts.py` script detects and reports em-dashes but does not auto-fix them; they block CI validation.

## Pattern

Em-dashes appear in two contexts on this project:

1. **Manually written learnings**: When composing `.a-cx/learnings/` files in an editor, spell-check or Mac autocorrect may insert em-dashes instead of hyphens in phrases like "X -- Y".

2. **Observer-written learnings**: The learnings observer sometimes includes em-dashes in its auto-generated prose (e.g., "the script runs -- when the build completes").

To fix:
```bash
# Find all em-dashes in learnings
grep -l $' -- ' .a-cx/learnings/*.md

# Replace in a file (bash, no sed):
python3 << 'EOF'
import pathlib
p = pathlib.Path("path/to/file.md")
s = p.read_text()
s = s.replace(" -- ", " -- ")  # em-dash to ASCII double-hyphen
p.write_text(s)
EOF

# Or use sed (GNU sed on Linux; BSD sed on macOS needs -i '' and escaping):
# macOS: sed -i '' 's/ -- / -- /g' file.md
# Linux: sed -i 's/ -- / -- /g' file.md
```

The capture gate will report: `em-dash normalized: .a-cx/learnings/FILENAME.md` for each affected file.

## Evidence

Observed 5+ times in session 3fad9120:
- Event 3: Initial gate check reports "would normalize 4 learnings to remove em dashes"
- Event 5: All three observer-written learnings contain em-dashes
- Event 7: Python script corrects em-dashes in all three files before commit
- Event 9: Post-capture gate check shows no em-dashes remain
- Event 23: Final verification: `grep -c $' -- ' .a-cx/learnings/MEMORY.md` returns 0

Every learnings file created or edited on this project will be normalized before it can pass the capture gate and CI.

