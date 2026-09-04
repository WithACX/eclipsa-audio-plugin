---
id: capture-gate-sequence
trigger: "after committing internal work product (.a-cx/learnings/, configs) in this project"
confidence: 0.92
domain: workflow
scope: project
date: 2026-09-03
---

# Capture Gate Sequence for A-CX Internal Work Product

## Action

After every commit that modifies `.a-cx/learnings/`, run the capture gate in sequence: check the exit code, verify clean state, then push and re-verify.

## Pattern

```bash
cd /path/to/worktree
PY="$(command -v python3 || command -v python)"
PR=/Users/branden/.claude/plugins/cache/a-cx-ai-config/dev-process/VERSION

# 1. Run the gate (will withhold invalid paths and report them)
"$PY" "$PR/scripts/capture_internal_artifacts.py" --check
GATE_EXIT=$?

# 2. Inspect what was withheld and why
git status --porcelain

# 3. If GATE_EXIT=0 or only WITHHELD (not REPAIRABLE), proceed:
"$PY" "$PR/scripts/capture_internal_artifacts.py"  # Commits as chore(a-cx)
git push origin BRANCH_NAME

# 4. Re-run --check on the new head
"$PY" "$PR/scripts/capture_internal_artifacts.py" --check
```

Key behaviors:
- A path marked **WITHHELD** does not hold the gate (one --fix cannot clear it; gate opens when human removes it)
- A path marked **REPAIRABLE** holds the gate (fix indicated in the output)
- `--check` returns exit 0 if nothing to capture, even if paths remain withheld
- After `git push`, CI will run the learnings validator and catch formatting errors
- `git status --porcelain | wc -l` should be 0 after capture + push

## Evidence

Observed 4+ times in session 3fad9120:
- Event 2-3: Initial gate check, reports 4 modifications and warns about withheld MEMORY.md
- Event 8-9: Full capture → push → re-check sequence; GATE_EXIT=0 after push
- Event 10-11: CI check polled after push; Clang Format pass on new head
- Event 20-21: Final gate re-run; worktree shows only .a-cx/learnings/MEMORY.md dirty (withheld, not blocking)

The sequence is deterministic: gate → inspect status → capture (if clear) → push → re-gate.

