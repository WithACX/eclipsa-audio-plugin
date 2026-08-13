---
id: code-exploration-grep-then-read
trigger: "when exploring code impact or understanding unfamiliar code sections"
confidence: 0.7
domain: workflow
scope: project
date: 2026-07-27
---

# Code exploration: grep to locate, Read for context

## Action
When navigating an unfamiliar codebase area or tracing code impact, use grep to find all usages/declarations, then Read specific line ranges for context rather than reading whole files. This combines broad search with targeted reading.

## Evidence
- Observed 4 times in session ba9eb5ce during PR #17 review fix
- Event 7-8: `grep framesWritten_` in processors/ to find all usages, then Read to inspect types
- Event 9-10: `grep class.*FileOutputProcessor` for inheritance, then sed/Read for context
- Event 11-12: `grep juce::int64` to check type conventions used elsewhere
- Event 19-20: `grep sampleTally_` to see how similar counter is incremented
- Pattern: Each grep search was immediately followed by Read of specific lines (offset + limit) rather than reading whole files

## Related
- [[always_use_a_worktree]] — isolation ensures grep results are clean and current
- [[build_test_before_pr]] — this exploration feeds into informed code changes
