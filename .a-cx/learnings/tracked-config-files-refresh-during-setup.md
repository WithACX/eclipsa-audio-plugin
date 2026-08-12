---
id: tracked-config-files-refresh-during-setup
trigger: "when running /setup-repo and checking for stale tracked files in .a-cx/bin and .a-cx/hooks"
confidence: 0.80
domain: process
scope: project
date: 2026-08-12
---

# Tracked Config and Script Refresh: .a-cx/bin/ and .a-cx/hooks/ Stay in Sync with Plugin Assets

## Action
During /setup-repo, verify that all files in .a-cx/bin/ and .a-cx/hooks/ match the bundled plugin assets. Refresh stale copies and commit the changes as part of the setup commit.

## Evidence
- Event logs from sessions 40776e8d, 99d31894 show **3+ instances** of refresh operations:
  - Line 5-6: Comparing remote .a-cx/bin/check_upstream_leak.py (168 lines stale) vs current asset (460 lines)
  - Line 40-42: Comprehensive diff of all installed files against bundled assets; identified DRIFT in 2 files
  - Line 52-53: Copying assets to both .a-cx/hooks/pre-push and .a-cx/bin/check_upstream_leak.py, then verifying with cmp
  - Line 60-61: Staging the refreshed files as part of the setup commit
- Pattern: Tracked copies (`check_upstream_leak.py`, `acx-upstream-pr.sh`) fall behind plugin assets and need refreshing each update cycle

## Details
Files in `.a-cx/bin/` and `.a-cx/hooks/` are **tracked in git** (committed to the repo) to:
1. Back the classify subcommand used by `/upstream-pr`
2. Provide a fallback when the .git/hooks copy is cleared during worktree operations
3. Document what version of the guard and tooling the repo is pinned to

However, these tracked copies are **generated** — they are copied from the plugin assets at /setup-repo time. When the plugin ships a new version (e.g., updated guard with new DEFAULT_INTERNAL_PATHS or new acx-upstream-pr.sh with --reviewer flag), the tracked copies become stale and must be refreshed.

The refresh is not automatic; /setup-repo must:
1. Copy the current bundled asset to the tracked location
2. Make the file executable if needed (chmod +x)
3. Stage and commit the changes

Failure to refresh means:
- The classify subcommand sees outdated guard rules
- /upstream-pr reports stale internal-path classification
- The mixed-commit reminder sees old path definitions
- If a worktree drops the .git/hooks copy, the fallback to `.a-cx/hooks/pre-push` runs stale code

## How to Apply
During or after /setup-repo:
1. Identify the plugin root (where the bundled assets live, typically under ~/.claude/plugins/cache/a-cx-ai-config/)
2. For each file in .a-cx/bin/ and .a-cx/hooks/:
   ```bash
   cp $PLUGIN_ROOT/skills/skill-lifecycle/assets/hooks/check_upstream_leak.py .a-cx/hooks/pre-push
   cp $PLUGIN_ROOT/skills/skill-lifecycle/assets/hooks/check_upstream_leak.py .a-cx/bin/check_upstream_leak.py
   cp $PLUGIN_ROOT/skills/skill-lifecycle/assets/acx-upstream-pr.sh .a-cx/bin/acx-upstream-pr.sh
   chmod +x .a-cx/hooks/pre-push .a-cx/bin/*.py .a-cx/bin/*.sh
   ```
3. Verify: `cmp -s $PLUGIN_ROOT/.../check_upstream_leak.py .a-cx/bin/check_upstream_leak.py && echo OK`
4. Stage and commit as part of the chore(a-cx): setup commit

Related: [[fork-guard-asset-sync]], [[task-driven-setup-workflow]]
