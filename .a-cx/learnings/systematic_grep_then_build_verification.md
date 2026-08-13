---
id: systematic-grep-then-build-verification
trigger: "when verifying a feature works end-to-end in both test and manual contexts"
confidence: 0.85
domain: workflow
scope: project
date: 2026-08-11
---

# Systematic Grep-then-Build Pattern for Feature Verification

## Action

When a feature requires both unit test verification and manual plugin testing, follow a consistent pattern: (1) Run full unit test suite to confirm no regressions, (2) Systematically grep CMake and source files to understand build configuration (plugin formats, target names), (3) Attempt targeted plugin builds to verify the code compiles in all required plugin formats (AU, VST3), (4) Document findings in a learning file before committing.

## Evidence

- Observed 5+ times in session 4e411527:
  - User ran `ctest` to verify all 268 tests passed (lines 1-4)
  - User grepped for `FORMATS`, `VST3`, `BUILD_VST3` across CMakeLists.txt and cmake/ directory to understand build configuration (lines 11-18)
  - User attempted to build VST3 plugins, discovered they were not enabled on macOS (ECLIPSA_PLATFORM_PLUGIN_FORMATS defaults to AU only, line 64 of macos.cmake)
  - User pivoted to building AU plugins instead (`RendererPlugin_AU AudioElementPlugin_AU`) which succeeded
  - User created a structured learning file documenting the dismissible banner architecture finding from the feature work
  - User committed with clear multi-line commit messages referencing the GitHub issue

## How to apply

1. **Test verification first**: Run `cd build && ctest` to ensure no regressions before any plugin build attempts.
2. **Grep before building**: Locate CMake configuration using targeted grep across CMakeLists.txt and cmake/ subdirectories—focus on:
   - `ECLIPSA_PLUGIN_FORMATS` and `ECLIPSA_PLATFORM_PLUGIN_FORMATS`
   - `BUILD_VST3` conditionals
   - Target names like `RendererPlugin_AU` vs. `RendererPlugin_VST3`
3. **Build in the enabled formats only**: Check the platform toolchain (`cmake/toolchains/macos.cmake`, etc.) to see which formats are actually enabled before attempting `cmake --build build --target <name>`.
4. **Document the learning**: If the fix involved architectural insight (e.g., dismissible banner state requirements), write a `.a-cx/learnings/<kebab-name>.md` file before committing.
5. **Commit discipline**:
   - Stage all code changes first: `git add <files>`
   - Commit with multi-line message (feat/fix prefix, body, closes issue reference)
   - Separate learning file commit with `chore(a-cx):` prefix

Related: [[clang_format_lints_entire_tree_not_diff]], [[Build plugin for manual testing]]
