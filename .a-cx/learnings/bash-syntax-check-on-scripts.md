---
id: bash-syntax-check-on-scripts
trigger: "when creating or moving bash scripts"
confidence: 0.85
domain: code-style
scope: project
date: 2026-09-04
---

# Always Run `bash -n` Syntax Check on Shell Scripts

## Action
Before committing or deploying a bash script, run `bash -n <script>` to validate syntax without execution.

## Evidence
- Observed 3 times in session 736b1387: events 5 (original), 21 (worktree), 22 (restored copy)
- Pattern: `bash -n .a-cx/bin/uninstall_dev_plugins.sh && echo "bash -n OK"`
- All instances returned `bash -n OK` confirming no syntax errors
- Applied to shell scripts in guarded directories (`.a-cx/bin/`)

## Why
- Bash scripts fail silently at runtime if syntax is broken
- A copy/restore operation might introduce subtle corruption (rare but possible)
- Syntax check is zero-cost and catches errors before they affect developers
- Part of quality gate for `.a-cx/` internal tools

## How to Apply
1. After copying or editing a bash script, immediately run `bash -n <path>`
2. Require exit code 0 before proceeding to commit or deploy
3. Include in worktree validation gates before pushing
4. Also applicable to scripts in `scripts/` and other shell-holding directories

[[file-integrity-verification-on-moves]]
