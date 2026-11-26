---
id: operator-waived-ci-local-verification
trigger: "when CI on acx/dev is known broken and the operator has waived the cancelled or missing checks for a PR"
confidence: 0.4
domain: testing
scope: project
date: 2026-11-26
---
# Verify Locally When CI Is Operator-Waived

## Action
Before review and merge, run a clean build and the full `ctest` suite serially (not with `-j`), and run `clang-format==23.1.0 --dry-run -Werror` on the changed files, then record both results in the PR.

## Evidence
- Observed 3 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599 (2026-11-25 to 2026-11-26)
- Pattern: The operator waived the always-cancelled `Clang Format` job and the missing `macos-14`/`windows-2022` contexts. Each unit dispatch and the PR #67 review carried the same instruction to verify locally in place of CI. Documentation-only PRs skipped the build and stated that they touched no C++ or CMake.
- Scope note: the waiver came from the operator for this run. Re-confirm it before applying it to another run.
