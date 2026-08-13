---
id: autonomous-unit-ledger-reconciliation
trigger: "after every /deliver-autonomous unit completion, before closing the run"
confidence: 0.9
domain: process
scope: project
date: 2026-08-13
---

# Comprehensive Ledger Reconciliation for Autonomous Units

## Action
After a unit reaches completion_gate (build, tests, format, review, PR open), run a verification sequence: validate payload PR and SHA, verify PR state on GitHub, confirm all derived issues are filed, check CI status, reconcile routing observations, and close the ledger entry before returning the fixed-shape payload.

## Evidence
Observed 6+ times across unit #55 execution (run aut-20260813-55):
1. Payload validation: PR number and capture SHA parsed and confirmed (event 21)
2. PR state verification: `gh pr view` confirms OPEN, MERGEABLE, reviewDecision, CI status (events 23-24, 48)
3. Derived issues verification: both #65 and #66 confirmed in tracker with correct Origin, Derivation-depth, and Readiness markers (events 25-26)
4. Ledger verification: `ledger_append.py verify` closes entry with status and bucket (event 35)
5. Routing reconciliation: `reconcile_routing.py` confirms no routing divergences (event 37)
6. Routing observations review: confirm agent model choices and attribution (event 40)

## Why
The autonomous run machinery depends on ledger invariants: units recorded with payloads, PRs verified non-merged or merged with witnesses, derived issues traceably filed, routing observations logged. A stall or interruption can leave the ledger in an inconsistent state (e.g., PR filed but payload not recorded, or derived issue filed without Origin marker). Post-completion reconciliation catches these before the run is marked complete, allowing recovery or blocking before the run facade closes.

## How to apply
Before returning the fixed-shape unit payload, run the full sequence:
1. `ledger_append.py record` to write the payload
2. Validate payload fields (PR number, capture SHA) with hand-coded regex
3. Fetch PR metadata, check mergeable/state/CI
4. Verify each derived issue #N exists, has Origin: #<unit>, and Readiness marker if applicable
5. `ledger_append.py verify` to close the entry with verdict and bucket
6. `reconcile_routing.py --observed` to confirm no routing divergences
7. Log the reconciliation result to the unit record

Related: [[unattended-runs-need-caffeinate]], [[merged_pr_upstream_contribution_manual_workflow]]
"