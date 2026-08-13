---
id: verify-process-config-before-major-workflows
trigger: "before executing /deliver-code, /deliver-autonomous, or /orchestrate"
confidence: 0.88
domain: process
scope: project
date: 2026-08-13
---
# Verify Complete Process Config and Auth Before Major Workflows

## Action
Before dispatching any major workflow (autonomous delivery, orchestrate, or /deliver-code), verify: (1) .a-cx/process.yaml and .a-cx/github.yaml are readable and correct; (2) gh auth status shows logged-in account with required token scopes; (3) git remotes (origin and upstream) are correctly configured; (4) CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH is set correctly (default 3 if unset); (5) .claude/settings.json lists all required plugins. This is a single preflight gate before work begins.

## Evidence
- Observed 4+ times in session 72dbefe5-1303-4214-a0a3-8a178b1574b1
- Pattern: (1) line 19 checks auth/remote/settings, (2) line 21 reads process.yaml/github.yaml, (3) line 31 repeats auth/remote inspection, (4) line 33 re-verifies gh auth and config
- Check saves 10-30s per workflow by catching misconfiguration early
- Root cause of [autonomous-preflight-workflow-sequence]: the first step reads docs BECAUSE the config is verified to be stable

## Common Failures
- gh token lacks scopes (found in this session: missing 'read:project')
- .a-cx/process.yaml has disabled TDD but not documented why (repo defaults apply)
- github.yaml tracker repo mismatches code repo (WithACX/eclipsa-audio-planning vs local WithACX/eclipsa-audio-plugin)
- CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH unset, depth default is 3
