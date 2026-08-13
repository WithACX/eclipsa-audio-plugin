---
id: upstream-pr-metadata-verification-workflow
trigger: "when preparing to create an upstream PR from a fork branch"
confidence: 0.75
domain: git
scope: project
date: 2026-08-13
---

# Upstream PR Metadata Verification Workflow

## Action
Before creating an upstream PR, verify existing PR metadata and collect structured data on reviewers and merge status via `gh pr view --json` queries.

## Evidence
- Observed 4 times in session 4e411527-a5dd-4709-abbc-1a990f5074e6 (2026-08-12)
- Pattern: Run `gh pr view <pr> --repo google/eclipsa-audio-plugin --json <fields>` to extract reviews, reviewRequests, mergedBy, author
- Applied to PRs 119, 120, 122, 125
- Example: Queried reviews/reviewRequests separately, then mergedBy/author/collaborators
- Purpose: Validate PR state and identify eligible reviewers before creating new upstream PR

## Why This Matters
Structured JSON queries (`--json` flag) are more reliable than parsing text output and avoid shell quoting issues. Running separate queries for different field groups (reviews vs. merge status vs. collaborators) keeps commands focused and output parseable.

## When to Apply
- Before filing an upstream PR from a fork
- When verifying PR eligibility or reviewer permissions
- Collecting data on existing related PRs (to avoid duplication)
