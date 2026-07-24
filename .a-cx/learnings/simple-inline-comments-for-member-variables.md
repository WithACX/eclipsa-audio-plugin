---
id: simple-inline-comments-for-member-variables
trigger: "when writing or reviewing a comment on a class-level member variable declaration"
confidence: 0.9
domain: code-style
scope: project
date: 2026-07-24
---
# Class-level Member Variables Get a Simple Inline Comment, Not a Block Comment

## Action
When a class-level member variable needs an explanatory comment, keep it to
a short comment on the same line as the declaration (e.g.
`long framesWritten_;  // samples handed to the writers this export`) rather
than a multi-line block comment above the declaration. Multi-line comments
above a member variable are reserved for genuinely non-obvious invariants
that don't fit in a single line -- the common case (what this field holds,
when it resets) fits inline.

## Evidence
- Explicit user feedback in session (2026-07-24), given while reviewing the
  audio/video duration-mismatch PR: a newly added `framesWritten_` member in
  `common/processors/file_output/FileOutputProcessor.h` had grown a
  multi-line block comment above it explaining its purpose; the user asked
  for this pattern (multi-line comments on member variable declarations) to
  be avoided going forward in favor of a simple inline description.

## How to apply
Before adding a comment above a member variable declaration, check whether it
fits on the same line as the declaration instead. If it does, move it inline.
Reserve a comment block above the declaration only when the explanation
genuinely cannot be compressed to one line (e.g. a subtle threading/lifetime
invariant), and even then prefer linking to a more detailed comment
elsewhere (e.g. on the method that establishes the invariant) over growing
the field-level comment.
