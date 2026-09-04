---
id: autonomous-unit-finalization-workflow
trigger: "after /deliver-autonomous completes a unit"
confidence: 0.75
domain: process
scope: project
date: 2026-08-14
---

# Autonomous Unit Finalization Workflow

## Action
After a delivery-unit agent returns its payload and reconciles the ledger, complete a 4-step finalization sequence: flush the learnings observer, check for uncaptured work products, verify gitignore status, and reconcile routing observations.

## Evidence
- Observed 1 time in session 7ec26130-d45c-4ca6-8362-40a98c8cc959 (unit #57 delivery, 2026-08-13)
- Pattern: Occurs after `ledger_append.py verify` and before returning control
- Workflow steps (in order):
  1. `observer-loop.py --once` (learnings plugin flush, timeout 120-180s)
  2. `capture_internal_artifacts.py --check` (identify uncaptured A-CX products like .a-cx/runs/autonomous/*.json)
  3. `git check-ignore` (verify .a-cx/runs/ is NOT ignored -- it should be version-controlled as work product)
  4. `reconcile_routing.py --observed` (validate observed agent models and tiers vs. declared)

## Why
The deliver-code workflow doc (deliver-code-workflow.md:1655-1671) explicitly mandates this "flush the producer first" pattern to capture session learnings before the unit context exits. The finalization ensures:
- Learnings are recorded in the repo
- Work product ledgers are staged for commit (not ignored)
- Routing observations are reconciled (model/effort audit trail)
- Session state is clean before handoff

## Related
- [[carry-forward-relocates-untracked-.a-cx-files]] -- finalization preserves .a-cx/runs/ files that the unit's worktree pick moved
- [[autonomous-unit-ledger-reconciliation]] -- reconcile ledger before returning fixed-shape payload
- [[autonomous-preflight-workflow-sequence]] -- sequence verification rules
