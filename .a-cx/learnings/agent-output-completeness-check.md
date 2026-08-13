---
id: agent-output-completeness-check
trigger: "when delegating large inventory or research tasks to agents"
confidence: 0.8
domain: workflow
scope: project
date: 2026-07-21
---
# Agent Output Completeness Check

## Action
Verify that agent final output includes the full inventory/list inline; do not accept references to "sections above" or summaries that claim to have listed something without showing it.

## Evidence
- Observed 4+ times in session e5d2fc75-c97e-4f3d-b9fd-992653b0f32c
- Pattern: Agent returns final message saying "I listed X items above" or "see sections A-F", but the actual output contains no such list or only shows fragments
- User corrects via SendMessage each time: "I cannot see any 'above' content — I only receive whatever text you output in your final reply"
- Affected task: speaker-position label inventory for issue #43
- Agent a0e16a937aea24ba4 required 2+ correction cycles before delivering full inline output

## Why
Agents sometimes provide incomplete final summaries when they've done research across multiple tool calls. Their summary may reference intermediate findings they "saw" earlier in their reasoning, but the final text output must be a complete standalone deliverable. The harness only captures the final text, not the reasoning chain.

## How to apply
When delegating inventory, catalog, or exhaustive-list tasks: explicitly instruct the agent that the final response must be the FULL output (not a summary or reference), stated inline with no forward-references to unreachable sections. For large outputs, say "if it's long, that's fine — write it all out completely."
