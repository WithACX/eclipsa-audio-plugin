---
id: fetchcontent-base-dir-sharing-breaks-libspatialaudio
trigger: "when reusing another tree's build/_deps cache to speed up a slow or flaky FetchContent download in this repo"
confidence: 0.5
domain: build
scope: project
date: 2026-07-21
---
# Sharing a whole `_deps` tree via `FETCHCONTENT_BASE_DIR` breaks libspatialaudio's generated config.h; prefer a per-dependency `FETCHCONTENT_SOURCE_DIR_<NAME>` override instead

## Action
When one specific FetchContent dependency is slow or flaky to download in a
fresh worktree (see [[fresh-worktree-full-deps-build-time]] for the general
budget), reuse an existing tree's already-populated source for just that ONE
dependency: `-DFETCHCONTENT_SOURCE_DIR_<NAME>=<path>/_deps/<name>-src` (name
uppercased). Do NOT redirect the whole tree with a blanket
`-DFETCHCONTENT_BASE_DIR=<path>/_deps` -- that also repoints every
dependency's BUILD-side directory (`<name>-build`) at the other tree's
already-configured one. For a dependency that generates a header via
`configure_file` into `${CMAKE_CURRENT_BINARY_DIR}` (libspatialaudio's
`include/config.h.in` -> `<build-dir>/config.h`), CMake appears to skip
re-running that generation step when the shared build dir already looks
populated, so consumers in the NEW tree fail with `fatal error: 'config.h'
file not found` even though the configure step itself reports success. This
surfaced specifically through `common/substream_rdr/substream_rdr.h`, so it
blocks anything that pulls in the ADM renderer path -- including the shared
`eclipsa_tests` binary, not just the GUI plugin targets. Reconfiguring with
only the slow dependency's SOURCE_DIR overridden (leaving every BUILD dir
local and fresh) avoids the issue entirely, at the cost of that other
dependency needing its build compiled locally.

## Evidence
- Observed during the #38 delivery run (worktree
  `.a-cx/worktrees/38-audio-video-duration-mismatch-warning`): LibIAMF's
  FetchContent step is a plain ~1.5-2GB URL zip download (not a git clone)
  that failed repeatedly on this network (`Connection reset by peer`, and
  once a silent multi-hour hang) -- confirmed via a direct `curl` test that
  itself needed ~1000s and still hit a mid-transfer reset. Pointing
  `-DFETCHCONTENT_BASE_DIR` at the primary tree's fully-built `_deps`
  configured cleanly and fast (76s), but the subsequent `eclipsa_tests`
  build failed on `libspatialaudio-src/include/hrtf/sofa_hrtf.h:4:10:
  fatal error: 'config.h' file not found` in both `RendererPlugin` and
  `AudioElementPlugin` (via `substream_rdr.cpp`). Reconfiguring with only
  `-DFETCHCONTENT_SOURCE_DIR_LIBIAMF=<primary>/build/_deps/libiamf-src` (no
  blanket `FETCHCONTENT_BASE_DIR`) configured in 76s and built
  `eclipsa_tests` to 100% cleanly.
