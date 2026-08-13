---
id: skill-documentation-read-before-execution
trigger: "when preparing to run a complex skill like deliver-autonomous or deliver-code"
confidence: 0.8
domain: workflow
scope: project
date: 2026-08-13
---

# Read Skill Documentation Before Execution

## Action
Read all relevant skill documentation (SKILL.md, agents/*.md, references/, knowledge/) before executing the skill or delegating to agents — this preloads context and prevents mid-execution discovery gaps.

## Evidence
- Observed 5 times in session f5088278-7044-4917-a33f-8bfffb6832c7 (2026-08-13)
- Files read before unit dispatch:
  - autonomous-workflow.md (references)
  - delivery-unit.md (agent definition)
  - process-config.md (knowledge)
  - derived-work.md (references)
  - And skill routing validation scripts
- Pattern: Read -> Validate -> Execute, in that order
- Applied before dispatching delivery-unit agent for issue #55

## Why This Matters
The deliver-autonomous and deliver-code skills are complex orchestration systems with many state machines, constraints, and gate rules. Reading the documentation first:
1. Clarifies preconditions (e.g., `autonomous_merge: false` means no merge this run)
2. Identifies what ledgers and registries track the work
3. Reveals validation steps that must run before execution (e.g., `validate_process_routing.py`)
4. Prevents silent failures from misunderstood posture/config

## When to Apply
- Before running `/deliver-code` or `/deliver-autonomous`
- Before dispatching a `delivery-unit` agent
- When orchestrating multi-unit work (Project mode)
- When the skill has complex state machines or gates (check SKILL.md's depth/context management section)
