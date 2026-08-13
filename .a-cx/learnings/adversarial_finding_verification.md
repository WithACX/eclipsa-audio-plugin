---
id: adversarial-finding-verification
trigger: "when a review agent reports a finding that is non-obvious, surprising, or could affect downstream fix decisions"
confidence: 0.80
domain: process
scope: project
date: 2026-07-27
---

# Spawn Adversarial Verifiers for High-Stakes Findings

## Action
When a code or C++ review agent reports a finding that is complex, relies on subtle code analysis, or would require significant refactoring if true, spawn an independent adversarial verifier agent to attempt to REFUTE the claim before accepting it as real.

## Evidence
- Observed 3+ times in session 0ec3a889 (events 52-59, PR #17 verification): after cpp-reviewer reported a HIGH finding about `sampleTally_` overflow, user spawned a general-purpose verifier with explicit instructions: "Try to REFUTE the claim below... Verify specifically: [4 concrete code questions]... Report a clear verdict: SURVIVES (the finding is real) or REFUTED (explain exactly why it's wrong)"
- Verifier read the actual code in the worktree, checked line numbers, confirmed accumulation pattern, validated Windows LLP64 context, and returned: "Verdict: SURVIVES. Every element of the claim checks out..."
- Pattern: allows catching both false positives (agent misunderstood the code) and false negatives (verifier adds nuance/confirmation)

## How to apply
For a finding that is:
- Non-obvious or requires deep code reading
- Would drive a HIGH/MEDIUM fix decision
- Relies on subtlety (e.g., overflow math, type sizes, platform-specific behavior)
- Affects an adjacent/pre-existing area (important to confirm scope is correct)

Spawn a general-purpose agent with prompt:
```
You are adversarially verifying a code review finding. Your job is to try to REFUTE the claim below.

CLAIM: [quote the finding]

Read these files: [paths]

Verify specifically:
1. [question 1 -- does X actually exist?]
2. [question 2 -- is Y actually a risk in this codebase?]
3. [question 3 -- is this truly pre-existing / untouched by the PR?]
4. [question 4 -- is the consequence real / reachable?]

Report a clear verdict: SURVIVES (the finding is real and accurately scoped) or REFUTED (explain exactly why it's wrong -- e.g. [reasons]). Be specific and cite line numbers/code.
```

Extract the complete verdict from the verifier's transcript (per multi-agent-review-transcript-extraction.md) and use it to decide whether to file the finding, defer it, or dismiss it.
