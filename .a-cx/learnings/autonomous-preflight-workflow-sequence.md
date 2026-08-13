---
id: autonomous-preflight-workflow-sequence
trigger: "when running /deliver-autonomous to dispatch one or more units"
confidence: 0.92
domain: process
scope: project
date: 2026-08-13
---
# Autonomous Delivery Preflight Workflow Sequence

## Action
Before dispatching any delivery-unit agent in an autonomous run, execute this preflight sequence: (1) read autonomous-workflow.md and delivery-unit.md to internalize dispatch contracts; (2) validate process routing (validate_process_routing.py) and ledger reconciliation rules (reconcile_routing.py); (3) inspect git state, worktrees, and run registry for claimed issues; (4) verify issue openness; (5) dispatch with full run ledger payload. This sequence establishes the run's authority and prevents re-dispatch of already-merged work.

## Evidence
- Observed 5+ times in session 72dbefe5-1303-4214-a0a3-8a178b1574b1 (lines 7-48 of observer analysis)
- Pattern: (1) Read workflow docs (lines 7-18), (2) validate routing (line 23), (3) inspect state (lines 31-36), (4) confirm issue state (line 46), (5) dispatch (line 47)
- Run prior to this one (aut-20260813-55) executed same sequence and produced complete ledger with orchestrator verification
- Timing: ~100s from first doc read to unit dispatch (lines 7-48)
- Materiality: prevents race conditions where a merged PR is re-dispatched, or a unit is lost between verification and ledger append

## Captured Insights
The preflight gates in the tooling (autonomous-workflow.md section 0, delivery-unit.md step 1) exist to catch:
- Claimed units already in another live run (race prevention)
- Stale ledger entries (merge verification before re-dispatch)
- Malformed payloads (field validation before interpolation)

Reading the references BEFORE dispatch ensures the agent understands why these gates exist and what fallback to take if they fire.
