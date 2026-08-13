---
id: vst3-manual-testing-build
trigger: "when a feature modifies room_views rendering or audio panning UI, or when acceptance criteria include visual verification"
confidence: 0.85
domain: testing
scope: project
date: 2026-08-13
---

# VST3 Plugin Build for Manual Testing

## Action
For features affecting the panner or room-view rendering, reconfigure CMake with `-DBUILD_VST3=ON`, verify both AudioElementPlugin_VST3 and RendererPlugin_VST3 targets exist in the build, and build the binaries (`make AudioElementPlugin_VST3 RendererPlugin_VST3`) so the standalone .vst3 bundles can be manually tested in a DAW.

## Evidence
Observed 6 discrete operation groups during feature implementation (issue #55, PAN-01.1):
1. Check VST3 gating: `grep -rn "BUILD_VST3" CMakeLists.txt` (event 53)
2. Verify JUCE plugin formats: `grep -A3 FORMATS audioelementplugin/CMakeLists.txt` (event 53)
3. Find plugin target names: `juce_add_plugin(AudioElementPlugin)` definitions (event 59)
4. CMake reconfiguration with VST3: `cmake -B build -DBUILD_VST3=ON` (event 62)
5. Confirm VST3 targets in build: `make help | grep VST3` shows AudioElementPlugin_VST3, RendererPlugin_VST3 (event 64)
6. Build VST3 binaries: `make AudioElementPlugin_VST3 RendererPlugin_VST3` (event 65, background task bo877vtwp)

The acceptance criteria for PAN-01.1 specifically require visual confirmation that "the drawn source marker agrees with the x and y position parameters" at the five corners of the parameter space -- a task that cannot be automated and requires manual testing in the DAW.

## Why
This repo uses JUCE, which builds multiple plugin formats (Standalone, VST3, AAX) from a single code path. The Standalone build is sufficient for unit tests and coordinate logic verification, but visual/interactive properties like panner rendering require testing in a real DAW context. The VST3 format is the standard for modern DAWs (Logic, Ableton, Reaper). The CI workflow on this branch does NOT build VST3 by default (acx-dev-ci.yml gates the cross-platform matrix on `workflow_dispatch`, leaving only Clang Format and matrix.os SKIPPED for branch pushes); therefore, manual VST3 builds are the only way to verify rendering before merge.

## How to apply
When working on room_views or panner features:
1. After CMake configure, add `-DBUILD_VST3=ON` to the cmake command
2. Run `make help | grep VST3` to confirm both plugin targets exist
3. Build: `make -j$(sysctl -n hw.ncpu) AudioElementPlugin_VST3 RendererPlugin_VST3`
4. The .vst3 bundles appear in the build directory (COPY_PLUGIN_AFTER_BUILD is ON by default, so they may also be copied to ~/Library/Audio/Plug-Ins/VST3/)
5. Open the bundle in a DAW and manually verify the visual criteria from the issue acceptance_criteria

Related: [[feedback_build_plugin_for_manual_testing.md]], [[eclipsa_fresh_worktree_build_prereqs.md]]
"