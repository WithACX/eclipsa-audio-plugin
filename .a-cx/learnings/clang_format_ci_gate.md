---
id: clang-format-ci-gate-requirement
trigger: "when pushing code that fails clang-format check in CI"
confidence: 0.8
domain: code-style
scope: project
date: 2026-07-22
---
# Clang-format CI gate requires local fixes before push

## Action
Run `clang-format --style=file:.clang-format -i` on changed .h and .cpp files locally, verify with `--dry-run -Werror`, and commit the formatting changes before pushing to fix CI failures.

## Evidence
- Observed in PR #17 (38-audio-video-duration-mismatch-warning branch): Clang Format check FAILED in CI
- User fetched remote, fast-forwarded, and ran clang-format locally on WavFileOutputProcessor.h and WavFileOutputProcessor.cpp
- Result: 2 files required formatting fixes (line wrapping on comments)
- Pattern: formatting failures appear after merging acx/dev into feature branches, suggesting upstream may have different formatting state

## Related
- [[build_test_before_pr]] — formatting check is part of the pre-push verification cycle
