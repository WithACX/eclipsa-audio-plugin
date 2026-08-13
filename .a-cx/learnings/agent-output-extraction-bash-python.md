---
id: agent-output-extraction-bash-python
trigger: "when parallel review agents truncate findings and recovery is needed"
confidence: 0.9
domain: workflow
scope: project
date: 2026-07-30
---

# Agent Output Extraction via Bash + Python JSON

## Action
When parallel agents produce truncated findings, extract full text from task `.output` JSONL files using `wc -l` + `cat | python3` JSON parser instead of SendMessage resumption.

## Evidence
- Session d6d4fc9d-efb6-4450-aa9a-c9540e254536 (PR #17 review, 2026-07-30)
- Pattern executed 3 consecutive times (events 1-4, 5-8, 9-12)
- Recovered security review findings (55 lines)
- Recovered C++ review findings (86 lines) 
- Recovered verification review findings (130 lines)

## How to Apply

When main-turn summary is truncated after running 3+ large review agents:

1. Identify the task .output file path from the truncated turn
2. Count lines: `wc -l "<task.output>"`
3. Extract all ASSISTANT TEXT blocks:
```bash
cat "<task.output>" | python3 -c "
import json, sys
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    try:
        obj = json.loads(line)
    except Exception:
        continue
    msg = obj.get('message') if isinstance(obj, dict) else None
    if not msg:
        continue
    role = msg.get('role')
    content = msg.get('content')
    if role == 'assistant' and isinstance(content, list):
        for c in content:
            if c.get('type') == 'text':
                print('=====ASSISTANT TEXT BLOCK=====')
                print(c.get('text'))
                print()
"
```

This bypasses truncation and recovers full verbatim findings. More direct than SendMessage resumption when you already have the file path and don't need to re-invoke agents.

## Trade-offs
- **Pros:** Direct, deterministic, no re-runs needed, works across file boundaries
- **Cons:** Manual command, requires knowing the .output file path, fragile if JSONL schema changes
