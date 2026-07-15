---
id: deliver-code-should-surface-prior-closed-prs
trigger: "when /deliver-code picks up an issue that already has history (re-opened, or previously attempted)"
confidence: 0.5
domain: process
scope: project
date: 2026-07-15
status: issue-raised
issue: "GH-457"
---
# Check for prior closed-without-merge PRs before re-delivering an issue

## Action
Before starting a fresh delivery attempt on an issue, check whether it
already has PRs that were opened and closed without merging (e.g. `gh pr
list`/`gh search prs` filtered to the issue, or `closingIssuesReferences` on
each candidate PR). None of `/deliver-code`'s standard pick gates do this
today. If prior closed-without-merge PRs exist, surface them (PR numbers,
whether any review comments explain the closure) to the developer before
proceeding -- a silent repeat of an already-failed attempt wastes a cycle and
can repeat the same mistake with no more diagnostic trail than the last time.
Filed as [[GH-457]] (WithACX/a-cx-ai-config#457) to make this a standing
`/deliver-code` pick-gate check instead of something an agent has to think to
do.

## Evidence
- Picking up tracker issue `WithACX/eclipsa-audio-planning#42` (already
  attempted twice earlier in the same session) had two prior closed PRs
  (`WithACX/eclipsa-audio-plugin#6`, `#7`) with zero recorded review comments.
  Nothing in the standard `/deliver-code` pick flow surfaced this; it was
  found only via an ad hoc `gh search prs` check prompted by the target
  issue's stale `status:in-review` label. See [[worktree-lfs-false-positive-diffs]]
  for the root cause this investigation subsequently uncovered.
