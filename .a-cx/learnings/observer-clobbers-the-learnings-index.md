---
id: observer-clobbers-the-learnings-index
trigger: "after the learnings observer runs in a worktree that contains .a-cx/learnings/MEMORY.md"
confidence: 0.9
domain: process
scope: project
date: 2026-09-04
---

# The Observer Replaces the Learnings Index Instead of Appending to It

## Action
After an observer pass, check `.a-cx/learnings/MEMORY.md` before propagating it
anywhere. The observer rewrites the file from scratch listing only the learnings
IT wrote in that pass, discarding every existing entry. Never copy a
freshly-observed `MEMORY.md` over a good one, and never resolve a conflict
between two copies in favour of the shorter.

## Pattern
Observed on 2026-09-04 in the worktree for issue 59. The primary clone held the
real index: `# Project Learnings Index`, 53 lines, 40 entries. The worktree copy
after the observer pass was `# Project 59 Learnings Index`, 6 lines, 4 entries,
being exactly the four learnings that pass had just written. The title had been
rewritten to name the issue.

Nothing was lost, for a reason worth knowing: `MEMORY.md` has no YAML
frontmatter, so the repo's learnings validator rejects it and
`capture_internal_artifacts.py` WITHHOLDS it from every capture. The clobbered
version could therefore never be committed. A property that reads like a defect
in the capture tooling is the only thing standing between an observer pass and a
destroyed index.

## How to apply
Treat the primary clone's copy as authoritative and append to it by hand. When a
worktree and the primary disagree, diff the entry sets rather than the files: the
correct copy is the superset. If the withhold behaviour is ever "fixed" by giving
`MEMORY.md` frontmatter, this becomes a live data-loss path and needs the
observer fixed first.
