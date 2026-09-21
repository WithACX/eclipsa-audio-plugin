# Project Learnings Index

## Workflow & Process
- [Detached repo topology workflow](detached-repo-topology-workflow.md) -- Code repo separate from tracker repo; switch --repo flags on gh commands
- [Build + test + format before PR](build-test-format-before-pr.md) -- ctest, clang-format, full build before opening PR
- [Worktree isolation for all branches](worktree-isolation-for-all-branches.md) -- Even lightweight fixes use isolated worktrees
- [Worktree experiment isolation pattern](worktree-experiment-isolation-pattern.md) -- Create detached worktree for diagnostics or -b branch for authoring, test locally, delete when done
- [CI simulation via YAML extraction](ci-simulation-yaml-extraction-pattern.md) -- Extract workflow step's `run` block as shell, execute locally to reproduce CI state before pushing
- [Interactive branch decisions at fork points](interactive_branch_decisions_at_fork_points.md) -- Ask user for main vs acx/dev, amend vs new PR strategy at branch decision gates
- [Task-based phase tracking](task-tracking-workflow-phases.md) -- Create TaskCreate for each phase, update status as you progress
- [Scheduled waits for long builds](scheduled_waits_for_builds.md) -- Use ScheduleWakeup (600–900s) for CMake/long builds, not tight polling
- [Multi-layered process health diagnosis](multilayer_process_health_diagnosis.md) -- When a build seems slow, check disk/CPU/network/logs together
- [Upstream PR metadata extraction](upstream-pr-gh-api-python-parsing.md) -- Systematically gather PR metadata using gh api --paginate piped to Python parsers
- [Deferred tool schema loading](deferred-tool-schema-loading-pattern.md) -- Load deferred tool schemas via ToolSearch before invoking them
- [Scope-anchored code review](scope-anchored-code-review.md) -- Anchor reviews to stated change objective; classify findings as in-scope vs adjacent
- [Documentation-first PR review](documentation-first-pr-review.md) -- Read pr-review-policy.md, process.yaml, AGENTS.md before launching review agents
- [Pinned clang-format verification workflow](clang-format-verification-workflow.md) -- Use scratchpad venv with clang-format 23.1.0 before push; CI enforces exact version
- [Build incrementally with cmake --target, then run filtered ctest](cmake-ctest-cycle.md) -- Rapid iteration: target build → filtered test pattern
- [Filter build and test output with grep](grep-output-filtering.md) -- Use grep -E to isolate errors/failures from cmake/ctest output
- [Capture gate sequence for A-CX work product](capture-gate-sequence.md) -- Run capture_internal_artifacts.py --check after commits to .a-cx/learnings/, inspect withheld paths, capture + push + re-gate
- [A-CX leak guard workflow](acx-leak-guard-workflow.md) -- Use capture_internal_artifacts.py + check_upstream_leak.py to auto-capture .a-cx/ files and verify config-only classification
- [Observer-written learnings require human review](observer-learnings-require-human-review.md) -- Always read observer-generated learnings and correct factual errors before capture; they become live documentation
- [Python pathlib for bulk text edits](python-pathlib-for-bulk-text-edits.md) -- Use pathlib + tuple replacement list over sed/awk for multi-line comment refactors
- [Git diff verify refactored comments](git-diff-verify-refactored-comments.md) -- After bulk edits, grep new lines for ticket refs and forbidden patterns before staging
- [Close-predicate gate workflow](close-predicate-gate-workflow.md) -- Run close_predicate.py before closing a parent to verify all five conditions hold
- [Read-back state verification](read-back-state-verification.md) -- After any GitHub state change, read it back from the API to confirm it persisted
- [Acceptance criteria accuracy](acceptance-criteria-accuracy.md) -- Never tick criteria without observed evidence; move misfiled criteria to child issues instead

## Build & Infrastructure
- [Fresh worktree full deps build time](fresh-worktree-full-deps-build-time.md) -- First build in new worktree is slow
- [Worktree build missing toolchain](worktree-build-missing-toolchain.md) -- Some builds fail in new worktrees due to missing deps
- [Fresh worktree submodule state recovery](fresh_worktree_submodule_state_recovery.md) -- If submodules show staged deletions or missing content, reset --hard and re-init
- [Worktree LFS false-positive diffs](worktree-lfs-false-positive-diffs.md) -- Git LFS files show false changes in fresh worktrees
- [Worktree VST3 build changed SAF checksum](worktree-vst3-build-changed-saf-checksum.md) -- Build artifacts can differ per worktree
- [LFS pointer file changes in worktree](lfs_pointer_files_in_worktree.md) -- LFS binaries show as modified after branch merge; ignore or restore

## Code Patterns & Issues
- [Bidirectional enum symmetry](bidirectional-enum-symmetry.md) -- Add opposite-direction cases when adding directional error states; cover both in tests
- [Boundary test pair for numeric constants](boundary_test_pair_for_numeric_constants.md) -- When widening a threshold/tolerance, add tests just-inside and just-outside to guard against reversions
- [JUCE writer open check needs stream status](juce-writer-open-check-needs-stream-status.md) -- Open/write coordination in JUCE audio writer
- [ValueTree listener writing back to own repository](valuetree-listener-writing-back-to-own-repository.md) -- Async ValueTree update feedback loops
- [Upstream PR range sweeps unrelated commits](upstream-pr-range-sweeps-unrelated-commits.md) -- Upstream PR helper includes extra commits; verify scope carefully
- [Python anchor-text replacement pattern](python-anchor-text-replacement-pattern.md) -- Use pathlib + unique anchor for multi-line C++ insertions; atomic and offset-safe

## Tooling & Process Improvements
- [Tool version byte-identity verification](tool-version-byte-identity-verification.md) -- Before pinning via one package manager, verify byte-identity across package managers on full tree
- [Agent output completeness check](agent-output-completeness-check.md) -- Agent findings may be incomplete; verify edge cases manually
- [Agent output verification via transcript](agent-output-verification-via-transcript.md) -- Parse agent `.output` JSON to extract and verify findings completeness
- [Deliver code should surface prior closed PRs](deliver-code-should-surface-prior-closed-prs.md) -- Prior closed-unmerged PRs are informative; surface them
- [A-CX tooling bug: deliver-pr-review severity markers](acx_tooling_bug_discovery.md) -- Inline comments show [advisory] instead of actual severity; file WithACX/a-cx-ai-config bugs
- [Python one-liners for structured files](python-one-liners-for-structured-files.md) -- Use Python heredocs in Bash for JSON/Markdown edits instead of sed; cleaner and portable
- [Use Python inline scripts for bulk file edits with assertions](python-bulk-edits.md) -- Chain .replace() calls with assertions for coordinated changes

## Manual Testing
- [Build plugin for manual testing](../../../.claude/projects/-Users-branden-Documents-Code-Automated-eclipsa-audio-plugin/memory/MEMORY.md) -- Proactively build VST3 when task needs manual DAW verification
- [Plugin binary verification workflow](plugin-binary-verification-workflow.md) -- After build, verify installed bundles via SHA256 hash match + symbol inspection
- [DAW cache inspection](daw-cache-inspection.md) -- When DAW doesn't pick up updated builds, check plugin cache files for stale entries and force rescan

## Code Style
- [Comment each unit test](../../../.claude/projects/-Users-branden-Documents-Code-Automated-eclipsa-audio-plugin/memory/MEMORY.md) -- One-line comment above every TEST/TEST_F
- [Clang-format CI gate requirement](clang_format_ci_gate.md) -- Run clang-format locally before push to fix CI formatting failures
- [Em-dash normalization requirement](em-dash-normalization-requirement.md) -- Replace U+2014 em-dashes with ASCII ` -- ` in learnings files before capture; CI validator rejects them
- [Use plugin scripts for issue mutations](a-cx-plugin-scripts-pattern.md) -- dispatch issue_cli/board_cli rather than raw gh for writes
- [Run gates before mutating state](gates-before-mutation.md) -- check blockers, sub-issues, assignment and criteria before a close
- [gh + jq for state reads](gh-cli-jq-state-queries.md) -- structured reads for every gate readback; issue writes still go through issue_cli
- [Verify mutations immediately](verify-mutations-immediately.md) -- read back after every write; the API can return success without landing it
- [Observer clobbers the learnings index](observer-clobbers-the-learnings-index.md) -- it rewrites MEMORY.md with only its own new entries; the capture withhold is the only thing preventing data loss
- [Structured diagnostic Bash scripts](structured-diagnostic-bash-scripts.md) -- Use section headers, formatted output, helper functions, and conditional fallback messages for readable diagnostics
