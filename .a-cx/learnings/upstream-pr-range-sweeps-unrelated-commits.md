---
id: upstream-pr-range-sweeps-unrelated-commits
trigger: "when running /upstream-pr (acx-upstream-pr.sh) to contribute a single commit upstream from this fork"
confidence: 0.6
domain: git
scope: project
date: 2026-07-15
status: issue-raised
issue: "GH-456"
---
# acx-upstream-pr.sh's range can be much wider than the single commit you meant to contribute

## Action
Before trusting `acx-upstream-pr.sh --source <commit>` to cherry-pick cleanly,
check `git merge-base upstream/main <commit>` and `git log --oneline
<merge-base>..<commit>` yourself. If `acx/dev` has accumulated other
not-yet-upstreamed work (other fixes, merge commits, A-CX-internal
tooling/docs/CI commits) since the last real sync with `upstream/main`, the
script's range will include all of it, not just your commit -- and will
likely fail on merge commits or get blocked by the pre-push guard for
carrying guarded paths. If the range is wider than just your intended
commit(s), skip the script's built-in cherry-pick step: check out a fresh
branch from `upstream/main` yourself, `git cherry-pick` only the exact
commit(s) you mean to contribute, and run the pre-push guard manually as a
backstop before pushing -- extract its content via `git show
acx/dev:.a-cx/hooks/pre-push` if `.a-cx/` isn't present in the fresh
upstream-based checkout.

Filed as [[GH-456]] (WithACX/a-cx-ai-config#456) to fix the script itself so
this stops requiring a manual workaround.

## Evidence
- Observed during the #42 delivery run: contributing fix commit `cc6912d` via
  `--source cc6912d` resolved `merge-base(upstream/main, cc6912d)` to a
  commit 20 commits back, sweeping in the `#44` WAV-checkbox fix, several
  merge commits, and a chunk of `.a-cx/bin`/`.a-cx/hooks` tooling commits.
  Worked around manually: fresh branch from `upstream/main`, single-commit
  cherry-pick, guard extracted via `git show` and run by hand, then pushed
  and opened PR google/eclipsa-audio-plugin#119 directly.
