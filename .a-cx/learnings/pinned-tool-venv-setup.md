---
id: pinned-tool-venv-setup
trigger: "when a tool version is pinned in CI but not available locally"
confidence: 0.80
domain: workflow
scope: project
date: 2026-09-04
---

# Extract pinned tool version from CI and mirror locally in a venv

## Action
When CI pins a tool version (e.g., clang-format 23.1.0 in the workflow), extract that version spec, create a local venv with the same exact version, and use it for pre-flight checks before pushing.

## Evidence
- Observed in session 61 (lines 22–27)
- Pattern: grep workflow → pip install exact version → run tool locally
- Applied to: clang-format 23.1.0 (23.1.0 from .github/workflows/acx-dev-ci.yml)
- Pre-flight check caught no formatting issues; CI passed on first try

## Why
- Mismatched versions can pass locally but fail CI
- Venv isolation keeps the pinned version separate from system tools
- Running locally before push catches format drift early
- Documentation in CI workflow is the source of truth; extract and mirror it
