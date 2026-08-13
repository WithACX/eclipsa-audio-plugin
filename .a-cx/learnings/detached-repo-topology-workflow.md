---
id: detached-topology-workflow
trigger: "when working with .a-cx/github.yaml that specifies a separate tracker.repo"
confidence: 0.92
domain: workflow
scope: project
date: 2026-07-21
---

# Detached Repo Topology Workflow

## Action
Always query issue metadata, board status, and labels against the separate tracker repo (`WithACX/eclipsa-audio-planning`), while making code changes, commits, and PRs in the code repo (`WithACX/eclipsa-audio-plugin`). Use `--repo` flags on `gh` commands to switch context.

## Evidence

- Observed 5+ times in session 0932d151
- Pattern: github.yaml declares `tracker.repo: WithACX/eclipsa-audio-planning` separate from code repo
- Workflow repeatedly queries tracker repo (issue view, sub-issues, blocked-by relations, labels, board state) before and after claiming worktrees in the code repo
- All board/assignment operations target the tracker repo; all code operations target the code repo
- This keeps issue management orthogonal from code management

## Why

Detached topology enables independent planning/board cadence from code work. Issues can be triaged and scheduled without blocking code deployments. The split also mirrors A-CX's typical multi-repo setups (planning repo + code repos).

## How to Apply

When `.a-cx/github.yaml` contains `tracker.repo` != current repo:
1. Always add `--repo WithACX/eclipsa-audio-planning` to `gh issue`, `gh label`, `gh api graphql` commands that query metadata, labels, or board state.
2. Omit `--repo` (or use `--repo WithACX/eclipsa-audio-plugin`) for code operations: worktree, branch, commit, PR, push.
3. Check github.yaml early in every deliver-code run to detect which repo is the tracker vs which is code.
