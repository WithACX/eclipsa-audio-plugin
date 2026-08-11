---
id: dismissible-banner-clear-underlying-value-not-a-new-flag
trigger: "when adding or reviewing a dismissible warning/error banner backed by a repository (WarningBannerBase subclass)"
confidence: 0.9
domain: architecture
scope: project
date: 2026-08-11
---

# Persist a Dismissal by Clearing the Underlying Value, Not a New Flag

## Action

A `WarningBannerBase` subclass whose visibility is seeded from a repository
value on construction (so a re-created editor can pick a still-relevant
warning back up) must make its dismissal outlive that re-creation. Before
adding a second persisted "dismissed" flag alongside the original value,
check whether anything else in the codebase still needs to read that
original value after a dismissal. If nothing does, the simpler fix is for
`onDismiss()` to clear the original value itself (the same way starting a
fresh occurrence of the condition already resets it) -- no new field, no
second reset point to keep in sync with the first.

## Evidence

- `ExportErrorBanner::onDismiss()` was an intentional no-op ("Transient
  dismiss: no repository write, unlike DAWWarningBanner's persisted
  dismiss"), while its constructor unconditionally seeded visibility from
  whatever `FileExport::exportError_` was still on record. Because
  `exportError_` is only cleared at the start of the *next* export
  (`FileOutputProcessor::initializeFileExport`,
  `WavFileOutputProcessor::setNonRealtime`), a dismissed banner reappeared on
  every editor reopen.
- The first fix attempt added a second persisted `exportErrorDismissed`
  flag, mirrored the reset into both export-start call sites, and added a
  matching serialization path. Working, but more moving parts than needed.
- `getExportError()` has exactly one production reader
  (`ExportErrorBanner`) besides tests, so nothing else depends on the value
  surviving a dismissal. The simpler fix: `onDismiss()` resets
  `exportError_` to `kNoError` directly, inline. A dismissed banner then has
  nothing left on record to re-seed visibility from on the next construction
  -- fewer fields, fewer reset call sites, same result. (An earlier pass
  extracted this into a small pure static helper purely for headless
  testability; reviewer feedback preferred keeping it inline in the
  `onDismiss()` override for simplicity, at the cost of that test coverage --
  a deliberate trade-off, not an oversight.)
- `DAWWarningBanner` genuinely needs a separate persisted flag
  (`RoomSetup::dawWarningDismissed_`) because its underlying condition
  (`DAWCompatibilityChecker::isDAWSupported()`) is re-derived fresh on every
  construction, not read back from a stored value -- there is nothing to
  "clear". That is the deciding factor, not an inherent property of
  dismissible banners in general.

## How to apply

Before adding a persisted "dismissed" flag next to an existing repository
value:
1. Grep for every reader of that value. If dismissal is the only reason
   anything would read it after the fact, clearing it on dismiss is
   sufficient and removes the need for a second flag entirely.
2. Only reach for a separate flag when something else still needs the
   original value to survive a dismissal (e.g. an export history log, a
   status indicator elsewhere) -- then, and only then, add the flag and
   reset it at every point that already resets the original value.
3. Extracting the dismiss mutation into a small pure static function
   (mirroring `messageForError`/`shouldShowOnTransition` in this file) buys
   headless unit-test coverage, at the cost of one more named symbol for a
   one-line mutation. Either inline-in-`onDismiss()` or extracted-and-tested
   is defensible; ask rather than assume which the reviewer wants when the
   logic is this small.
