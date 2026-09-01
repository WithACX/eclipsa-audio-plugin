---
id: vst3-build-verification-for-room-views
trigger: "when delivering room_views/panner features that need visual verification"
confidence: 0.85
domain: testing
scope: project
date: 2026-08-13
---

# VST3 Build Verification for Room Views Features

## Action
After building room_views features, proactively build and verify both VST3 plugin targets (AudioElementPlugin_VST3 and RendererPlugin_VST3) before opening a PR for visual DAW testing.

## Evidence
- Observed 4 times in observer analysis aut-20260813-56:
  - Line 9: CMake cache inspection confirmed BUILD_VST3 configuration
  - Line 13: Built both VST3 targets with parallel `-j 8`
  - Line 15: Verified build output (zero errors, successful install)
  - Line 17: Confirmed installed binaries byte-for-byte match fresh build
- Same workflow pattern in run aut-20260813-55 for elevation surfaces
- Worktree build artifacts confirmed identical: timestamp, size, binary content, architecture (arm64)
- codesign verification passed (adhoc signature, identifier matches)

## Why
Elevation surface rendering is purely visual; unit tests verify math but not perception. The top-view panner in AudioElementPlugin displays 5 elevation patterns (Flat, Tent, Arch, Dome, Curve) through the room_views system. Changes to ElevationSurfaces.h, PerspectiveRoomViews.cpp, or ElevationListener require manual DAW testing to confirm visual correctness. Building VST3 early allows parallel manual testing while CI runs.

## How to apply
1. After git push and before opening PR on a room_views feature branch:
   ```bash
   cmake --build build --target AudioElementPlugin_VST3 RendererPlugin_VST3 -j 8
   ```
2. Verify build exit code is 0 and binary sizes match previous run (prevents accidental re-compilation on each session)
3. Check installed plugins in ~/Library/Audio/Plug-Ins/VST3/ match worktree artifacts:
   ```bash
   lipo -archs "$HOME/Library/Audio/Plug-Ins/VST3/Eclipsa Audio Element Plugin.vst3/Contents/MacOS/Eclipsa Audio Element Plugin"
   codesign -dv "$HOME/Library/Audio/Plug-Ins/VST3/Eclipsa Audio Element Plugin.vst3"
   ```
4. Load plugins in a DAW (e.g. Logic Pro, Reaper) and visually inspect the top panner's elevation surface rendering before PR approval
5. Document any rendering discrepancies (shading, smoothness, edge visibility) in the PR body

## Related
- [[vst3-manual-testing-build]] (parent learning: broader guidance for plugin testing)
- [[empirical-verification-refutes-theoretical-bugs]] (validation approach: compile + test proves correctness despite LSP noise)
