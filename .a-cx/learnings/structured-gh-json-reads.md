---
id: structured-gh-json-reads
trigger: "when reading issue, pull request, workflow run, or search state for this repository through the gh CLI"
confidence: 0.7
domain: workflow
scope: project
date: 2026-11-25
---
# Read GitHub state with gh --json and a jq filter
## Action
Read GitHub state with gh commands that pass --json fields and a -q jq filter, not free-form text output.
## Evidence
- Observed 9 times in session 20d2857c-ac35-453b-be66-b4c8b81c4599
- Pattern: Issue, PR, run, and search reads consistently used gh with --json and -q to pull specific fields such as isDraft, mergeStateStatus, reviewDecision, and statusCheckRollup, rather than parsing plain text output.
