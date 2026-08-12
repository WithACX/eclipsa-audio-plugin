---
id: fork-setup-decisions-admin-guards
trigger: "when running /setup-repo on an external fork (not the primary A-CX repo)"
confidence: 0.75
domain: process
scope: project
date: 2026-08-12
---

# Fork-Specific Setup Decisions: Admin Bypass, Guard Coverage, Workflows

## Action
When /setup-repo runs on an external fork, ask explicit questions about three fork-specific decisions: admin bypass standing authorization, upstream guard coverage, and whether to install PR reminder workflows.

## Evidence
- Event logs from sessions 99d31894 show **2 separate AskUserQuestion blocks** (line 7-8, 48-49)
- Line 7-8: Q1 = Handle stale commit? Q2 = Guard location (live .git/hooks vs tracked .a-cx/hooks)?
- Line 48-49: Q1 = Admin bypass? Q2 = PR reminders on a public fork? Q3 = /github step?
- Same questions appear across independent sessions, indicating they are systematic decisions, not one-off edge cases
- Pattern: Line 54-57 show the chosen answers (admin_bypass: true, guard coverage: yes, /github step: skip) are then applied to config files

## Details
This fork (WithACX/eclipsa-audio-plugin) is public and forks google/eclipsa-audio-plugin. Three setup decisions have fork-specific correct answers:

1. **Admin bypass (deliver.admin_bypass)**: Set `true` to allow /deliver-code to merge PRs with `gh pr merge --admin` when a branch ruleset blocks. This is appropriate on A-CX-managed forks where we have admin rights and want to avoid manual approval loops.

2. **Guard coverage (scm.internal_paths in .a-cx/github.yaml)**: Add `.github/workflows/acx-dev-ci.yml` (and any other A-CX-specific workflows) to internal_paths. The default guard does not cover these, so without this addition they would ride upstream in a PR to google/eclipsa-audio-plugin.

3. **PR reminder workflows**: Do NOT install `.github/workflows/acx-pr-ready-reminder.yml` or `acx-pr-reviewer-reminder.yml` on a public fork. The org-central sweep already covers reminders, and putting A-CX-specific workflows in a public repo violates separation of concerns. Instead, add guard coverage for `acx-dev-ci.yml` so it is not pushed upstream.

## How to Apply
When running `/setup-repo` on this fork, expect three questions:
1. For admin bypass: Answer **true** (standing authorization, since you have admin rights and want /deliver-code to merge cleanly)
2. For guard coverage: Answer **true** (add scm.internal_paths covering `.github/workflows/acx-*.yml`)
3. For /github sync: Answer **skip** (label/template provisioning runs org-central; do not run it on public forks)

Update `.a-cx/process.yaml` and `.a-cx/github.yaml` accordingly during /setup-repo, or manually if updating an existing setup.

Related: [[fork-topology-worktree-isolation]]
