---
id: github-cli-board-gate-sequence
trigger: "when transitioning a PR from draft to ready for review on this repo"
confidence: 0.85
domain: workflow
scope: project
date: 2026-09-03
---

# GitHub CLI + Board Gate Sequence for PR Ready Transition

## Action

When a PR is ready to move out of draft, run these gates in order:

1. `closing_list_guard.py` -- block if the closing issue has open sub-issues or blockers  
2. `gh pr ready` -- take the PR out of draft mode  
3. `board_cli.py set-field ... --value "In Review"` -- move the linked issue to In Review column  
4. `issue_cli.py label --add status:in-review --remove status:ready` -- update the status label  
5. Optionally request a reviewer via `gh pr edit --add-reviewer`. GitHub silently
   refuses the PR AUTHOR as a requested reviewer: the command exits 0 and the
   readback shows `reviewRequests: []`. Always read back and never report a
   reviewer as assigned on the exit status alone.

## Pattern

The guard runs **before** draft removal. If it fails, the PR stays draft and the board does not move. The board and label updates happen **after** the PR status changes, so they reflect the current gate state.

```bash
# Guard gate (fails if blocking issues exist):
python3 $GH/scripts/closing_list_guard.py \
  --repo $TRACKER --body-file pr-body.md

# PR state transition:
gh pr ready $N --repo $OWNER/$REPO

# Board + label sync:
python3 $GH/scripts/board_cli.py set-field \
  --owner withACX --number 1 --repo $TRACKER --issue $ISSUE \
  --field Status --value "In Review"
python3 $GH/scripts/issue_cli.py label \
  --repo $TRACKER --issue $ISSUE \
  --add "status:in-review" --remove "status:ready"
```

The board field and the `status:*` label are written SEPARATELY here -- on this
repo `issue_cli.py label` reported `board_fields_mirrored: 0`, so do not assume
setting one moves the other. Board writes also need the `project` token scope;
`read:project` reads the board but cannot move it, and label writes work on
`repo` alone, so the two can silently disagree.

## Evidence

Observed 3 times in session 3fad9120:

- Event 31: `closing_list_guard.py` check passes (no open sub-issues, no blocking issues)
- Event 49: `gh pr ready` completes; GitHub refuses to add the author as a requested reviewer
- Event 51: `board_cli.py set-field` moves #59 to In Review; `issue_cli.py label` adds/removes status labels; parent #54 stays In Progress (3 of 4 children still open)

The order is: guard → PR ready → board sync. The guard failures prevent the entire sequence from running.
