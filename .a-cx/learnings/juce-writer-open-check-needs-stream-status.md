---
id: juce-writer-open-check-needs-stream-status
trigger: "when checking whether a juce::AudioFormatWriter (created via AudioFormat::createWriterFor) actually opened its underlying file successfully"
confidence: 0.7
domain: architecture
scope: project
date: 2026-07-17
---
# `writer != nullptr` is not enough to detect a JUCE AudioFormatWriter open failure

## Action
Checking only `writer_ != nullptr` after `format.createWriterFor(stream,
...)` is insufficient to detect an open failure. `WavAudioFormat::createWriterFor`
(and other JUCE format implementations) only check that the passed-in
`OutputStream*` pointer itself is non-null before constructing the writer --
they do NOT check whether that stream actually opened its underlying file.
`juce::FileOutputStream`'s constructor always succeeds in producing a valid
C++ object even when the OS-level `open()` call fails (e.g. missing parent
directory, permission denied): the failure is recorded in the stream's
`status` member (`Result`, queryable via `getStatus()`), not reflected by a
null pointer. So a writer wrapping a stream whose file never actually opened
still comes back non-null from `createWriterFor`.

To correctly detect this, capture the stream pointer separately before
handing it to `createWriterFor`, and check `stream->getStatus().wasOk()` in
addition to `writer != nullptr` right after construction. If the writer is
non-null but the stream's status is bad, treat it as an open failure (delete
the writer, which also deletes the stream it owns via
`AudioFormatWriter`'s destructor).

## Evidence
- `FileWriter::isOpen()` (`common/processors/file_output/FileWriter.h`)
  originally checked only `writer_ != nullptr`, which meant an invalid
  export path (e.g. a per-audio-element WAV path resolving into a
  non-existent subdirectory) was NOT detected at open time -- only a later
  mid-write failure caught it, purely by coincidence, in
  `FileOutputProcessor`'s path (since it always writes audio blocks after
  opening). This bug was fully masked there and only surfaced in
  `WavFileOutputProcessor`'s new test, which toggled render start/stop
  without writing any audio, and observed `ExportError` incorrectly staying
  `kNoError` despite a genuine, logged open failure.
