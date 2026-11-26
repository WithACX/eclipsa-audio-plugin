---
id: gh-pass-explicit-repo-flag
trigger: "when running gh issue, gh pr, or gh project commands against a WithACX repo from this checkout"
confidence: 0.7
domain: tooling
scope: project
date: 2026-11-26
---
# Pass `-R` Explicitly on Every gh Call

## Action
Pass `-R WithACX/<repo>` on every `gh` command in this checkout, because the local default repo points at the upstream google repo.

## Evidence
- Observed 9 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599 (2026-11-25 to 2026-11-26)
- Pattern: Every `gh issue view`, `gh pr list`, `gh pr view`, and `gh label list` call in the session carried an explicit `-R WithACX/eclipsa-audio-planning` or `-R WithACX/eclipsa-audio-plugin`. The autonomous-run dispatch prompt stated the reason directly: the local gh default points at the upstream google repo.
