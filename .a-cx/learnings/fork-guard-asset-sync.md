---
id: fork-guard-asset-sync
trigger: "when updating or validating the upstream leak guard on an external fork"
confidence: 0.78
domain: process
scope: project
date: 2026-08-12
---

# Fork Guard Bundled-Asset Synchronization

## Action
Keep `.a-cx/bin/check_upstream_leak.py` and `.a-cx/hooks/pre-push` in sync with the bundled guard shipped by the plugin, and validate that scm.internal_paths covers all A-CX-internal workflows.

## Evidence
- Event logs from sessions 40776e8d, 99d31894 show **3+ separate checks** comparing installed guard versions:
  - Line 5-6: Remote .a-cx/bin/check_upstream_leak.py vs shipped asset (296 → 460 lines)
  - Line 40-42: Comparing all copies (.git/hooks, .a-cx/bin, .a-cx/hooks) against bundled assets
  - Line 52-53: Refreshing stale tracked copies from current plugin assets
- Line 50-59: Validation that scm.internal_paths includes `.github/workflows/acx-dev-ci.yml` so A-CX CI files don't ride upstream
- Pattern: Guard updates ship in plugin releases; tracked copies drift if not refreshed during /setup-repo

## Details
The guard (check_upstream_leak.py) has two installed locations on this fork:
1. `.git/hooks/check_upstream_leak.py` — live, invoked by pre-push hook
2. `.a-cx/bin/check_upstream_leak.py` — tracked copy, backs /upstream-pr classify subcommand
3. `.a-cx/hooks/pre-push` — alternate tracked copy (less commonly used)

All three should match the bundled `assets/hooks/check_upstream_leak.py` from the current plugin version. When the plugin ships a guard update (e.g., new DEFAULT_INTERNAL_PATHS, improved path matching), the tracked copies go stale and the classify subcommand sees outdated behavior.

Additionally, A-CX-specific .github/workflows files (e.g., `acx-dev-ci.yml`) are **not** in the guard's built-in DEFAULT_INTERNAL_EXACT list, so they must be added to scm.internal_paths in .a-cx/github.yaml, or they would be pushed to upstream (google/eclipsa-audio-plugin).

## How to Apply
When running or updating /setup-repo on this fork:
1. After refreshing assets, compare installed copies: `cmp assets/hooks/check_upstream_leak.py .a-cx/bin/check_upstream_leak.py`
2. If drift is found, copy from the bundled asset and re-validate
3. In .a-cx/github.yaml, add scm.internal_paths for any A-CX-specific .github/workflows/* files
4. Validate guard coverage: `python3 -c 'import pathlib, sys; sys.path.insert(0, ".git/hooks"); from check_upstream_leak import load_guard_config; paths, _ = load_guard_config(pathlib.Path(".")); print(any(f in paths for f in [".github/workflows/acx-dev-ci.yml"]))'`

Related: [[fork-topology-worktree-isolation]]
