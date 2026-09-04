---
id: worktree-experiment-isolation-pattern
trigger: "when testing changes to a branch, running diagnostics, or trying multiple solutions without polluting main checkout"
confidence: 0.82
domain: workflow
scope: project
date: 2026-09-02
---

# Use Worktrees to Isolate Experimental Branches, Test Locally, Then Clean

## Action
Create a detached worktree for branch experiments (CI debugging, formatting, test cycles) to avoid blocking main checkout. Verify changes locally, then delete the worktree when done.

## Evidence
- Observed 3 times in session 1c15f3d8:
  - fmt-fix worktree (lines 9-10): diagnostic on upstream PR branch
  - pin-clang-format worktree (lines 45-46): draft solution branch
  - cleanup (line 88): removal after push

## Pattern

**Create for branch work:**
```bash
# Diagnostic worktree (read-only, no commits)
git worktree add --detach .a-cx/worktrees/fmt-fix origin/feat/top-down-panner-view
cd .a-cx/worktrees/fmt-fix

# Authoring worktree (intended for commits)
git worktree add .a-cx/worktrees/pin-clang-format -b ci/pin-clang-format-23 origin/acx/dev
cd .a-cx/worktrees/pin-clang-format
```

**Restore LFS state if needed (for read-only diagnostics):**
```bash
# After checking out, restore original state if reading but not modifying
git diff --name-only | while read -r f; do
  git cat-file -p "HEAD:$f" > "$f" 2>/dev/null && \
  git update-index --assume-unchanged "$f"
done
```

**Clean up when done:**
```bash
git worktree remove --force .a-cx/worktrees/fmt-fix
git worktree remove --force .a-cx/worktrees/pin-clang-format
```

## Why This Workflow

- **Main checkout stays usable:** Can switch branches, pull, edit while worktree is active
- **Clean isolation:** Each worktree has its own git state, working directory, uncommitted changes
- **Fast branch testing:** Create, test, commit, verify, push in worktree; destroy when satisfied
- **LFS-safe:** Worktrees can have different LFS states without conflicts with main checkout

## Gotchas

- `git worktree remove` fails if the worktree is dirty; use `--force` to skip that check
- Worktrees and main checkout **share the same git object store** (.git/objects); disk usage is only for the new checkout
- `--detach` prevents accidental commits and forces you to read-only explore; use a `-b branch_name` to author instead
- Background jobs in a worktree may not play well with main checkout operations (rare but possible)

## Common flows observed

**Diagnostic-only (read-only):**
1. Create detached worktree on upstream branch
2. Run commands, grep logs, extract data
3. Use find/grep to understand failures
4. Remove worktree

**Authoring a fix:**
1. Create worktree on a new `-b branch_name`
2. Make commits, test locally (build, format check, ctest)
3. Push the branch
4. Open PR
5. Remove worktree once PR is healthy

## When to use
- Debugging a remote PR without checking it out locally
- Drafting a fix on a new branch while keeping main clean
- Running intensive CI simulations (ctest, full build) without blocking main
- Comparing two branch states side-by-side

Related: [[ci-simulation-yaml-extraction-pattern]], [[clang-format-pinned-23-1-0]]
