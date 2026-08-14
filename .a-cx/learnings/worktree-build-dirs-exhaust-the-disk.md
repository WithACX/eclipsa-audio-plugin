---
id: worktree-build-dirs-exhaust-the-disk
trigger: "before configuring CMake in a fresh worktree of eclipsa-audio-plugin, or when a FetchContent extract fails with 'Problem extracting tar' / archive_write_header"
confidence: 0.8
domain: process
scope: project
date: 2026-08-13
---
# Check free disk before configuring: each worktree build is 5-10 GB

## Action

Run `df -h` and `du -sh .a-cx/worktrees/*/build` before the first `cmake -B build`
in a new worktree, and delete the `build/` directories of worktrees whose issues
are already merged. Each one is 5-10 GB, they are gitignored and fully
regenerable, and nine of them had accumulated to 71 GB.

The failure this prevents does not name the disk. `FetchContent` downloads a
1.77 GB libiamf archive; with the disk full the download truncates and the
extract fails, so the message looks like a corrupt dependency:

```
CMake Error: Problem with archive_write_header(): Can't create
  'libiamf-.../code/dep_external/lib/binaural/libboost_filesystem.so.1.82.0'
CMake Error: Problem extracting tar: .../48b8b5dc...zip
```

The tell is the zip size: 764 MB on disk against 1.77 GB in a healthy worktree.
Free space, delete the partial `build/`, and re-run configure -- it succeeds
with no other change.

Delete build directories only, never another run's worktree, and only for
worktrees whose delivery is finished.

## Evidence

- Observed delivering PAN-01.3 (#57): configure failed at 116 MB free;
  reclaiming five merged worktrees' build directories freed 44 GB and configure
  then completed with no change to the build itself.
