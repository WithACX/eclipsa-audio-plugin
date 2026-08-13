---
id: parallel-review-agent-output-truncation
trigger: "when running 3+ large parallel review agents synchronously in PR review workflow"
confidence: 0.85
domain: workflow
scope: project
date: 2026-07-30
---

# Parallel Review Agent Output Truncation

## Action
When running 3+ review agents in parallel (code-reviewer, security-reviewer, cpp-reviewer), expect main-turn output truncation; recover the full findings by resuming each agent via SendMessage with their agentId.

## Evidence
- Observed 3 times in session acbba4eb-1c42 (PR #17 review, 2026-07-30)
- Pattern: Three agents spawned synchronously (lines 30-32 of observer log), completed with status=completed (lines 33-35), but main-loop received only trailing note about scripts/ folder
- Recovery: User sent SendMessage to each agent ID (a902dc42c92b680b9, a4d7980179a932d6a, a2a2af52fdc7472d8) requesting full verbatim findings
- Each agent successfully resumed and output to .output JSONL file per SendMessage confirmation

## Why
Large agent outputs (1500+ diff lines, 261 test cases) combined with 3 parallel agents exceed the main-turn context window; the harness truncates but agents complete fully to their .output files.

## How to Apply
1. After running 3+ parallel review agents synchronously, check if output is truncated (look for trailing notes without the findings table)
2. Rather than waiting/polling, immediately send SendMessage to each agent with their agentId requesting the complete findings repost
3. Agents resume from transcript and output to .output file; poll that file or schedule a wakeup to collect results
4. Parse the .output JSONL directly if needed (alternative to agent resumption via SendMessage)

See also: [[multi_agent_review_transcript_extraction]], [[agent-review-output-completeness]]
