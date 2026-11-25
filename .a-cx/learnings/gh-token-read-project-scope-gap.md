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
Observed: `board_cli.py list-field` fails because the token is missing a scope required for Projects v2 board queries (`read:project`), and the derived-bug sweep falls back to a label-based query. Use the label-based fallback when this happens. Any change to the token's scopes is a credential decision for whoever administers that token, raised as its own issue; this learning does not authorize it.

## Evidence
- Observed 4 times in session 72dbefe5-1303-4214-a0a3-8a178b1574b1
- Pattern: board_cli.py list-field fails with "Your token has not been granted the required scopes: ['read:project']"
- Impact: `sweep_derived_bugs.py` cannot evaluate board predicate `Status == Todo`, so derived-bug sweep is unreachable
- Workaround (used): Fall back to LABEL-based bug query instead of board status
- Affected tools: board_cli.py (sweep_derived_bugs.py, project-management plugin)

## Why This Matters
The autonomous delivery run cannot sweep open bugs by board status. It falls back to label-based queries, losing precision on issue readiness.
