---
id: ci-clang-format-blocks-before-build
trigger: "when pushing a PR branch that touches common/, rendererplugin/, or audioelementplugin/ .h or .cpp files"
confidence: 0.5
domain: process
scope: project
date: 2026-07-15
---
# CI rejects unformatted code before any build or test step runs

## Action
Run `clang-format --style=file:.clang-format -i <changed .h/.cpp files>`
locally before pushing. The `macos-14` CI job (`Cross-Platform Build
(acx/dev)`) runs `find <dir> -iname '*.h' -o -iname '*.cpp' | xargs
clang-format --style=file:.clang-format --dry-run -Werror -i` as its first
step across `common/`, `rendererplugin/`, and `audioelementplugin/`. A single
misformatted line fails the whole job immediately with `code should be
clang-formatted [-Wclang-format-violations]` -- before any actual compilation,
build, or test output, so a passing local build gives no signal that CI will
pass.

## Evidence
- Observed once during the #42 delivery run: a hand-written ternary +
  multi-arg call (`fileSelectLabel_.setText(kNoFileChosen ? ... : ...,
  juce::dontSendNotification)`) in `AudioFilePlayer.cpp` compiled and ran
  correctly locally but failed the `macos-14` CI job purely on
  clang-format-violation errors; `clang-format --style=file:.clang-format -i`
  fixed it and the re-pushed commit passed the formatting step.
