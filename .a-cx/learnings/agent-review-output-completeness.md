---
id: agent-review-output-completeness
trigger: "when resuming an agent for a second or third restatement of findings"
confidence: 0.85
domain: workflow
scope: project
date: 2026-07-27
---

# Agent Review Output Must Be Complete and Verbatim

## Action
When requesting code review or security review findings from agents, require they output the complete verbatim findings (severity, file:line, title, evidence, fix, scope + rationale for each) rather than meta-summaries that reference earlier messages.

## Evidence
- Session 9963b560: User requested security review findings for PR #17, agent returned summary; user resent message asking for "complete final findings... in full"
- Same session: User requested code review findings, agent again returned summary-style; user explicitly corrected: "I still need the actual verbatim content of your findings, not a meta-summary referring back to 'my previous message above'"
- Pattern repeated twice on the same PR (security + code review phases), indicating a systematic correction across review agents
- Root cause: agents assume prior context is visible to the user, but output is isolated per agent

## How to apply
Before accepting a restatement from a review agent (especially on second/third iterations), verify it contains full finding details, not references. If you get a summary, use SendMessage to clarify: "print out in full right now: each finding with severity, file:line, title, evidence, fix, and scope tag + rationale."
