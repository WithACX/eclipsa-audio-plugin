---
id: upstream-pr-range-sweeps-unrelated-commits
trigger: "when running /upstream-pr (acx-upstream-pr.sh) to contribute a single commit or PR upstream from this fork"
confidence: 0.75
domain: git
scope: project
date: 2026-07-16
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
- Recurred contributing PR #11 (issue #41, "surface export failures"):
  `acx/dev` had drifted 26 non-merge commits from `upstream/main`, and the
  4 real contribution commits were interleaved with 2 internal
  `chore(a-cx): log learnings...` commits AND two already-separately-
  upstreamed fixes (`b763071` -> PR #118 merged, `cc6912d` -> PR #119
  closed) that a full-range replay would have re-swept in. Same manual
  workaround applied (fresh worktree off `upstream/main`, cherry-pick only
  the 4 target commits by SHA, guard run by hand via `git show
  acx/dev:.a-cx/hooks/pre-push`), producing clean PR
  google/eclipsa-audio-plugin#120. Confirms GH-456 is not yet fixed in the
  installed script version and the manual workaround remains necessary.
- Recurred contributing already-MERGED fork PR #17 (issue #38,
  "audio/video duration mismatch warning") retroactively, well after merge:
  `--source <PR-17-tip-sha>` (the only SHA still reachable once the fork's
  PR branch was deleted post-merge) resolved a `merge-base(upstream/main,
  source)` far enough back to sweep in 18 merge commits and unrelated
  history, and separately hit `die "commit ... mixes A-CX-internal paths
  with contribution code"` on a completely unrelated commit outside PR
  #17's own range -- confirming the wide-range problem gets worse, not
  better, the longer a merged PR sits before its upstream contribution is
  cut. Recovered the exact PR #17 commit list via `gh pr view 17 --json
  commits` (GitHub retains it even after merge/branch-deletion), classified
  each of the 25 commits with the guard's own `check_upstream_leak.py
  classify <sha>` (2 `empty` merges + 4 `config-only` chore/docs commits
  correctly excluded, 0 `mixed`), then applied the same manual workaround:
  fresh branch off `upstream/main`, cherry-picked only the 19 remaining
  `contribution`-classified commits in original order (1 came back empty
  post-cherry-pick -- a clang-format commit whose delta no longer applied
  against `upstream/main`'s state -- and was `--skip`ped rather than
  force-committed empty), guard run by hand, pushed, opened
  google/eclipsa-audio-plugin#122 directly via `gh pr create
  --repo ... --head WithACX:<branch>`. Lesson: for a MERGED (not open) fork
  PR, don't rely on `--source <sha>`'s auto-ranging at all -- pull the
  authoritative commit list from `gh pr view <n> --json commits` and
  `classify` each one, since the script's own default-source resolution
  (open-PR lookup) doesn't apply and manual SHA selection can't safely
  approximate it.
