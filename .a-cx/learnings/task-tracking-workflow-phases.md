---
id: task-tracking-workflow-phases
trigger: "when executing multi-phase delivery workflows"
confidence: 0.75
domain: process
scope: project
date: 2026-07-25
---
# Task-Based Phase Tracking for Delivery Workflows

## Action
Create a TaskCreate for each major workflow phase and update its status (pending → in_progress → completed) as you progress, providing real-time tracking visibility of multi-step tasks and preventing phase skips.

## Evidence
- Observed 5+ times in session be6aa039: discrete tasks created and status-tracked for build, test, format, commit, push phases (TaskCreate/TaskUpdate events 12–40)
- Observed 10+ times in session 2e881e7f: review workflow phases tracked with TaskCreate and TaskUpdate lifecycle management (events 71–80)
- Pattern: Multi-phase delivery work is deliberately broken into task checkpoints; each is marked in_progress when started and completed when finished, creating an audit trail for which phases were executed
