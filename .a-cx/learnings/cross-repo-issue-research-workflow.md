---
id: cross-repo-issue-research-workflow
trigger: "when filing an investigate or bug issue against withACX/a-cx-ai-config from a dev repo"
confidence: 0.9
domain: process
scope: org-wide
date: 2026-09-03
status: observed
---

# Cross-Repo Issue Research and Deduplication Workflow

## Action
Before filing an investigate issue against a-cx-ai-config from a dev repo, perform a deduplication check:

1. **Search existing rules** in the target repo for related guidance (e.g., if filing about comments, search `coding-standards/rules/common/coding-style.md`)
2. **Check label taxonomy** on the target repo (fetch all `area:*`, `domain:*`, `scope:*` labels)
3. **Search for existing open issues** using `gh search issues --repo withACX/a-cx-ai-config --state open "keyword"` to catch similar reports
4. **Verify ownership gate** -- confirm a CODEOWNERS file exists and can be resolved
5. **Create missing labels** if needed (e.g., `domain:code-style` did not exist; created it before filing)
6. **Check content conformance** -- ensure the body headings match the investigate type template
7. **File with proper metadata** -- use `issue_cli.py create-investigate` or `create-intake` with correct labels and origin marker

## Evidence
Observed 8+ steps in session 3fad9120-e811-45f1-925f-06502d2a6c54:
- Events 15-26: Searched coding-standards files for comment coverage; found only one thin line
- Events 27-28: Fetched all label axes on a-cx-ai-config to understand taxonomy
- Events 41-42: Checked for CODEOWNERS file (found it missing -- a blocker for cross-repo intake)
- Events 43-44: Created missing `domain:code-style` label with correct color code
- Events 49-50: Filed investigate issue #2834 with conforming body structure

## Why This Matters
- **Deduplication prevents duplicate issues** -- the repo tracks 273+ open issues; searching saves triage work
- **Label taxonomy must be pre-created** -- filing a label that doesn't exist fails silently
- **CODEOWNERS absence blocks intake** -- the intake intake machinery refuses to file if no reviewer can be resolved from .github/CODEOWNERS
- **Content conformance** -- an investigate issue must have ## What to investigate, ## Why it matters, ## Definition of done headings, or the file command rejects it

## Pattern
```bash
# 1. Search rules in target repo
gh api repos/withACX/a-cx-ai-config/contents/skills/coding-standards/rules/common/coding-style.md | jq '.content' | base64 -d | grep -i "comment"

# 2. Fetch all label axes
gh label list --repo withACX/a-cx-ai-config --limit 300 --json name --jq '[.[].name | select(startswith("area:")) | sort]'

# 3. Search open issues for duplicates
gh search issues --repo withACX/a-cx-ai-config --state open "comment style" --json number,title

# 4. Verify ownership gate
python3 scripts/ownership_gate.py withACX/a-cx-ai-config

# 5. Create missing label if needed
gh label create "domain:code-style" --repo withACX/a-cx-ai-config --color "0075ca"

# 6. File investigate issue
python3 scripts/issue_cli.py create-investigate \
  --repo withACX/a-cx-ai-config \
  --title "..." \
  --body-file body.md \
  --label learning --label "domain:code-style" --label "scope:org-wide"
```

## Related Learning
[[code-comment-rules-missing-from-shared-config]] -- the specific issue filed via this workflow
