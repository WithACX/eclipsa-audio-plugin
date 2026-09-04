---
id: pr-body-repo-convention-match
trigger: "when opening a PR in this repo"
confidence: 0.78
domain: process
scope: project
date: 2026-09-04
---

# Match the PR body convention from merged PRs in the repo

## Action
Before creating a PR, inspect a recently merged PR (e.g., PR #26 in eclipsa-audio-plugin) to see the title format, closing-link style, and body structure. Mirror that format: opening with "Closes WithACX/eclipsa-audio-planning#<N>", body sections separated clearly, and ending with "Assisted with Claude Code".

## Evidence
- Observed in session 61 (lines 45–46, 72–74)
- Pattern: read merged PR → extract structure → reformat draft body to match → verify after push
- PR 26 showed: title with [PAN-##] label, "Closes" on first line, body sections, no heading duplication
- Session 61 applied the same structure to PR 28

## Why
- Consistency with repo style signals attention to maintainability
- Closing-link format varies across projects; matching existing PRs avoids guard/linter issues
- "Assisted with Claude Code" footer is part of this repo's convention (A-CX standard)
- Closing-link verification (guard script) catches inverted references early
