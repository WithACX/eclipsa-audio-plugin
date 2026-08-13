---
id: acx-tooling-bug-discovery
trigger: "when reviewing A-CX plugin output and finding discrepancies from documented behavior"
confidence: 0.7
domain: process
scope: org-wide
date: 2026-07-22
---
# A-CX tooling bug: deliver-pr-review inline comments show bucket label instead of severity

## Action
When the `deliver-pr-review` skill posts inline PR comments, verify that each comment is prefixed with the finding's actual severity level (`[HIGH]`, `[MEDIUM]`, `[LOW]`) — not the blocking/advisory bucket label (`[advisory]`). File a bug in WithACX/a-cx-ai-config if the prefix is incorrect.

## Evidence
- Reviewed PR google/eclipsa-audio-plugin#121 via deliver-pr-review / upstream-pr
- Skill posted 5 inline comments; every one was prefixed `[advisory]`
- Underlying findings ranged from HIGH (reentrant deadlock risk) to MEDIUM/LOW (style notes)
- Documented format (review-workflow.md) specifies severity marker in comment body, not bucket label
- Result: filed WithACX/a-cx-ai-config#917
- Root cause: skill substitutes bucket label for severity marker during comment construction

## Impact
Reviewers scanning inline PR comments cannot triage by importance. A HIGH-severity memory-safety finding reads identically to a LOW-severity style note.
