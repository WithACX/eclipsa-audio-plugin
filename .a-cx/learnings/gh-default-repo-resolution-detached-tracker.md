---
id: gh-default-repo-resolution-detached-tracker
trigger: "when running a bare `gh issue view <n>` / `gh issue list` (no `--repo` flag) in this repo"
confidence: 0.6
domain: workflow
scope: project
date: 2026-07-15
---
# Bare `gh issue` commands silently resolve to the wrong repo here

## Action
This repo's `.a-cx/github.yaml` sets a detached tracker
(`tracker.repo: WithACX/eclipsa-audio-planning`) -- issues live in that
separate planning repo, not in this code repo. But `gh`'s default repo
resolution for this working directory (`gh repo view`) resolves to the
public upstream `google/eclipsa-audio-plugin` (the fork parent), not
`origin` (`WithACX/eclipsa-audio-plugin`) and not the tracker repo. A bare
`gh issue view <n>` therefore does not error -- it silently returns
whatever issue number `<n>` happens to exist in the unrelated upstream
repo, which can look plausible (a real title/body) while being completely
wrong. Always pass `--repo WithACX/eclipsa-audio-planning` explicitly for
issue reads/writes in this repo; never trust a bare `gh issue` result
here.

## Evidence
- Observed once during the #42 delivery run: `gh issue view 42` (no
  `--repo`) returned a closed, unrelated issue ("Test logic pro fixes")
  from `google/eclipsa-audio-plugin`. The real issue #42
  ("[BUG] General UI Cleanup") only appeared when querying
  `--repo WithACX/eclipsa-audio-planning` directly.
