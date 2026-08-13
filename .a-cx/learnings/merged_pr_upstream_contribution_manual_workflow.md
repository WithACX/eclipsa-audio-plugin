---
id: merged_pr_upstream_contribution_manual_workflow
trigger: "when contributing an already-merged fork PR to upstream after the original PR branch is deleted"
confidence: 0.85
domain: git
scope: project
date: 2026-07-30
---
# Contributing a merged fork PR to upstream requires pulling the commit list from GitHub, not relying on --source

## Action
For an already-MERGED fork PR (where the PR branch has been deleted and only the merge commit is reachable), do NOT use `acx-upstream-pr.sh --source <tip-sha>`. Instead: (1) Pull the authoritative commit list from GitHub via `gh pr view <n> --json commits --jq '.commits[].oid'` (retained even post-merge/branch-deletion). (2) Classify each commit with the guard's own `check_upstream_leak.py classify <sha>` to separate empty merges, config-only (chore/docs) commits, and contribution commits. (3) Create a fresh branch off `upstream/main`. (4) Cherry-pick only the `contribution`-classified commits in original order, skipping any that come back empty post-cherry-pick (delta no longer applies). (5) Run the pre-push guard manually via `printf '<local> <remote> <branch>' | python3 /tmp/guard.py upstream`. (6) Push to fork and open PR directly via `gh pr create --repo google/... --head WithACX:<branch>`.

## Evidence
- Observed 2026-07-30 contributing PR #17 (issue #38, audio/video duration-mismatch warning) retroactively after merge: the PR's original branch was deleted post-merge on WithACX/eclipsa-audio-plugin, leaving only the reachable tip SHA `81d6210`. Using `--source 81d6210` tried to resolve a merge-base far enough back to sweep in 18 merge commits + unrelated history, and hit the guard's `die "commit ... mixes A-CX-internal paths with contribution code"` on a completely unrelated commit outside PR #17's own range (confirming the wide-range problem gets worse the longer a merged PR sits).
- Recovery workflow: (1) `gh pr view 17 --json commits` yielded the exact 25-commit list (2: empty merges, 4: config-only chore/docs, 19: contribution). (2) Classified each with `check_upstream_leak.py classify <sha>` to confirm the split. (3) Fresh branch off `upstream/main`: `git checkout -b 38-... upstream/main`. (4) Cherry-picked only the 19 contribution commits; 1 came back empty (clang-format delta no longer applied vs. upstream/main) and was `--skip`ped. (5) Ran guard manually; passed with exit 0. (6) Pushed to fork, opened PR via `gh pr create --repo google/eclipsa-audio-plugin --base main --head WithACX:38-...`, obtained PR #122.
- Key insight: `acx-upstream-pr.sh --source <sha>` auto-ranging is only safe when the source is an open fork PR (default-source lookup) or a commit with a known tight range. For a MERGED PR (no open fork PR, unknown range), the script's range computation is unreliable and the manual workaround is necessary. GitHub's PR metadata (retains commit list post-merge) + the guard's own `classify` subcommand provide the authoritative inputs.
