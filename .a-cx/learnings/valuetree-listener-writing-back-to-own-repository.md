---
id: valuetree-listener-writing-back-to-own-repository
trigger: "when a class registers itself as a juce::ValueTree::Listener on a repository (FileExportRepository, etc.) AND that same class also calls repository.update() from a method reachable through that listener's own callback chain"
confidence: 0.8
domain: architecture
scope: project
date: 2026-07-17
---
# A ValueTree listener writing back to its own repository has two distinct reentrancy hazards, not one

## Action
Before writing to a `RepositorySingleBase`-backed repository (e.g.
`fileExportRepository_.update(...)`) from inside code that is itself
reachable via that same object's `juce::ValueTree::Listener` callbacks
(`valueTreePropertyChanged`/`valueTreeChildAdded`/etc., or anything they
call), check for BOTH of these independently -- fixing one does not fix the
other:

1. **Reentrant-lock deadlock.** `RepositorySingleBase::update()` calls
   `state_.copyPropertiesFrom(...)`, which fires listener callbacks
   SYNCHRONOUSLY on the calling thread (verified:
   `juce_ValueTree.cpp` `setProperty`/`sendPropertyChangeMessage` are direct
   calls, never queued). If the method holding a non-reentrant lock (e.g.
   `juce::SpinLock`, explicitly documented as "if a thread tries to acquire a
   lock it already holds, this will never return") calls `update()` while
   still holding that lock, and the listener chain leads back into a method
   that re-acquires the same lock, it hangs forever.
2. **Stale-snapshot clobbering (subtler, no lock required).** Real call
   sites in this codebase read-modify-write a FULL object snapshot:
   `config = repo.get(); config.setX(y); repo.update(config);` (e.g.
   `rendererplugin/src/screens/FileExportScreen.cpp`'s export button
   handler). `ValueTree::SharedObject::copyPropertiesFrom` applies the
   source's properties in the SAME order they were declared in
   `toValueTree()` (which follows the `EXPORT_VALUE` macro declaration order
   in the data structure's header, e.g. `FileExport.h`). If a listener
   callback triggered partway through that loop (say, by the `manualExport`
   property, declared early) synchronously writes to a DIFFERENT property
   (say `exportError`, declared later) via its own nested `update()` call,
   the OUTER loop's still-in-progress iteration will reach that later
   property from its own (older, captured-before-any-of-this) snapshot and
   silently reapply the stale value, overwriting what the nested callback
   just set. This is deterministic given the declaration order, not a rare
   race.

Fix for both: never write to the repository synchronously from inside a
reachable-via-listener call path. Defer via `juce::MessageManager::callAsync`
(the pattern `common/components/src/ExportErrorBanner.h` already uses for
this exact repository) so the write runs strictly after the ENTIRE
triggering call stack -- including any outer caller's own `copyPropertiesFrom`
loop -- has fully unwound. For unit-testing deferred-to-message-thread logic,
this codebase has no running JUCE message loop in its gtest binaries
(`JUCE_MODAL_LOOPS_PERMITTED` is 0, so `runDispatchLoopUntil` isn't even
compiled in) -- add a small protected virtual dispatch seam (e.g.
`postToMessageThread(std::function<void()>)`, default = `callAsync`) that a
test-only subclass overrides to queue tasks into a manually-drainable vector
instead, then drain it explicitly only after the triggering `update()` call
in the test has already returned (faithfully reproducing the real timing
guarantee without a real message loop).

## Evidence
- Found while extending `ExportError` classification to
  `WavFileOutputProcessor` (issue #47,
  `WithACX/eclipsa-audio-planning#47`): a first code-review pass caught the
  deadlock; fixing it naively (moving the `update()` call outside the lock)
  still left the clobbering bug, only caught by tracing the exact
  synchronous call chain in a SECOND, independent review pass rather than
  trusting that the lock fix alone was sufficient.
- `FileOutputProcessor`/`PremiereProFileOutputProcessor` do NOT register as
  listeners on `fileExportRepository_`, so their synchronous `update()` calls
  (added in the same change, for the IAMF/per-element WAV paths) never had
  either hazard -- this is specific to classes that both listen AND write to
  the same repository.
