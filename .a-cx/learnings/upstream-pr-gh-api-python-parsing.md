---
id: upstream-pr-gh-api-python-parsing
trigger: "when systematically gathering PR metadata for upstream review"
confidence: 0.75
domain: workflow
scope: project
date: 2026-07-22
---

# Upstream PR Metadata Extraction via gh api + Python

## Action
When gathering metadata for upstream PR review (reviews, comments, commit messages, timeline), consistently pipe `gh api --paginate` to Python inline JSON parsers (`python3 -c '...'`) to extract and format specific fields rather than relying on gh's built-in output flags.

## Evidence
- Observed 4 times in session c316c463 (2026-07-22):
  - `gh api repos/google/eclipsa-audio-plugin/pulls/121/comments --paginate | python3`
  - `gh api repos/google/eclipsa-audio-plugin/issues/121/timeline --paginate | python3`
  - `gh api repos/google/eclipsa-audio-plugin/pulls/121/commits --paginate` (2x with different parsers)
- Pattern: `--paginate` to handle multi-page results, then inline Python to extract / reformat JSON fields
- Context: Investigating upstream PR google/eclipsa-audio-plugin#121 (WAV writer fix) for review scope, handling, and policy gaps

## Note
This is a tool-specific technique that has proven useful for parsing paginated GitHub API output. Consider whether this pattern should be abstracted into a reusable script or wrapped in a helper function if it's used across multiple sessions.
