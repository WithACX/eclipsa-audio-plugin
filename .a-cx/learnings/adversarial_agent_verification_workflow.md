---
id: adversarial-agent-verification-workflow
trigger: "when code review findings require independent verification before merge"
confidence: 0.8
domain: process
scope: project
date: 2026-07-27
---
# Adversarial Agent Verification for Code Review Findings

## Action
When a code review identifies a complex finding (especially HIGH severity across multiple files), spawn a dedicated agent to independently verify it with explicit refute instructions before accepting it as real.

## Evidence
- PR #17 session (eclipsa-audio-plugin):
  - Event 1-2: C++ reviewer identified HIGH finding: incomplete int64 widening in WavFileOutputProcessor and LoudnessExportProcessor
  - Event 3-8: Spawned general-purpose agent with adversarial prompt ("try to REFUTE the claim") to verify independently
  - Agent ran code inspection and returned CONFIRMED verdict with full file:line evidence
  - Pattern: Prevents plausible-but-wrong findings from surviving merge
- Extraction technique: When agent output truncated, use Bash to parse .output JSONL and extract full text response

## How to apply
1. For HIGH/CRITICAL findings with cross-file impact: add `run_in_background: false` to force synchronous agent return
2. Frame the verification prompt to ask the agent to REFUTE, not validate (switches reasoning mode)
3. Require exact file:line evidence and scope judgment in the response
4. If the response is truncated in the main loop, parse the agent's .output JSONL file with Python json/grep to recover the full finding

## Related
- [[multi_agent_review_transcript_extraction]] — parsing agent outputs when truncated
- [[agent_review_output_completeness]] — demanding full findings, not summaries
