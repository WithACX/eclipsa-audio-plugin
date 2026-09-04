---
id: gh-cli-jq-state-queries
trigger: "when querying or mutating GitHub issue/PR state"
confidence: 0.92
domain: workflow
scope: org-wide
date: 2026-09-04
---
# Use gh CLI + jq for Structured GitHub State Operations

## Action
READ GitHub state with `gh` and `--json --jq` rather than parsing text output. This
covers issue and PR reads, sub-issue and blocker queries, check state, and every
readback in a verification gate.

Issue WRITES are the exception and go through `issue_cli.py`
(`assign`/`label`/`comment`/`close`/`reopen`/`amend`), never a raw
`gh issue edit|close|comment` (ILC-19.1). Board field writes go through
`board_cli.py`. PR operations (`gh pr view|ready|edit|merge`) stay on `gh`, since
no plugin script wraps them. See [[a-cx-plugin-scripts-pattern]], which states the
write side of this rule and would otherwise read as contradicting it.

## Evidence
- Observed 6+ times in session 3fad9120-e811-45f1-925f-06502d2a6c54
- Pattern: Every GitHub state check (PR view, issue state, sub-issues, blockers, criteria, board status) uses `gh <command> --json --jq '<filter>'`
- Events: 1 (PR state), 5 (merge verification), 7 (sub-issues + blockers + labels), 17 (sibling tasks), 21 (close + board), 23 (parent state + sibling blockers)
- Enables: error-free state queries, idempotent operations, easy piping to Python scripts
