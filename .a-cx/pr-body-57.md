Closes WithACX/eclipsa-audio-planning#57

## What this delivers

[PAN-01.3] Add the height indicator to the top-down panner.

The vertical screen axis of the top-down panner carries front/back, so height has no axis
of its own. This adds the indicator that makes it readable: the room's horizontal
cross-section at the source's current height, drawn as an unfilled `controlBlue` square
with a line along all four sides, plus two leader lines tying the source to it. It is
drawn over the elevation surface and under the source marker.


## How it works

- **`common/components/src/room_views/HeightIndicator.h`** (new) carries the geometry as
  pure functions over 3D anchors: `crossSectionOutline(height)` returns the four sides at
  the room bounds `(+/-1, height, +/-1)`, and `leaderLines(source)` returns the two
  connectors -- one to the back edge (NDC z = +1, which this transform renders at the
  bottom of the view) at the source's own left/right position, one to the right edge
  (NDC x = +1) at the source's own front/back position. Both connector endpoints share the
  source's height exactly, which is what keeps them attached once the perspective divide
  scales the outline. Header-only and in a named namespace for the same two reasons
  `ElevationSurfaces.h` gives: a test can only reach it from a header, and
  `PerspectiveRoomViews.cpp` is unity-built into `components.cpp` where an anonymous
  namespace is shared across the whole translation unit.
- **The perspective behavior is not implemented, it is inherited.**
  `getTopViewTransform()`'s w is `5 - height`, so a higher source divides by less and
  projects further from the centre: the outline expands toward the walls as the source
  rises and contracts as it falls. Nothing scales it by hand, and the test asserts the
  ratio the transform implies rather than a tuned constant.
- **Height comes from the z position parameter through `Coordinates::toRoomNdc`**, applied
  in `PerspectiveRoomView::transformDynamicVertices` -- the one place that scale is
  expressed. `ElevationListener` recomputes z from x and y and writes it back through
  `setZPosition` for Tent, Arch, Dome, and Curve, so reading the parameter is correct
  under every elevation pattern and this view derives no height of its own. There is no
  per-pattern branch in the indicator.

## Two changes worth reviewing on their own

- **`DrawableTrack` gains `ndcPos`** (`PerspectiveRoomView.h`). The window position alone
  has lost the height, and the projection is not invertible without it, so the derived
  view had no route back to the source's height (`tracks_` is private). The member is
  populated in `transformDynamicVertices` from the point that function already computes,
  and defaults to the room's centre so a view painting before any track data arrives reads
  a defined value. No existing consumer reads it.
- **The three `constexpr` view transforms moved from `Coordinates.cpp` into
  `Coordinates.h`.** They were declared in the header and defined only in the `.cpp`,
  which is not usable: a `constexpr` function must be defined in every translation unit
  that calls it, so `Coordinates::getTopViewTransform()` was an undefined symbol for every
  caller outside that file. The new test hit exactly that link error. The values are
  unchanged byte for byte; only their location moved. `getIsoViewTransform` is not
  `constexpr` (it composes matrices at run time) and stays in the `.cpp`.

## Testing

`common/components/tests/HeightIndicator_test.cpp` (8 tests, registered in the tests
`CMakeLists.txt`). The anchor-placement tests assert on the anchors directly, where the
expected values are exact; the perspective tests run against the real
`getTopViewTransform()`, because the properties are properties of that transform:

- the outline's anchors rest on the room bounds at the requested height;
- the four sides form one closed loop (no gap, no retraced edge);
- height reaches the outline through the mapping helper -- a z parameter at
  `kPositionExtent` puts the outline at the ceiling, which is what would fail if the scale
  were open-coded and drifted;
- each connector runs from the source to its edge, at the source's own position on the
  other axis;
- every connector endpoint shares the source's height, at five heights;
- the projected outline expands with height by exactly the `(5 - h)` ratio;
- it stays concentric with the room at every height;
- both connectors land on their outline edge in two differently shaped windows -- the
  resize check.

Full suite: `ctest` 304/304 passed, run serially (`FileOutputTests` are not
parallel-safe). `clang-format --style=file:.clang-format --dry-run -Werror` clean across
`common/`, `rendererplugin/`, and `audioelementplugin/`.

**Still to verify by hand.** The rendering itself has no automated harness (the plan's
O5). Both VST3 targets were built and installed from this branch for that purpose. What
to look at: one source marker (not two), the outline resting on the room's sides, the
outline expanding and contracting as the z dial moves, the connectors staying attached
through that and through a window resize, and the indicator reading correctly under each
of the five elevation patterns. Sibling #58 (PAN-01.4) is the filed DAW verification task.

## Found while delivering this, filed not fixed

**#69 -- room view display flags are read uninitialized on the first paint.**
`PerspectiveRoomView`'s `displaySpeakers_`, `displayTracks_`, and `displayLabels_` have no
initializer and neither constructor sets them, while `paint()` reads all three. For the
panner specifically, `setDisplayTracks` is never called at all, so an indeterminate true
makes the base class draw the source marker underneath this view's own override. Left
alone here because correcting it changes what the renderer plugin draws on its first
frame, which needs its own verification.

## `chore(a-cx)` commit

This branch carries a `chore(a-cx)` commit of A-CX internal work product, separate from
the contribution commit by content. It includes files adopted by the carry-forward capture
that a previous session left uncommitted in the shared clone and that are **unrelated to
this issue** -- read them as content, not as a file list:

- `.a-cx/learnings/elevation-surfaces-grep-strategy.md`
- `.a-cx/learnings/lsp-diagnostics-vs-compile-truth.md`
- `.a-cx/learnings/vst3-build-verification-for-room-views.md`
- `.a-cx/runs/autonomous/aut-20260813-57.json`
