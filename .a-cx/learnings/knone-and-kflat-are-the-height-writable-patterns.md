---
id: knone-and-kflat-are-the-height-writable-patterns
trigger: "when writing the z position parameter from a panner input handler"
confidence: 0.9
domain: architecture
scope: project
date: 2026-09-04
---
# kNone and kFlat are the two elevation patterns a user may write height in

## Action

Gate any height write on the elevation pattern: `kTent`, `kArch`, `kDome` and
`kCurve` derive z from x/y in `ElevationListener::parameterChanged`
(`Elevation.h`) and overwrite a direct write before it is seen, so ignore the
input in those modes. `kNone` and `kFlat` leave z alone, which is also why
`PositionSelectionScreen::updateDialVisibility` keeps the height dial enabled in
exactly those two. Treat kNone and kFlat together, not Flat alone.

## Evidence

- PAN-02.2 (#60) named only Flat in its body; the operator decided on
  2026-09-04 to include kNone, against design.md O4 and the dial-enable rule.
- `PannerInput::elevationOwnsHeight` is the single predicate, pinned by
  `PannerInputTest.derivedElevationPatternsOwnHeight` and
  `flatAndNoPatternLeaveHeightToTheUser`.
- Relevant to PAN-02.3 (#61), whose speaker-click behavior is also per-pattern.
