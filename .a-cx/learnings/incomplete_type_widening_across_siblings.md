---
id: incomplete-type-widening-across-sibling-classes
trigger: "when widening a type to fix overflow in one class, check mirror classes that consume the same getters"
confidence: 0.75
domain: code-style
scope: project
date: 2026-07-27
---
# Incomplete Type Widening Across Sibling Consumer Classes

## Action
When widening a type in a data class (e.g., FileExport) to fix overflow (e.g., `long` → `juce::int64` for 32-bit-under-Windows-LLP64), verify that ALL consumer classes that receive those widened getters are updated in the same diff, not just the primary consumer.

## Evidence
- PR #17 fixed 32-bit overflow in FileExport's startSampleIdx_, endSampleIdx_, sampleTally_ by widening to juce::int64
- WavFileOutputProcessor.h:124-125 still declared startSampleIdx_/endSampleIdx_ as `long`, causing silent truncation on assignment
- LoudnessExportProcessor.h:81-83 similarly kept sampleTally_/startSampleIdx_/endSampleIdx_ as `long`
- LoudnessExportProcessor.cpp accumulated sampleTally_ every block — independent overflow ceiling at ~12.4 hours @ 48kHz
- Both files received narrowing assignments: `startSampleIdx_ = config.getStartSampleIdx()` (int64 → long)
- No test existed to catch the overflow (~12+ hours audio required)

## How to apply
1. When widening in a getter's return type, grep for every place that getter is called
2. Read each call site and verify the receiving variable is widened to match
3. If the PR already edits that file for unrelated reasons, fix the type widening in the same commit
4. Consider: Is there an accumulator (like sampleTally_) that needs widening even if it's not assigned from a getter?

## Related
- [[systematic_code_review_taskification]] — breaking type-change impact into grep + read + trace
- [[code_exploration_grep_then_read]] — pattern for locating and understanding all call sites
