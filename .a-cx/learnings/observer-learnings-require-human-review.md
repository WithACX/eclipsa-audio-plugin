---
id: observer-learnings-require-human-review
trigger: "when learnings are automatically written by the learnings observer before capture"
confidence: 0.95
domain: process
scope: project
date: 2026-09-03
---

# Observer-Written Learnings Require Human Review Before Capture

## Action

Always READ and VERIFY the content of observer-written learnings before running `capture_internal_artifacts.py` to commit them. Correct any factual errors, false causal claims, or inaccurate advice IN THE FILE before committing.

A captured learning is prose that future runs will apply, so an error in one is live documentation until the next human edits it.

## Evidence (2026-09-04 reinforcement)

Recurred twice more in one session, which is what took this to 0.95:

- A batch of three learnings contained an inverted script order and a false
  causal claim, both corrected before capture.
- A later batch of four contained TWO learnings that contradicted EACH OTHER on
  the same question. One said to use the plugin scripts for issue mutations, the
  other said to use `gh` exclusively and never parse text. Both were written in
  the same pass. Reconciled by scoping one to reads and PRs and the other to
  issue writes.
- One embedded an absolute plugin-cache path including a home directory and a
  version number, which is wrong on any other machine.

So the failure is not only per-file accuracy: a single batch is not internally
consistent either. Read the batch as a set, not file by file.

## Pattern

The learnings observer generalizes from session observations to extract patterns. It produces high-confidence learnings (0.85+), but they can contain:

- **Inverted order** in multi-step procedures (what comes first vs. second)
- **False causality** (claiming X must happen before Y to feed Z, when actually Y happens before Z and X is independent)
- **Scope errors** (claiming a tool checks "blockers" when it only checks "open sub-issues")
- **Mirroring misunderstandings** (assuming bidirectional sync when one-directional is configured)
- **Advice that contradicts the harness** (polling patterns that conflict with notification-based wakeups)

Corrections are marked in the file when applied. Flag them in the PR body if they affect understanding of the captured work.

```bash
# Workflow:
# 1. Observer writes learnings to .a-cx/learnings/*.md
# 2. Human reads each file
for f in .a-cx/learnings/*.md; do
  # Read and correct errors in place
  vim "$f"
done
# 3. Then run capture
python3 $PR/scripts/capture_internal_artifacts.py
```

## Evidence

Observed 3+ times in session 3fad9120:
- Event 5: Observer writes `deliver-code-python-script-gate-sequence.md` claiming capture must run first to feed verdicts (false)
- Event 5: Observer writes `github-cli-board-gate-sequence.md` claiming guard checks blockers and board mirrors bidirectionally (both false on this repo)
- Event 6-7: Human reads all three learnings, finds and corrects errors, commits the corrected versions
- Event 12: PR body updated to declare which learnings were corrected and why

No observer-written learning in the repo should be assumed correct without human verification of its prose.

