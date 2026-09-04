---
id: comment-formatting-line-limits
trigger: "when writing code comments or refactoring comments in C++ on this project"
confidence: 0.85
domain: code-style
scope: project
date: 2026-09-03
status: observed
---

# Comment Formatting: One or Two Lines, Line Length Limits

## Action
Keep code comments to one or two lines maximum. Each comment explains what the code does at that point. Move multi-line block comments above member variables to inline comments on the same line. Respect 80-character column limit on all lines including comments.

## Evidence
Observed 5+ times in session 3fad9120-e811-45f1-925f-06502d2a6c54:
- Event 3-4: Moved member variable comment from block above to inline: `bool draggingSource_ = false;  // true between press and release`
- Event 9-10: Extracted all final added comments, showing concise one-to-two-line format
- Event 13: Triggered learning-workflow for self-diagnostics on comment style
- Events 33-34: Created org-wide learning linking back to this pattern
- Event 49-50: Filed investigate issue #2834 on a-cx-ai-config noting comment rules are missing from shared config

## Pattern Observed
Comments on this project follow these rules not found in the shared `coding-style.md`:
1. One or two lines per comment maximum
2. Explain what the code does at that point, not why the name was chosen
3. No tracker references in code comments (repo is a public downstream fork)
4. Avoid hyphen/double-hyphen clause grammar; avoid "which" and "hence"
5. Member variables get comment on same line, not a block above
6. One fact per comment; do not repeat the same reason

## Workflow
When refactoring comments:
1. Check column length: `awk 'length > 80 {print NR": "length}'`
2. If multi-line block above member variable, move to inline
3. If comment is 3+ lines, trim to 2 maximum
4. If comment names an issue/key, remove it (fork repository)
5. Use `clang-format --style=file:.clang-format -i` to reformat

## Related Learning
[[code-comment-rules-missing-from-shared-config]] -- org-wide issue that these project rules should be shared
