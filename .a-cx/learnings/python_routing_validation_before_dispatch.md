---
id: python-routing-validation-before-dispatch
trigger: "when preparing to dispatch work via autonomous or deliver skills"
confidence: 0.7
domain: process
scope: project
date: 2026-08-13
---

# Python Routing Validation Before Dispatch

## Action
Run `validate_process_routing.py` and `reconcile_routing.py` to verify skill routing configuration before dispatching units or creating run ledgers.

## Evidence
- Observed 3 times in session f5088278-7044-4917-a33f-8bfffb6832c7 (2026-08-13)
- Scripts: 
  - validate_process_routing.py (line 31) — checks .a-cx/process.yaml routing entries
  - reconcile_routing.py --declared (line 32) — resolves and validates model/effort routing
  - ledger_append.py --help (line 40) — inspects ledger management subcommands
- Pattern: Run validator -> review JSON output -> proceed with dispatch only if exit=0
- All validation checks passed (exit code 0) before dispatching unit #55

## Why This Matters
The deliver-autonomous and deliver-code skills read .a-cx/process.yaml and apply model routing declarations. If routing config is malformed or has conflicts:
1. Validation catches it before a full dispatch
2. The JSON output shows exactly which entries failed and why
3. Early exit prevents silent failures where a unit starts without proper model assignment

## When to Apply
- Before running `/deliver-autonomous`
- Before a Project-mode `/deliver-code` run with multiple units
- When .a-cx/process.yaml has been edited or new routing declarations added
- As part of pre-flight checks in complex orchestration runs

## Tools
- `/opt/homebrew/bin/python3 <PR_ROOT>/skills/deliver-code/scripts/validate_process_routing.py --repo-root . --json`
- `/opt/homebrew/bin/python3 <PR_ROOT>/skills/deliver-autonomous/scripts/reconcile_routing.py --repo-root . --declared --json`
