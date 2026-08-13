---
id: pr-review-workflow-role-detection
trigger: "when running /deliver-pr-review or manual PR review workflows"
confidence: 0.80
domain: workflow
scope: project
date: 2026-07-27
---

# PR Review Workflow: Role Detection, CI Checks, and State Validation

## Action
When reviewing a PR via /deliver-pr-review or equivalent workflows:
1. Detect whether you (the runner) are the PR author (owner mode) or a different reviewer (reviewer mode)
2. Check `process.yaml` for `pr_review` (auto/human/both) and `admin_bypass` settings
3. Validate CI status and PR `mergeable` state before fan-out to review agents
4. Snapshot git state before and after agent work (CHD-04 pattern)
5. Extract full findings from agent transcripts

## Evidence
- Observed 5+ times in session 0ec3a889 (events 11-22, PR #17 review): user ran sequential checks before spawning review agents:
  - Event 11: `gh auth status` + `gh api user --jq .login` to get current user
  - Event 13: `gh pr view 17 --json [metadata]` to get PR author, CI status, mergeable state
  - Event 15: `gh repo view --json owner,name` + `cat .a-cx/process.yaml` to read merge policy
  - Event 19: `gh pr checks 17` to validate CI status
  - Events 21-22: Snapshot git state pre-fan-out
- Pattern: orchestrated PR review steps in dependency order (auth → metadata → policy → CI checks → state snapshot → agents → state verification → findings extraction)
- Root: CI/mergeable validation gates approval and merge decisions; role detection determines whether review posts to the PR or is report-only

## How to apply
**Pre-review checklist:**
1. Auth and user identity: `gh auth status` + `me=$(gh api user --jq .login)`
2. PR metadata: `gh pr view <N> --json author,state,mergeable,statusCheckRollup,files,title,body`
3. Policy: `gh repo view --json owner,name` + check `.a-cx/process.yaml` for `deliver.pr_review` and `deliver.admin_bypass`
4. Role: compare `me == author` (owner mode = report-only, no post/merge) vs. `me != author` (reviewer mode = can post/merge if eligible)
5. CI status: `gh pr checks <N>` -- confirm no failing checks or CI not-yet-run
6. Mergeable check: `mergeable == MERGEABLE` (from pr view) -- no conflicts
7. Snapshot state: save `git status`, `git rev-parse HEAD`, `git ls-remote origin main` BEFORE fan-out
8. Run review agents (code, security, C++ in parallel)
9. Verify state: re-run git checks, confirm no mutations
10. Extract findings: parse agent transcripts, confirm completeness
11. Merge decision: only if verdict=SHIP, CI=green, mergeable=MERGEABLE, no destructive deletions, and (owner-mode skip) or (reviewer mode + eligible + admin_bypass if needed)

See also: [[multi-agent-review-transcript-extraction]], [[multi-agent-fan-out-state-safety]], [[review-agent-findings-completeness-required]]
