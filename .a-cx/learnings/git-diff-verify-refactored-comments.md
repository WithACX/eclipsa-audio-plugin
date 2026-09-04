---
id: git-diff-verify-refactored-comments
trigger: "when committing a comment refactoring or bulk text change to code"
confidence: 0.80
domain: git
scope: project
date: 2026-09-04
---

# Verify Comment Refactors with git diff Grep Checks Before Staging

## Action
After refactoring comments in bulk, run git diff checks to verify:
1. No ticket/issue references were left in new lines (grep for PAN-*, #[0-9]+, ADR-*, etc.)
2. No forbidden patterns like double-hyphen ` -- ` in added comments
3. No formatting violations before staging

## Evidence
- Observed 5+ times in session 3fad9120-e811-45f1-925f-06502d2a6c54 (events 2, 15, 16, 17, and implicit in 18-20)
- Pattern: After bulk edit, run `git diff origin/acx/dev...HEAD -- '*.h' '*.cpp' | grep '^+' | grep -vE '^\+\+\+' | grep -E 'PAN-[0-9]|#[0-9]+|ADR-[0-9]'`
- Same checks applied to staged working-tree changes
- Checks caught the remaining PAN-02.2/PAN-02.3 references after the first bulk rewrite (event 15)

## Why
This repo has active learnings against ticket references and specific prose patterns in comments. Bulk edits risk missing pockets of violations. The grep pattern focuses on NEW lines only (grep '^+'), so it catches only the author's additions, not pre-existing code.

## How to apply
After bulk-editing comments, run these checks before staging:
```bash
git diff origin/acx/dev...HEAD -- '*.h' '*.cpp' | grep '^+' | grep -vE '^\+\+\+' | grep -E 'PAN-[0-9]|#[0-9]+|ADR-[0-9]'
git diff -- '*.h' '*.cpp' | grep '^+' | grep -vE '^\+\+\+' | grep -- ' -- '
```
If no results, you're clean. See [[no-issue-numbers-in-code-comments]] and [[feedback-comment-style-short-and-code-aligned]] for the underlying rules.
