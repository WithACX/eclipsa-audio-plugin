---
id: no-issue-numbers-in-code-comments
trigger: "when writing or reviewing a code comment (in .cpp/.h or elsewhere) that references an issue, ticket, ADO item, or PR number"
confidence: 0.9
domain: code-style
scope: project
date: 2026-07-24
---
# Do Not Put Issue/Ticket Numbers in Code Comments

## Action
Never write a code comment that names an issue or ticket number (e.g.
`// Issue #38: ...`). A comment should explain the WHY of the code entirely
on its own terms -- a hidden constraint, invariant, or non-obvious
consequence -- without depending on an external tracker for context. Issue
and PR references belong in the commit message or PR description, not in
source. This is the same rationale that already rules out WHAT-focused
comments and comments describing the current task/fix/caller: source
comments should read the same whether or not the reader has access to
the issue tracker, and should not rot when the tracker's IDs change or the
issue is renumbered/closed/moved.

## Evidence
- Explicit user feedback in session (2026-07-24), given while reimplementing
  PR #17 (`38-audio-video-duration-mismatch-warning`): several comments and
  test names in the original PR referenced "Issue #38" inline (e.g.
  `common/processors/tests/FileOutputProcessor_test.cpp`'s
  `mux_flags_mismatch_when_video_longer_than_audio` test comment, and
  `common/data_structures/src/FileExport.h`'s field comment). The user asked
  to stop this pattern going forward, for both `.cpp`/`.h` comments and by
  extension test descriptions/comments.

## How to apply
When writing or reviewing a comment that would otherwise start with
`// Issue #N:` or `// <TRACKER>-1234:`, drop the reference entirely and keep
only the substantive explanation of why the code does what it does. If the
issue number was standing in for context the reader needs (e.g. "this is the
short-bounce-vs-video-duration case"), keep that context in prose form
without the numeric reference.
