---
id: bash-grep-patterns-for-build-verification
trigger: "when polling or verifying a CMake build in this repo"
confidence: 0.88
domain: tooling
scope: project
date: 2026-09-03
---

# Bash Grep Patterns for Build Verification

## Action

When monitoring or verifying a CMake build, use these grep patterns in order:

1. **Progress**: `grep -oE "^\[ *[0-9]+%\]" logfile | tail -1` -- extract the current build percentage  
2. **Errors**: `grep -E "error:|Error [0-9]+|FAILED" logfile | grep -vi warning | head -5` -- find real errors, excluding warnings  
3. **Component reach**: `grep -E "componentname|targetname" logfile | tail -N` -- confirm your target has been compiled  
4. **Artifacts**: `find build -name "targetname" -type f` -- verify the binary exists after build completes  

Each pattern must filter warnings via `grep -vi warning` or the signal is noise.

## Pattern

Do not read test results until the build reaches `[100%]` and the artifacts
exist -- before that there is nothing to read. Use:

```bash
# Poll progress:
grep -oE "^\[ *[0-9]+%\]" "$SP/build.log" | tail -1

# Check for stopper errors (not warnings):
grep -E "error:|Error [0-9]+|FAILED|fatal error|undefined symbol" "$SP/build.log" \
  | grep -v -i "warning"

# Verify the target compiled by name (or unit test number):
grep -E "Building CXX object.*components|test_room_coordinates" "$SP/build.log" | tail -5

# After [100%], verify artifacts:
find build -name "eclipsa_tests" -type f
find build -name "*.vst3" -maxdepth 8
ctest -C Release --output-on-failure  # only after artifacts exist
```

Put the log in the scratchpad and run the build with `run_in_background`, which
notifies on exit -- do NOT poll on a timer for completion. Poll only to answer a
specific question ("has it reached my file yet?"). For early failure detection,
arm a Monitor on the log filtered to real errors, and exclude `warning` or
`ld: warning:` lines will flood it.

## Evidence

Observed 4+ times in session 3fad9120:

- Event 5: `grep -oE "^\[ *[0-9]+%\]"` returns `[ 67%]`; error check finds none; target hasn't compiled yet  
- Event 7: Progress `[ 73%]`; `grep -cE "components"` returns `29` (tracking compilation)  
- Event 11: Final build line confirms `[100%] Built target eclipsa_tests`; three ld warnings present (not errors)  
- Event 17: Format check uses `clang-format --dry-run -Werror` for all three directories in one go  
- Event 42: Post-push, `gh pr checks` shows Clang Format pass, matrix skipping (expected for this repo's CI config)

Patterns are composable -- they can run in a loop between builds or in a watch script. The key is separating progress (tail-1) from errors (filtered warnings) from completeness (artifacts exist).
