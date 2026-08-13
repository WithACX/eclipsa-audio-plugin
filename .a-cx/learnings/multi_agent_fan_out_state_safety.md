---
id: multi-agent-fan-out-state-safety
trigger: "when running multi-agent fan-out work (review, verification agents in parallel)"
confidence: 0.85
domain: process
scope: project
date: 2026-07-27
---

# Snapshot Git State Before and After Multi-Agent Fan-Out

## Action
Before spawning multiple agents in parallel, snapshot the local git state (staged/unstaged/untracked files, HEAD, current branch) and the remote `main` tip via `git ls-remote`. After the agents complete, re-verify the state matches to detect any unexpected side-effects or state mutations (CHD-04 pattern).

## Evidence
- Observed 3+ times in session 0ec3a889 (PR #17 review, events 21-22 and 50-51): user ran bash script to save `git status --porcelain=v1`, `git rev-parse HEAD`, `git rev-parse --abbrev-ref HEAD`, and `git ls-remote origin refs/heads/main` BEFORE fan-out, then re-ran the same commands AFTER agents completed, comparing outputs: "OK: local state unchanged" / "OK: remote main unchanged"
- Pattern: fan-out spawns 3 agents (code-reviewer, security-reviewer, cpp-reviewer) reading the repo but not modifying it; state snapshot guards against any hidden mutations or race conditions
- Root: agents run in background with git-tool access; snapshot provides early warning if an agent accidentally commits, modifies files, or pushes

## How to apply
**Before spawning agents:**
```bash
state_before=$(git status --porcelain=v1 --untracked-files=all | sort; git rev-parse HEAD; git rev-parse --abbrev-ref HEAD)
remote_before=$(git ls-remote origin "refs/heads/main" | cut -f1)
```

**After agents complete:**
```bash
state_after=$(git status --porcelain=v1 --untracked-files=all | sort; git rev-parse HEAD; git rev-parse --abbrev-ref HEAD)
remote_after=$(git ls-remote origin "refs/heads/main" | cut -f1)
if [ "$state_after" = "$state_before" ]; then echo "OK: local state unchanged"; else echo "ALERT: local state changed"; fi
if [ "$remote_after" = "$remote_before" ]; then echo "OK: remote main unchanged"; else echo "ALERT: remote main moved"; fi
```

Abort or investigate if either check fails, rather than proceeding blindly.
