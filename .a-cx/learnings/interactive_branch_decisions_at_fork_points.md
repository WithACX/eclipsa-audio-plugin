---
id: interactive_branch_decisions_at_fork_points
trigger: "when a delivery task reaches a branch topology decision point (main vs acx/dev, amend vs new branch)"
confidence: 0.85
domain: process
scope: project
date: 2026-08-06
---
# Ask user for branch/PR strategy at fork topology decision points

## Action
When a delivery task reaches a point where multiple valid paths exist for branching or PR strategy (e.g., main vs acx/dev as the base, amend existing PR vs open a new one), explicitly ask the user to choose. Do not infer the path from task context or precedent; the decision depends on intent (internal-only vs upstream-contribution-ready, solo fix vs cumulative PR) that only the user can clarify.

## Evidence
- Observed 2026-08-06 in session 4082aef7:
  - Line 9 (observer log): Asked "Where should this fix branch from?" with two options:
    - main (Recommended): "Keeps the diff upstream-contribution-ready"
    - acx/dev: "Keeps this strictly an internal fix"
  - User chose `acx/dev`, a deliberate decision that changed the downstream PR strategy
  - Line 21: Later asked "PR #18 already adds this test but by committing a real .mov fixture. How do you want to proceed?" with three branching strategies (amend, new branch on top, new branch off acx/dev)
  - User chose the third option, which determined the entire subsequent workflow
- Pattern: Both decision points have structural consequences (what can later be upstreamed, what conflicts with existing work) that should not be guessed by the orchestrator

## Related
- `.a-cx/docs/fork-workflow.md` — explains the main/acx/dev branch model and when to use `/upstream-pr`
- [[always_use_a_worktree]] — branch decisions imply worktree creation, which is non-negotiable per repo policy
