# Eclipsa Audio Plugins

## Project Overview

Eclipsa Audio Plugins are audio plugins for authoring and rendering
immersive audio in the IAMF (Immersive Audio Model and Formats) ecosystem.
The repo ships two plugins: an Audio Element plugin for authoring audio
elements and a Renderer plugin for mixing and monitoring. Both build as
VST3, AU, and AAX formats for macOS and Windows DAWs.

This is a public fork of `google/eclipsa-audio-plugin`. A-CX contributes
changes back upstream. See "Branch Topology and Upstream" before committing.

"Groove" is A-CX's internal name for this project. It began as the private
`WithACX/groove` repo, was upstreamed to `google/eclipsa-audio-plugin`, and
A-CX now contributes through this fork. Internal planning lives in
`WithACX/eclipsa-audio-planning`, tracked on the org "Groove" board
(project #1).

## Tech Stack

- C++ (JUCE framework) for plugin UI and audio processing.
- CMake 3.29+ with Ninja as the build generator.
- vcpkg and CMake FetchContent for dependencies; Git LFS for binary assets.
- Protobuf for serialized data structures.
- Intel oneAPI MKL on Windows.
- GoogleTest for unit tests.

## Architecture

- `audioelementplugin/` -- the Audio Element authoring plugin (`src/`, `test/`).
- `rendererplugin/` -- the Renderer plugin (`src/`, `test/`).
- `common/` -- code shared by both plugins:
  - `components/` -- reusable JUCE UI components.
  - `data_repository/` -- in-memory state repositories.
  - `data_structures/` -- domain models and Protobuf-backed types.
  - `processors/` -- audio processing nodes.
  - `substream_rdr/` -- substream rendering.
  - `logger/` -- logging.
- `cmake/` -- toolchains, prebuilt libs, and build helpers
  (`eclipsa_build_tests.cmake` wires GoogleTest).
- `scripts/` -- build and packaging helpers.
- `third_party/` -- vendored dependencies.

## Building

Requires Git LFS installed (`git lfs install`) before cloning assets.

```
cmake -B ./build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build ./build
```

Add `-DBUILD_AAX=ON` for AAX format (requires the Avid AAX SDK). On Windows,
pass `-DMKL_ROOT=...`, `-DVCPKG_ROOT=...`, and
`-DVCPKG_TARGET_TRIPLET=x64-windows`. See `README.md` for full platform setup.

## Testing

- Framework: GoogleTest, discovered via CTest.
- Configure with tests enabled: add `-DCI_TEST=ON -DBUILD_TESTING=ON` to the
  CMake generate step.
- Run: `ctest` from the `build/` directory.
- Tests live in each plugin's `test/` folder. Add a test alongside the code
  it covers and register it in that folder's `CMakeLists.txt`.

## Coding Standards

Follow the A-CX coding, naming, and security standards provided by the active
plugins (see the `coding-standards`, `naming-code`, and `security` skills).
Match the surrounding JUCE and Google C++ style already in the file you edit.
Capture architecture decisions as ADRs (`architecture-decision-records` skill)
when choosing a library, format, or pattern.

Run `clang-format` on every changed `.h`/`.cpp` file before committing:

```
clang-format --style=file:.clang-format -i <changed files>
```

Both `upstream`'s CI (`cmake-multi-platform.yml`) and this fork's CI
(`acx-dev-ci.yml`) enforce `.clang-format` with `-Werror` on `common/`,
`rendererplugin/`, and `audioelementplugin/`. An unformatted file fails the
check and blocks the PR -- it does not autofix in CI.

CI lints the entire `common/`, `rendererplugin/`, and `audioelementplugin/`
trees, not just this commit's diff -- a violation left by an earlier commit
on the same branch (or any file you didn't personally touch) fails the check
too. Before every commit, run the same dry-run CI runs across all three
directories, not only the files you just edited:

```
find common -iname '*.h' -o -iname '*.cpp' | xargs clang-format --style=file:.clang-format --dry-run -Werror
find rendererplugin -iname '*.h' -o -iname '*.cpp' | xargs clang-format --style=file:.clang-format --dry-run -Werror
find audioelementplugin -iname '*.h' -o -iname '*.cpp' | xargs clang-format --style=file:.clang-format --dry-run -Werror
```

## Branch Topology and Upstream

This repo is a fork of a non-A-CX public upstream. Keeping A-CX tooling out of
the upstream diff is a hard rule:

- `main` mirrors `google/eclipsa-audio-plugin` cleanly. Never commit A-CX
  tooling (`.a-cx/`, `.claude/`, `AGENTS.md`, `CLAUDE.md`) to `main`.
- `acx/dev` branches off `main` and carries the tooling. The team works here.
- Cut every upstream-bound PR from `main`, never from `acx/dev`.
- A local `pre-push` git hook blocks pushes to the `upstream` remote whose
  diff touches guarded A-CX paths. It is not a secrets scanner; normal
  secrets hygiene still applies.
- Contribute upstream with `/upstream-pr` (runs `.a-cx/bin/acx-upstream-pr.sh`);
  it cuts a clean branch from `main`. Full developer guide with git commands:
  `.a-cx/docs/fork-workflow.md`.
- Use a dedicated `git worktree` for any commit or delivery work rather than
  editing directly in the primary checkout -- even a small, no-PR fix. The
  primary tree may be actively checked out by another session or agent; a
  branch switch there mid-task can collide with your work. Create one with
  `git worktree add .a-cx/worktrees/<slug> -b <branch>`, do all edits and
  commits inside it, then `git worktree remove` (and `git worktree prune`)
  once the work is pushed or merged.

Internal planning and issues live in `WithACX/eclipsa-audio-planning`, tracked
on the org "Groove" board. Do not open A-CX planning issues on this public fork.
The board's status flow and automations are documented internally in
`.a-cx/docs/board-workflow.md`.

## Documentation

- Root `README.md` is the comprehensive entry point: purpose, platform setup,
  build, and debug. Update it only when top-level structure or purpose changes.
- Folder READMEs are scoped to their folder: why it exists and its conventions.
  Every folder with 3 or more files should have a `README.md`.
- Never duplicate content between docs; link instead.
- README files explain the why and how of a folder (purpose, conventions,
  usage). Do not list individual files.

## After Completing Work

- Added files so a folder now has 3 or more files and no README? Create one.
- Changed a folder's structure or conventions? Update that folder's README.
- Changed the repo's overall purpose or top-level structure? Update root README.
- Made an architectural decision? Capture an ADR.
- Do a local build and verify tests pass (`ctest`) before opening a PR --
  always, not only when asked, and not skipped on the assumption that CI will
  catch it. The `build/_deps` third-party tree is large (multiple GB); reuse
  the existing configured `build/` directory for an incremental build of the
  affected target rather than reconfiguring from scratch in a new worktree or
  checkout.
- Run `clang-format` on every changed `.h`/`.cpp` file before opening a
  PR -- CI enforces it with `-Werror` and a violation blocks the PR (this bit
  an upstream-bound PR that only ran `ctest` locally and skipped formatting).
- Confirm you are on `acx/dev` (or a branch off it) for tooling and internal
  changes; cut upstream-bound PRs from `main`.

## What NOT to Do

- Do not commit A-CX tooling files to `main` or include them in an
  upstream-bound PR.
- Do not commit binary assets outside Git LFS.
- Do not disable tests or the CMake test build to make CI green.
- Do not open internal A-CX planning issues on this public fork.
- Do not add issue/ticket numbers to code comments (e.g. "Issue #38: ...").
  Comments should explain the code on its own terms; issue references rot
  as trackers change and belong in the commit message or PR description
  instead.
- Do not use multi-line block comments above a class-level member variable
  declaration. Keep it to a simple inline comment on the same line (e.g.
  `long framesWritten_;  // samples handed to the writers this export`).
