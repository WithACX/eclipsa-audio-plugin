---
id: review-agent-findings-completeness-required
trigger: "when receiving findings from review agents (code-reviewer, security-reviewer, cpp-reviewer)"
confidence: 0.85
domain: process
scope: project
date: 2026-07-27
---

# Require Full Verbatim Findings from Review Agents, Not Summaries

## Action
When requesting code review, security review, or C++ review findings from agents, explicitly require they output the complete verbatim findings (severity, file:line, title, evidence, fix, scope + causation rationale for each) rather than meta-summaries that reference earlier messages or claim "findings above" without restating them.

## Evidence
- Observed 4+ times in session 0ec3a889 (events 30-32, PR #17 security/code/C++ reviews): agents returned summaries like "scripts/ note doesn't apply" or "my review findings from the previous message stand as the deliverable" instead of full findings text
- User had to explicitly extract findings from transcripts using Python parsing (events 42-49) to recover the actual content, then cross-check completeness against the learned pattern from agent-output-verification-via-transcript.md (event 35-36)
- Root cause: agents assume prior context is visible to the user, but each agent's output is isolated in its own transcript block; "see my findings above" means nothing when the user is reading a fresh agent completion message without the prior context visible

## How to apply
In the prompt to review agents, **always** include explicit requirement language:
> "Report your full findings verbatim -- do not summarize or truncate. For each finding report: severity (CRITICAL/HIGH/MEDIUM/LOW), file, line, title, evidence (the actual problematic code/reasoning), suggested fix, scope tag (in-scope/adjacent), and the causation rationale. If you find nothing at a given severity, say so explicitly."

If you receive a restatement that lacks full details, use a follow-up message:
> "I still need the actual verbatim content of your findings, not a meta-summary. Print out in full right now: each finding with severity, file:line, title, evidence, fix, and scope tag + rationale."

Then extract from transcript if needed (per multi-agent-review-transcript-extraction.md).
