---
id: systematic-code-review-taskification
trigger: "when reviewing code changes that have multiple independent issues"
confidence: 0.80
domain: process
scope: project
date: 2026-07-25
---

# Systematic Code Review with Task Decomposition

## Action
Break multi-issue code reviews into discrete tasks immediately — create one TaskCreate per finding, fix them in order, mark each done as you complete it. This prevents scope creep and ensures nothing slips through.

## Evidence
- Observed 4 times in session 2e881e7f (PR #17 review):
  - Identified asymmetry in `framesWritten_` reset between FileOutputProcessor and PremiereProFileOutputProcessor
  - Identified unasserted test paths for mismatch warnings
  - Identified missing RAII wrapper for GF_ISOFile
  - Identified missing [[nodiscard]] annotation
- Created TaskCreate for each finding immediately after verification
- Updated each task status to `completed` as fixes landed (Edit + TaskUpdate pattern)
- Prevented loss of issues by capturing them atomically rather than in-progress mental notes

## How to apply
When code review reveals multiple defects: TaskCreate each one with a single-sentence subject, then pipeline through fixes. Don't accumulate them mentally — the formal task list is your safety net for complex reviews.
