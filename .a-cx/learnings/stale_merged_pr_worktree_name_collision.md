---
id: stale_merged_pr_worktree_name_collision
trigger: "when git or a script reports a branch name already exists locally after a PR is merged"
confidence: 0.75
domain: git
scope: project
date: 2026-07-30
---
# Merged PR worktree/branch names can collide with new upstream contributions

## Action
When a merged fork PR has a stale `.a-cx/worktrees/<name>` and local branch `<name>` still lingering post-merge, and a new upstream contribution needs that same branch name, remove the stale pair cleanly: `git worktree remove .a-cx/worktrees/<name> --force` to delete the worktree (safe because the PR is merged and no work is at risk), then `git branch -D <name>` to delete the local tracking branch. Only safe when the branch is fully merged into `acx/dev`; verify with `git merge-base --is-ancestor <name> acx/dev` first (exit code 0 = merged, safe to delete).

## Evidence
- Observed 2026-07-30 when preparing to contribute PR #17 upstream: the branch `38-audio-video-duration-mismatch-warning` existed locally from that PR's development, and the upstream-pr script wanted to create a same-named branch. Branch was confirmed merged into acx/dev (commit `81d6210` was an ancestor of acx/dev HEAD), so deletion was safe.
- Recovery: `git worktree remove .a-cx/worktrees/38-audio-video-duration-mismatch-warning --force` succeeded, then `git branch -D 38-audio-video-duration-mismatch-warning` deleted the branch. Name was then reusable.
- Related: this only becomes an issue when a PR is merged *and* the feature branch is deleted from the fork's PR state (auto-cleanup on merge), leaving a stale local worktree behind. Later attempts to create an upstream contribution with that merged PR's name will fail with "branch already exists" errors.
