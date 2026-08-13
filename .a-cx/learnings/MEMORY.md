# Project Learnings Index

## Workflow & Process
- [Detached repo topology workflow](detached-repo-topology-workflow.md) — Code repo separate from tracker repo; switch --repo flags on gh commands
- [Build + test + format before PR](build-test-format-before-pr.md) — ctest, clang-format, full build before opening PR
- [Worktree isolation for all branches](worktree-isolation-for-all-branches.md) — Even lightweight fixes use isolated worktrees
- [Interactive branch decisions at fork points](interactive_branch_decisions_at_fork_points.md) — Ask user for main vs acx/dev, amend vs new PR strategy at branch decision gates
- [Task-based phase tracking](task-tracking-workflow-phases.md) — Create TaskCreate for each phase, update status as you progress
- [Scheduled waits for long builds](scheduled_waits_for_builds.md) — Use ScheduleWakeup (600–900s) for CMake/long builds, not tight polling
- [Multi-layered process health diagnosis](multilayer_process_health_diagnosis.md) — When a build seems slow, check disk/CPU/network/logs together
- [Upstream PR metadata extraction](upstream-pr-gh-api-python-parsing.md) — Systematically gather PR metadata using gh api --paginate piped to Python parsers
- [Deferred tool schema loading](deferred-tool-schema-loading-pattern.md) — Load deferred tool schemas via ToolSearch before invoking them
- [Scope-anchored code review](scope-anchored-code-review.md) — Anchor reviews to stated change objective; classify findings as in-scope vs adjacent
- [Documentation-first PR review](documentation-first-pr-review.md) — Read pr-review-policy.md, process.yaml, AGENTS.md before launching review agents

## Build & Infrastructure
- [Fresh worktree full deps build time](fresh-worktree-full-deps-build-time.md) — First build in new worktree is slow
- [Worktree build missing toolchain](worktree-build-missing-toolchain.md) — Some builds fail in new worktrees due to missing deps
- [Fresh worktree submodule state recovery](fresh_worktree_submodule_state_recovery.md) — If submodules show staged deletions or missing content, reset --hard and re-init
- [Worktree LFS false-positive diffs](worktree-lfs-false-positive-diffs.md) — Git LFS files show false changes in fresh worktrees
- [Worktree VST3 build changed SAF checksum](worktree-vst3-build-changed-saf-checksum.md) — Build artifacts can differ per worktree
- [LFS pointer file changes in worktree](lfs_pointer_files_in_worktree.md) — LFS binaries show as modified after branch merge; ignore or restore

## Code Patterns & Issues
- [Bidirectional enum symmetry](bidirectional-enum-symmetry.md) — Add opposite-direction cases when adding directional error states; cover both in tests
- [Boundary test pair for numeric constants](boundary_test_pair_for_numeric_constants.md) — When widening a threshold/tolerance, add tests just-inside and just-outside to guard against reversions
- [JUCE writer open check needs stream status](juce-writer-open-check-needs-stream-status.md) — Open/write coordination in JUCE audio writer
- [ValueTree listener writing back to own repository](valuetree-listener-writing-back-to-own-repository.md) — Async ValueTree update feedback loops
- [Upstream PR range sweeps unrelated commits](upstream-pr-range-sweeps-unrelated-commits.md) — Upstream PR helper includes extra commits; verify scope carefully

## Tooling & Process Improvements
- [Agent output completeness check](agent-output-completeness-check.md) — Agent findings may be incomplete; verify edge cases manually
- [Agent output verification via transcript](agent-output-verification-via-transcript.md) — Parse agent `.output` JSON to extract and verify findings completeness
- [Deliver code should surface prior closed PRs](deliver-code-should-surface-prior-closed-prs.md) — Prior closed-unmerged PRs are informative; surface them
- [A-CX tooling bug: deliver-pr-review severity markers](acx_tooling_bug_discovery.md) — Inline comments show [advisory] instead of actual severity; file WithACX/a-cx-ai-config bugs

## Manual Testing
- [Build plugin for manual testing](../../../.claude/projects/-Users-branden-Documents-Code-Automated-eclipsa-audio-plugin/memory/MEMORY.md) — Proactively build VST3 when task needs manual DAW verification

## Code Style
- [Comment each unit test](../../../.claude/projects/-Users-branden-Documents-Code-Automated-eclipsa-audio-plugin/memory/MEMORY.md) — One-line comment above every TEST/TEST_F
- [Clang-format CI gate requirement](clang_format_ci_gate.md) — Run clang-format locally before push to fix CI formatting failures
