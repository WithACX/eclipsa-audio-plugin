---
id: grep-before-edit-workflow
trigger: "when about to modify code in this codebase"
confidence: 0.8
domain: workflow
scope: project
date: 2026-07-24
---

# Use grep to Explore Related Code Before Editing

## Action
Search the codebase with grep to understand what code patterns already exist before making targeted edits. This reveals scope, consistency, and ripple effects.

## Evidence
- Observed 6 times in session 8189b128 (events 4, 6, 8, 15, 19-20)
- Pattern: grep for test name to locate source → grep for error patterns to find all usages → grep for related feature flags to understand scope → then edit with full context
- Example: before editing the `mux_iamf_lpc_1ae_1mp` test, user grepped for `getExportError() == kNoError` across the codebase to understand where similar assertions existed
- User grepped for `exportVideo = true` to find related tests before finalizing changes

## Why
This codebase is large (~1100 test file lines, many processor files). Grep-first prevents making isolated edits that don't account for related code, inconsistent patterns, or missing assertions elsewhere.
