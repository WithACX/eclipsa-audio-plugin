---
id: grep-include-flag-zsh-incompatibility
trigger: "when using grep with --include=*.cpp or --include=*.h flags"
confidence: 0.75
domain: workflow
scope: project
date: 2026-07-30
---

# Grep --include Flag Incompatible with Zsh

## Action
Use plain `grep -rl "pattern" .` then filter results with additional pipes, instead of relying on `--include` flags.

## Evidence
- Observed 4+ times in session 93a33be3: grep with `--include=*.cpp`, `--include=*.h`, `--include=*.cc` flags all returned "(eval):1: no matches found"
- Workaround: `grep -rl "pattern" . 2>/dev/null | grep -v "/\.a-cx/worktrees|/\.git"` succeeds consistently
- Pattern: Zsh doesn't recognize the `--include=` syntax the same way as bash; it treats it as a literal glob pattern instead of a filter option

## Related
- [[code_exploration_grep_then_read.md]] — grep workflow after fixing syntax
