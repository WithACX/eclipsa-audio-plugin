---
id: a-cx-plugin-scripts-pattern
trigger: "when executing structured issue operations (criteria ticking, verdict recording, board updates)"
confidence: 0.89
domain: workflow
scope: org-wide
date: 2026-09-04
---
# Use a-cx-ai-config Plugin Scripts for Structured Operations

## Action
For issue mutations (close predicates, verdict recording, criteria ticking, board field updates, issue assignment), always dispatch the appropriate Python script from `a-cx-ai-config/dev-process` or `project-management` rather than calling gh CLI directly. This centralizes validation, schema enforcement, and idempotency.

## Evidence
- Observed 7+ script invocations in session 3fad9120-e811-45f1-925f-06502d2a6c54
- Pattern: Every structured operation uses a dedicated Python CLI from the plugin
  - `close_predicate.py` for gate checks (Events 3, 23)
  - `parent_verdicts.py` for criterion recording (Events 11, 13)
  - `tick_acceptance_criteria.py` for criteria updates (Events 11, 15)
  - `issue_cli.py` for assign/close/comment (Events 19, 21)
  - `board_cli.py` for status/field updates (Events 19, 21, 23)
- Resolve the script path per knowledge/bundled-script-invocation.md. Do not hard-code a
  plugin cache path: it carries a version and a home directory, so it is wrong on any
  other machine and stale after the next plugin update.
- Benefit: Schema validation, error handling, idempotency, consistency across the codebase
