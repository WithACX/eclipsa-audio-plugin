# Eclipsa Audio Plugins -- Claude Code Rules

Read `AGENTS.md` for project context, tech stack, architecture, build,
testing, branch topology, and anti-patterns. This file contains Claude
Code-specific configuration only.

## Linked Plugins

`.claude/settings.json` enables the A-CX plugins configured for this repo
(the `cpp` optional plugin plus the org-managed mandatory plugins). See that
file for the current list.

## Hooks

Observer and session hooks arrive via the `learnings` plugin. No per-repo
hooks are configured here.

## What NOT to Do (Claude-specific)

- Don't disable hooks with `--no-verify` unless the user explicitly asks.
- Don't invoke plugins not listed in `.claude/settings.json` -- they will not
  load.
- Don't stage A-CX tooling onto `main`; see the branch topology rule in
  `AGENTS.md`.
