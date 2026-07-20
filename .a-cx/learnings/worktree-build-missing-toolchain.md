---
id: worktree-build-missing-toolchain
trigger: "when configuring or extending an existing build/ directory in this repo, especially one created by cloning another tree's build state, or when adding a new plugin format (VST3/AAX) to an existing configured build"
confidence: 0.7
domain: process
scope: project
date: 2026-07-16
---
# A worktree's build/ can silently run without the macOS platform toolchain

## Action
Before trusting an existing `build/` directory's configuration in this repo,
check `grep -i "ECLIPSA_PLATFORM\b" build/CMakeCache.txt`. If it's empty/absent,
the platform toolchain (`cmake/toolchains/macos.cmake`) was never actually
applied to that build directory -- `CMAKE_TOOLCHAIN_FILE` only takes effect on
the very first configure of a fresh build dir and cannot be added retroactively
to an existing one. A build in this state can still succeed for targets that
don't need the toolchain's settings (SAF performance library selection,
`FilePermissions_mac.mm` platform sources, RPATH, `-Wl,-ld_classic`), because
CMake's non-fatal `SEND_ERROR`s let configure continue and reuse already-built
artifacts -- but adding a new target (e.g. `-DBUILD_VST3=ON`) can expose the
gap as real link failures (undefined `startSecurityScopedAccess` et al.) or
even a hard configure error (`ECLIPSA_PLATFORM` empty breaks
`third_party/CMakeLists.txt`'s `include()`). The only clean fix is a fresh
`build/` directory configured with `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos.cmake`
from the start; patching individual cache variables afterward (as a stopgap)
can itself change compiled behavior -- see
[[worktree-vst3-build-changed-saf-checksum]].

## Evidence
- Observed during the #41 delivery run (worktree
  `.a-cx/worktrees/41-export-fails-silently`): this worktree's `build/` had
  been building `eclipsa_tests`, `RendererPlugin`, and `AudioElementPlugin`
  successfully for three implementation phases with `ECLIPSA_PLATFORM` and
  `CMAKE_TOOLCHAIN_FILE` both completely absent from `CMakeCache.txt`. Adding
  `-DBUILD_VST3=ON` first hit a hard `CMake Error` (missing
  `cmake/prebuiltLibs/.cmake`, since `${ECLIPSA_PLATFORM}` was empty), then
  after patching that, a real linker failure for `RendererPlugin_VST3`
  (`FilePermissions_mac.mm` was never compiled in, since
  `ECLIPSA_PLUGIN_PLATFORM_SOURCES` was also never set).
- Reproduced a stronger, more immediate form during the #43 delivery run
  (worktree `.a-cx/worktrees/43-smpte-dolby-speaker-labels`): configuring a
  brand-new, empty `build/` directory without
  `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/macos.cmake` (i.e. following
  AGENTS.md's short-form `cmake -B ./build -DCMAKE_BUILD_TYPE=Release -G
  Ninja` example literally) failed configure outright with `CMake Error at
  third_party/Spatial_Audio_Framework/framework/CMakeLists.txt:128 (message):
  Intel mkl.h not found` -- this time a hard, immediate `SEND_ERROR` on the
  very first configure, not a later silent gap. So the toolchain flag isn't
  only needed to avoid a future link failure on an existing build; omitting
  it can fail the very first configure of a fresh build directory on this
  machine (no `/opt/intel` or `/usr/local/lib/libsaf_mkl_custom_*` present).
  Confidence raised 0.6 -> 0.7. The correct macOS command lives in
  `README.md`'s "Building For MacOS" section, not AGENTS.md's abbreviated
  Ninja example.
