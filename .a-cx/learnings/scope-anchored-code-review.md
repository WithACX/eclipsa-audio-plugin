---
id: scope-anchored-code-review
trigger: "when reviewing code changes in a PR"
confidence: 0.85
domain: code-style
scope: project
date: 2026-07-27
---

# Scope-Anchored Code Review

## Action
Anchor every code review to the stated change objective, and classify each finding as **in-scope** (caused by or required for the change) or **adjacent** (pre-existing, independent). Tag findings with scope + one-line causation rationale.

## Evidence
- Observed 3 times in session b1aa09f4-14ee-4f71-b6ea-1aecc5d5b37a
- Pattern: All three review agents (code-reviewer, security-reviewer, cpp-reviewer) in PR #17 review received the same instruction: state the change objective first, classify every finding as in-scope or adjacent with causation rationale
- Verdict: This is the methodology used in deliver-pr-review workflow for this project

## Why
This discipline prevents scope creep and clarifies whether a finding actually blocks the PR or is pre-existing cleanup. It makes the reviewer and author agree on what "done" means before review starts.

## How to apply
When running a code review or /deliver-pr-review on this project, always:
1. State the change objective (what the PR fixes/adds/changes, derived from title/body/linked issue)
2. For each finding: classify as **in-scope** (caused by diff, required for objective, or precondition for the change) or **adjacent** (pre-existing and independent)
3. Provide a one-line causation rationale for the classification
4. Report: {severity, file, line, title, evidence, fix, **scope tag + rationale**}
5. Verdict: report only in-scope findings for merge gating; note adjacent findings separately (can be filed as follow-up issues but don't block this PR)

## Related [[scope-axis]] (from deliver-pr-review policy docs; the single source)
