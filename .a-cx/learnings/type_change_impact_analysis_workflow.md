---
id: type-change-impact-analysis-workflow
trigger: "when a type change is reviewed (e.g., long → juce::int64), verify downstream impact systematically"
confidence: 0.9
domain: code-style
scope: project
date: 2026-07-27
---
# Type Change Impact Analysis: Grep → Read → Trace

## Action
When analyzing the impact of a type change (return type, field type, local variable type), use a three-pass workflow:
1. **Grep**: Find all locations where the type is used (assignments, comparisons, arithmetic)
2. **Read**: Read the full context around each usage to understand how it's applied
3. **Trace**: For each usage, trace whether the caller's type matches or truncates

## Evidence
- PR #17 widened FileExport's getStartSampleIdx/getEndSampleIdx/getSampleTally from long to juce::int64
- Events 31-32: Grep found 20+ usages across WavFileOutputProcessor and LoudnessExportProcessor
- Events 15-22, 33-52: Read the full .h and .cpp files to understand field declarations vs. assignments
- Events 39-48: Traced int64 values through processBlock/prepareToPlay to confirm overflow danger in accumulators
- Test exploration: Confirmed no existing test covered overflow scenarios (~12+ hours audio required)

## How to apply
1. `grep -rn <type_name> <affected_files>` to locate every reference
2. For each file with hits: read the full class definition (header + implementation)
3. Check:
   - Assignment targets (does RHS value fit in LHS type?)
   - Comparisons (are both operands same width?)
   - Arithmetic (is accumulation bounded?)
   - Test coverage (is overflow scenario tested?)
4. Document any truncation or unsafe narrowing as a finding

## Related
- [[systematic_code_review_taskification]] — breaking changes into discrete impact steps
- [[code_exploration_grep_then_read]] — grep-first-then-read pattern established in memory
