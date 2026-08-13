---
id: findings-to-remediation-closed-loop
trigger: "when review agents generate findings and a follow-up fix is implemented"
confidence: 0.85
domain: process
scope: project
date: 2026-07-30
---

# Review Findings → Remediation Closed Loop

## Action

After multi-agent review generates findings in a review_findings.md artifact, write a fix commit that explicitly references those findings by name, and follow up in a later session to verify the specific code locations mentioned in findings have been addressed (grep → read → verify).

## Evidence

- Observed 3 times in session 3e45cbf9 (2026-07-30):
  - Code reviewer agent generates findings (via code-reviewer)
  - C++ reviewer agent generates findings (via cpp:cpp-reviewer)
  - Findings synthesized into `/scratchpad/pr17-worktree/review_findings.md`
  - Session b79f1b98 discovers commit 7bd643e with message: "Addresses the HIGH and MEDIUM findings from review_findings.md on PR #17"
  - Commit message itemizes each fix and the finding it addresses
  - Verification: grep for the specific functions/files mentioned in findings, read the fixed code, confirm the fix
  - Pattern: **generate → synthesize → fix → verify** creates a durable closed loop

## How to apply

When a multi-agent review completes:
1. Synthesize findings into a single `review_findings.md` document with clear file/line anchors for each issue
2. When implementing fixes in a follow-up commit, reference the findings document explicitly in the commit message: "Addresses the MEDIUM findings from review_findings.md on PR #N"
3. Itemize each finding and how it was fixed in the commit message (makes the connection durable and auditable)
4. In a later session, verify fixes by grepping the exact file/function names from the findings, reading the fixed code, and checking that the remediation matches the stated finding

This pattern turns review findings into a **durable artifact** that drives accountability and can be verified independently of the reviewer's immediate feedback.
