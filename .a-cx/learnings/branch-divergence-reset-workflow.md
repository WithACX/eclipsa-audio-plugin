---
id: branch-divergence-reset-workflow
trigger: "when acx/dev is ahead of or behind origin/acx/dev after a setup session"
confidence: 0.82
domain: git
scope: project
date: 2026-08-12
---

# Branch Divergence Resolution: Fetch, Inspect, Reset

## Action
When acx/dev local branch is ahead of or significantly behind origin/acx/dev (remote), fetch all remotes, inspect the divergence, and reset to remote if the local-only commits are stale or redundant.

## Evidence
- Event logs from sessions 40776e8d, 5c6aba7e show **3 distinct workflows**:
  1. Line 1-2: Push rejected — fetch required
  2. Line 3-4: Fetch and analyze divergence with git rev-list to count ahead/behind
  3. Line 10-15: Fetch, inspect local-only commits, then `git reset --hard origin/acx/dev` + sync main
- Pattern: Repeated across multiple sessions (40776e8d → 5c6aba7e), same resolution each time
- Line 13: Local commit 93c9e5f is 1 ahead, 80 behind — the 80 remote commits include merged PRs and learnings; the local commit duplicates the remote's guard refresh with extra chore artifacts

## Details
On this fork, acx/dev tracks origin/acx/dev. When local commits are stale (behind by many remotes, duplicating remote changes), the correct workflow is:
1. `git fetch --all --prune` to sync both origin and upstream
2. `git rev-list --left-right --count acx/dev...origin/acx/dev` to see ahead/behind counts
3. `git log HEAD..origin/acx/dev` to inspect remote-only commits
4. `git log origin/acx/dev..HEAD` to inspect local-only commits
5. If local commits are stale/redundant, `git reset --hard origin/acx/dev`
6. Sync main branch: `git branch -f main origin/main` (or `git branch --track` if main doesn't exist locally)

This avoids rebase conflicts on redundant changes and ensures the working tree is fresh for subsequent work (e.g., a setup-repo session that makes new changes and pushes them).

## How to Apply
Before starting a setup session or when a push is rejected:
```bash
git fetch --all --prune
git rev-list --left-right --count acx/dev...origin/acx/dev  # See counts
git log --oneline origin/acx/dev..HEAD                      # Inspect local-only
if [ "$(git rev-list --count origin/acx/dev..HEAD)" -gt 0 ]; then
  # Review local commits; if stale, reset
  git reset --hard origin/acx/dev
fi
git branch -f main origin/main                              # Sync main
```

Related: [[fork-topology-worktree-isolation]]
