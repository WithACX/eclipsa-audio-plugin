---
id: agent-output-verification-via-transcript
trigger: "when verifying agent completeness in multi-agent review workflows"
confidence: 0.75
domain: process
scope: project
date: 2026-07-25
---

# Transcript-Based Agent Output Verification

## Action
After agents complete findings reports (especially in fan-out reviews), extract and index their raw transcript blocks via `jq`/`json` parsing to verify all findings are present and in expected format, rather than relying on agent-authored summaries.

## Evidence
- Observed 4+ times in session 2e881e7f (events 19-22, 26-30): bash calls with python3 `json.loads()` parsing of agent `.output` files to extract assistant text blocks, indexing by block number, then printing specific blocks to validate completeness
- Reconfirmed in session 408bafdc-2e9e-4a55-ba25-658940b8db90 (2026-07-27, PR #17 security review):
  - Event 8-9: Python script scanning `a8448472e2fd45f4f.output` for assistant text blocks >300 chars, found blocks at lines 69, 70, 73, 74
  - Event 9-10: User extracted line 73 (8062 bytes) — the full security report
  - Event 5-6: SendMessage to agent requesting "repost full security findings report" after notifications didn't transmit it
- Pattern: Agent work is complete and in the transcript, but main-loop notification is missing or truncated; user uses Python `json.loads()` + parsing to recover the actual findings
- Root: Ensures that when agents reference "findings above" or use referential language, the user can directly access the complete text rather than relying on the agent's characterization

## How to apply
When a multi-agent review completes and agents are resumed for clarification: parse their `.output` JSON file (`tasks/agent_id.output`) with a python script reading `message.content[].text` from assistant roles, index the blocks, and print the target block to confirm the findings are present in full. Path pattern: `/private/tmp/claude-504/-Users-branden-Documents-Code-Automated-eclipsa-audio-plugin/{session_id}/tasks/{agent_id}.output`
