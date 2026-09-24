---
id: acx-leak-guard-workflow
trigger: "when capturing internal A-CX tools into .a-cx/"
confidence: 0.75
domain: process
scope: project
date: 2026-09-04
---

# A-CX Leak Guard: Automatic Capture and Classification Workflow

## Action
Use `capture_internal_artifacts.py` from `.a-cx/bin/` to auto-capture A-CX-only files into the guarded bucket, then verify commit classification with `check_upstream_leak.py classify <commit>` to confirm `config-only` status before merging.

## Evidence
- Observed 3 times in session 736b1387 (eclipsa-audio-plugin chore/move-uninstall-script-to-acx-bin):
  - Event 9-10: Captured `uninstall_dev_plugins.sh` into `.a-cx/bin/`, classified as `chore(a-cx): capture 1 internal work-product file`
  - Event 11-12: Verified commit `aff40b78...` classification: `config-only` → safe to merge upstream
  - Event 24-25: Re-checked guarded bucket in primary clone; all captured files accounted for
- Pattern: Always pair capture with leak-guard classification check
- Used when: Moving A-CX dev helpers from `scripts/` (upstream-owned) to `.a-cx/bin/` (guarded)

## Why
- A-CX internal tools must never ride into upstream PRs or main branch
- `.a-cx/` is guarded by path in `check_upstream_leak.py` so files are automatically classified `config-only`
- `scripts/` is upstream-owned (mirrors Google's eclipsa-audio-plugin), so A-CX tools must leave it
- Automatic capture ensures tools exist on all developer machines, not just one

## How to Apply
1. Always place A-CX developer utilities in `.a-cx/bin/` or `.a-cx/config/`, not `scripts/`
2. After moving or creating a file in `.a-cx/`, run `capture_internal_artifacts.py`
3. Before merging a PR containing new `.a-cx/` files, run `check_upstream_leak.py classify <commit>` and verify `config-only`
4. Confirm `.a-cx/bin/` is un-ignored in `.gitignore` (`!.a-cx/bin/`) so files are genuinely tracked

[[file-integrity-verification-on-moves]] [[bash-syntax-check-on-scripts]]
