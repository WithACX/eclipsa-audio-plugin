---
id: gh-token-read-project-scope-gap
trigger: "when gh token lacks 'read:project' scope and board_cli.py queries fail"
confidence: 0.95
domain: security
scope: org-wide
date: 2026-08-13
---
# gh Token Lacks 'read:project' Scope for GitHub Projects V2

## Action
Grant the GitHub token the `read:project` scope to allow board_cli.py and derived-bug sweep to query GitHub Projects V2 board status. Current token scopes: ['gist', 'read:org', 'repo', 'workflow']. Missing: 'read:project'.

## Evidence
- Observed 4 times in session 72dbefe5-1303-4214-a0a3-8a178b1574b1
- Pattern: board_cli.py list-field fails with "Your token has not been granted the required scopes: ['read:project']"
- Impact: `sweep_derived_bugs.py` cannot evaluate board predicate `Status == Todo`, so derived-bug sweep is unreachable
- Workaround (used): Fall back to LABEL-based bug query instead of board status
- Affected tools: board_cli.py (sweep_derived_bugs.py, project-management plugin)
- Location: .git token at https://github.com/settings/tokens

## Why This Matters
The autonomous delivery run cannot sweep open bugs by board status. It must fall back to label-based queries, losing precision on issue readiness. Once the scope is granted, sweep eligibility becomes deterministic and repeatable.
