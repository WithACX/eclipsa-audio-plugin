---
id: task-driven-setup-workflow
trigger: "when running /setup-repo or orchestrating multi-step repository configuration"
confidence: 0.75
domain: workflow
scope: project
date: 2026-08-12
---
# Task-Driven Setup Workflow Pattern

## Action
Break complex setup tasks into discrete, tracked steps using TaskCreate → TaskUpdate (in_progress/completed) to manage visibility and prevent missed steps.

## Evidence
- Observed 8+ times across two sessions (a75b1823, 275d572e)
- Pattern: `/setup-repo` orchestrated as series of TaskCreate calls (1-8), each marking status transitions
- First session: Tasks 1-8 (branches, settings, AGENTS.md, process.yaml, pre-push guard, commit, PR, planning repo)
- Second session: Tasks 1-3 (planning repo creation, provisioning, board verification)
- Each task followed TaskUpdate(in_progress) → work → TaskUpdate(completed) or left pending for manual steps

## Why This Matters
On external forks with complex branch topology and dual repos (public code fork + private planning repo), a task list:  
- Makes the setup procedure transparent and re-runnable
- Flags which steps are automated vs. manual (gh API, user UI clicks)
- Prevents accidental loss of setup context when the main workflow pauses

## How to Apply
When running `/setup-repo` in a new project, expect TaskCreate calls for:
1. Branch creation (acx/dev off main)
2. Plugin configuration (.claude/settings.json)
3. Instruction generation (AGENTS.md, overlays)
4. Process config (.a-cx/process.yaml)
5. Git hooks (pre-push guard)
6. Tooling commit and PR
7. Planning repo creation
8. Planning repo provisioning (labels, board link)

Track manual steps separately (UI clicks, approval gates) and document them in a post-setup summary.
"
