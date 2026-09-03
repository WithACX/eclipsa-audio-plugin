---
id: deliver-code-python-script-gate-sequence
trigger: "after a deliver-code unit completes and you need to record acceptance criteria and verdicts"
confidence: 0.90
domain: workflow
scope: project
date: 2026-09-03
---

# Deliver-Code Python Script Gate Sequence

## Action

After a delivery unit's code is committed, chain the deliver-code plugin scripts in order:

1. `capture_internal_artifacts.py` -- gather `.a-cx/learnings/` files and learnings ledger  
2. `tick_acceptance_criteria.py --apply` -- mark criteria checkboxes pass/fail based on unit record  
3. `parent_verdicts.py record --publish` -- record the parent criterion this unit earned (if any)  

Each script reads the unit record from `.a-cx/runs/{ISSUE}.json` and updates the tracker issue. Run from the worktree root with `--repo` and `--run-id` set.

## Pattern

The order that matters is parent_verdicts BEFORE the tick, because ticking only
transcribes what is already recorded and never re-judges the work. `capture` is
NOT part of that chain -- it commits work product and feeds no verdict. (An
earlier version of this note claimed capture had to run first to feed the
verdicts; that was wrong.)

```bash
# Verdicts are already written into .a-cx/runs/$ISSUE.json by this point.
python3 $PLUGIN/skills/deliver-code/scripts/parent_verdicts.py record \
  --parent $PARENT --unit $ISSUE --run-id "run-$(date +%Y%m%d)" \
  --repo $TRACKER --criterion "..." --verdict "..." --publish
python3 $PLUGIN/skills/deliver-code/scripts/tick_acceptance_criteria.py \
  --issue $ISSUE --repo $TRACKER --run-id "run-$(date +%Y%m%d)"          # dry run
python3 $PLUGIN/skills/deliver-code/scripts/tick_acceptance_criteria.py \
  --issue $ISSUE --repo $TRACKER --run-id "run-$(date +%Y%m%d)" --apply
```

The scripts read the same `.json` record, so unit record truth is shared across all three gates.

## Evidence

Observed 4 times in session 3fad9120:

- Event 23: `capture_internal_artifacts.py` returns 0, produces 3 learnings captured
- Event 40: `capture_internal_artifacts.py --release-captured` used to clean up carried-forward paths from the primary clone
- Events 59, 63, 65: `tick_acceptance_criteria.py` dry-run, then `parent_verdicts.py record --publish`, then `tick_acceptance_criteria.py --apply`

Corrected against the run it was derived from: the applying tick came AFTER
`parent_verdicts.py record`, and the tick was dry-run first. `capture` ran both
before the tick (the contribution's own work product) and again at the completion
gate, so it is not a once-only step in a fixed chain.
