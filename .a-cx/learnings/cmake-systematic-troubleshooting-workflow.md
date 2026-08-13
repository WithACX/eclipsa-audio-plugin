---
id: cmake-systematic-troubleshooting-workflow
trigger: "when CMake configuration fails during build setup"
confidence: 0.82
domain: workflow|process
scope: project
date: 2026-08-11
---

# CMake Systematic Troubleshooting Workflow

## Action
When cmake configuration fails, use background process management + tail logs + grep error patterns + flag isolation + clean rebuild to diagnose platform and dependency issues methodically.

## Evidence

- **Observed 3 times in session 4e411527-a5dd-4709-abbc-1a990f5074e6**
- Pattern: nohup cmake... > /tmp/log 2>&1 & → while kill -0 $PID sleep → tail -N /tmp/log → grep CMake Error
- Each iteration reveals a specific blocker (missing MKL, missing AAX SDK, missing platform toolchain include)

### Workflow Steps

1. **Background CMake Run**
   ```bash
   nohup cmake -B build [flags] > /tmp/cmake_config.log 2>&1 &
   echo "PID: $!"
   ```

2. **Wait for Completion**
   ```bash
   while kill -0 $PID 2>/dev/null; do sleep 3; done
   tail -30 /tmp/cmake_config.log
   ```

3. **Locate Error Lines**
   ```bash
   grep -n "CMake Error\|error:" /tmp/cmake_config.log
   ```

4. **Isolate Root Cause**
   - Read error context (sed lines around error)
   - Grep CMakeLists.txt for related config flags
   - Check CMakeCache.txt for affected variables

5. **Adjust & Retry**
   - Build flag hypothesis (e.g., BUILD_AAX=OFF if AAX SDK missing)
   - On config-file errors: clean cache `rm -rf build/CMakeCache.txt build/CMakeFiles`
   - Re-run from step 1

## Why

Eclipsa's build system has multiple optional features (VST3, AAX) and platform-specific logic. Failures often stem from:
- Missing optional dependencies (AAX SDK, MKL headers)
- Platform toolchain not loaded (ECLIPSA_PLATFORM variable unset)
- Stale CMake cache from prior failed configuration

Systematic diagnosis prevents guessing and surfaces the actual blocker.

## Related

- [[fresh-worktree-build-prereqs]] - prerequisites before first build
- [[build-plugin-for-manual-testing]] - building after successful cmake

