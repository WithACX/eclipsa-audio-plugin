---
id: deferred-tool-schema-loading-pattern
trigger: "when using deferred tools (SendMessage, TaskUpdate, TaskOutput, etc.) in multi-agent workflows"
confidence: 0.8
domain: workflow
scope: org-wide
date: 2026-07-25
---

# Deferred Tool Schema Loading Pattern

## Action
Before invoking a deferred tool (one that appears in `<system-reminder>` but is not pre-loaded), first call ToolSearch with the exact tool name as `select:ToolName` to load its schema, then proceed with the tool invocation.

## Evidence
- Observed 3+ times in session 2e881e7f (events 1-2, 3-4, 11-12): ToolSearch calls with `select:TaskOutput`, `select:SendMessage`, `select:TaskUpdate` immediately before invoking those tools
- Pattern: Each deferred tool name matches exactly one ToolSearch query; a single successful ToolSearch call precedes each tool invocation
- Root: Deferred tools have their schemas loaded on-demand; the harness will InputValidationError if a deferred tool is called directly without first loading its schema via ToolSearch

## How to apply
Maintain a pattern: before calling any deferred tool (identified by name in the initial system-reminder list), invoke `ToolSearch` with `query: "select:ToolName"` first. For multiple deferred tools, batch them: `ToolSearch(query: "select:Tool1,Tool2,Tool3")` in a single call, then invoke the tools.
