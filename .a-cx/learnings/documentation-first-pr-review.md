---
id: documentation-first-pr-review
trigger: "before launching deliver-pr-review agents or reviewing a PR"
confidence: 0.75
domain: workflow
scope: project
date: 2026-07-27
---

# Documentation-First PR Review Pattern

## Action
Read the current PR review policy and project configuration files (pr-review-policy.md, process.yaml, AGENTS.md) before launching any specialized review agents or automated PR assessment.

## Evidence
- Observed 5 times in session 9963b560 (2026-07-27):
  - read review-workflow.md (understand the review orchestration)
  - read pr-review-policy.md (understand merge conditions, scope rules, severity thresholds)
  - read process.yaml (understand project's pr_review setting: auto vs human vs both)
  - read model-routing.yaml (understand which agents/models are used for which task classes)
  - read AGENTS.md (understand project context, branch topology, CI rules)

## Why
The deliver-pr-review skill uses configurable policies that change across projects and sessions. Scope rules, merge conditions, severity thresholds, and model tier assignments are all in external configs. Reading the current state avoids:
- Assuming a finding blocks merge when it doesn't (scope-anchored review misunderstanding)
- Launching agents with wrong severity expectations
- Not understanding first-error-wins vs. independent-error handling rules
- Missing project-specific constraints (clang-format CI gate, worktree isolation, tdd flag)

The five reads represent distinct config layers that collectively describe the review context.

## How to apply
Before running `/deliver-pr-review <PR>` or `/deliver-code` with review enabled:
1. Read `.a-cx/process.yaml` (pr_review setting, tdd flag, isolation mode)
2. Read the current A-CX pr-review-policy.md from the active a-cx-ai-config plugin (merge thresholds, scope rules)
3. Skim AGENTS.md if the project is unfamiliar (branch topology, CI blockers, coding standards)

If reviewing a PR from a different org repo (not withACX), also check the target repo's CLAUDE.md for any local overrides.
