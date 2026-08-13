---
id: multi-agent-review-transcript-extraction
trigger: "when running multi-agent fan-out reviews (code + security + C++ in parallel)"
confidence: 0.85
domain: workflow
scope: project
date: 2026-07-27
---

# Multi-Agent Review Findings Extraction via Transcript Parsing

## Action
After spawning parallel review agents (code-reviewer, security-reviewer, cpp-reviewer), extract their full findings from the session transcript files using Python JSON parsing rather than relying on agent-provided summaries in main-loop notifications.

## Evidence
- Observed 5+ times in session 0ec3a889 (PR #17 review): agent outputs saved to `/private/tmp/claude-504/.../0ec3a889-cccd-48ae-a115-24916aaf223b/tasks/{agentId}.output` as JSONL files; user parsed with `python3 -c "import json; [...]"` to extract assistant text blocks (events 42-49)
- Pattern: agents complete work, but main-loop notification is truncated or missing; user locates `.output` file, parses lines with `json.loads()`, finds large text blocks (>400 chars), extracts `message.content[].text` from assistant blocks
- Confirmed reuse from learning at event 35-36 (agent-output-verification-via-transcript.md, session 2e881e7f): documented this same extraction pattern used 4+ times
- Root: agent-authored summaries or referential language ("findings above") lose detail; direct transcript access recovers the complete findings

## How to apply
After a multi-agent review fan-out completes:
1. Locate the agent output file: `/private/tmp/claude-504/-Users-branden-Documents-.../tasks/{agent_id}.output` 
2. Parse with Python: `json.loads(line)` for each newline, find assistant-type blocks with `message.content[i].text`
3. Index blocks by position; large blocks (>400 chars) are likely full findings reports
4. Print target block(s) to verify findings are present in full before accepting them

Example command:
```bash
python3 - <<'EOF'
import json
with open("tasks/{agent_id}.output") as f:
    for i, line in enumerate(f.readlines()):
        obj = json.loads(line)
        if obj.get("type") != "assistant": continue
        for block in obj["message"].get("content", []):
            if block.get("type") == "text" and len(block["text"]) > 400:
                print(f"--- line {i}, len {len(block['text'])} ---")
EOF
```
Then extract the target line with a follow-up script indexing that line number.
