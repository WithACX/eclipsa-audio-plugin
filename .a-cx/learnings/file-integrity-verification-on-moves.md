---
id: file-integrity-verification-on-moves
trigger: "when moving or copying scripts between directories"
confidence: 0.85
domain: security
scope: project
date: 2026-09-04
---

# Always Verify File Integrity via SHA256 on Script Moves

## Action
Before deleting or overwriting the original copy of a script, compute SHA256 hashes of both source and destination (or committed blob) and confirm they match.

## Evidence
- Observed 3 times in session 736b1387: events 5 (before copy), 17 (before deletion), 22 (after restore)
- Pattern: `shasum -a 256 <original> <destination>` or `git cat-file blob <ref>:<path> | shasum -a 256`
- Use case: Moving `uninstall_dev_plugins.sh` from `scripts/` → `.a-cx/bin/`
- Result: Confirmed byte-for-byte identity before each destructive operation

## Why
- Scripts are sensitive; a single-byte change changes behavior or breaks syntax
- `.a-cx/` is guarded by leak-check tooling; proof of identity ensures safe handoff
- Prevents silent data loss if copy/restore operations fail partially

## How to Apply
1. After copying a script to its new location, capture the source hash
2. Before deleting the original, verify the destination/committed blob matches
3. After restoring from origin, re-verify against the original commit
4. Use `git cat-file blob` for origin-hosted versions; `shasum` for working tree

[[bash-syntax-check-on-scripts]] [[a-cx-leak-guard-workflow]]
