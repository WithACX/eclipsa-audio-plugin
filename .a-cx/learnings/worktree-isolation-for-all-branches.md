---
id: worktree-isolation-for-all-branches
trigger: "when starting work on any new branch"
confidence: 0.9
domain: process
scope: project
date: 2026-07-21
---
# Always Use Git Worktree for Branch Isolation

## Action
Always create a git worktree for every branch, even lightweight no-PR fixes. Never edit directly in the primary working tree.

## Evidence
- Documented in memory file feedback_worktree_isolation_always.md (user preference)
- Observed in every session window: all work uses .a-cx/worktrees/<branch-name>/ paths
  - Session starting upstream-47-wav-writer-export-failures creates worktree ../eclipsa-audio-plugin-upstream-pr-14, then cleans it up (line 16)
  - Session for issue #43 uses .a-cx/worktrees/43-smpte-dolby-speaker-labels (lines 100+)
  - Session shows worktrees 42, 43, 47 all coexist (line 124)
- Pattern is consistent across all observed sessions
- Primary tree remains stable for context switching and parallel worktrees

## Why
Worktree isolation prevents:
- Accidental commits to the wrong branch
- Cross-branch build pollution (separate build/ directories per worktree)
- Conflicts between concurrent work
- State confusion when switching contexts
- Loss of in-progress work if a checkout fails

The cost (disk space for multiple build/_deps) is acceptable given the safety and parallelism gains.

## How to apply
```bash
git worktree add .a-cx/worktrees/<branch-name> <base-branch>
cd .a-cx/worktrees/<branch-name>
# ... do work, build, test, PR ...
cd /path/to/primary
git worktree remove .a-cx/worktrees/<branch-name>
```
Never skip this step, even for "quick" changes.
