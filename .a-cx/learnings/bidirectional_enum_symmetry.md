---
id: bidirectional-enum-symmetry
trigger: "when adding direction-specific error states to ExportError or similar enums"
confidence: 0.75
domain: code-style
scope: project
date: 2026-07-24
---

# Bidirectional Enum Symmetry

## Action
When adding a direction-specific error state to an enum (e.g., `kVideoLongerThanAudio`), immediately add its opposite (e.g., `kAudioLongerThanVideo`) to the same enum, and ensure both get parallel test coverage, error messages, and comparison logic.

## Evidence
- Observed 5 times in session 8189b128: adding `kAudioLongerThanVideo` to FileExport.h enum alongside `kVideoLongerThanAudio`
- Pattern appears in: ExportErrorBanner.h message mapping, ExportErrorBanner_test.cpp assertions, FileOutputProcessor.cpp logic, FileOutputProcessor_test.cpp test cases
- Each new error state required: message text + test assertion + conditional branch (not just one direction)
- Tests expanded from 1 case to 3 (video longer, audio longer, no mismatch) to cover both directions

## Why
Audio/video sync checks are inherently bidirectional. Adding only one direction creates an incomplete feature, forces a follow-up PR, and can lead to silent failures if one case wasn't tested. Symmetry also makes the code's intent clearer and reduces review back-and-forth.

## How to apply
Before closing the PR, scan for any enum values or boolean flags that capture directional state. Check the paired logic in every file that touches that enum (processors, UI components, tests). If you see `kVideoLongerThanAudio`, immediately ask "what happens when audio is longer?" and add the opposite case.
