---
id: top-view-front-is-a-positive-y-parameter
trigger: "when signing a front/back position change against the top-down panner's screen axis"
confidence: 0.9
domain: architecture
scope: project
date: 2026-09-04
---
# The top view's front of the room is a POSITIVE front/back parameter

## Action

Toward the front of the room (the top of the panner view) is front/back
parameter **+1**, not -1. Three places compose to decide it, so it is not
readable off the axis name: the front speakers sit at negative room-view NDC z
(`SpeakerLookup.h` -- Centre -1.0, L/R -0.866), `Coordinates::toRoomNdc` maps
front/back to NDC z with its sign inverted, and `getTopViewTransform` then puts
negative NDC z at the top of the view. Probe the composed chain
(`toRoomNdc` then `toWindow`) rather than reading any single coefficient -- see
[[probe-the-transform-dont-restate-its-coefficients]].

## Evidence

- PAN-02.2 (#60) shipped the up arrow as front/back +1. Its issue body asserted
  fb = -1 for the same behavior, which was the wrong sign for what it asked for.
- Probed: fb=+20 lands at window y 145.92 against 200.00 at the origin; the
  Centre speaker (the room's front) lands at 64.80. Smaller window y is higher
  on screen.
- Pinned by `PannerInputTest.upArrowMovesTheSourceTowardTheTopOfTheView` and
  `theTopOfTheViewIsTheFrontOfTheRoom`, asserted against the real transform.
