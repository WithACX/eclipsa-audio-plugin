---
id: probe-the-transform-dont-restate-its-coefficients
trigger: "when writing an inverse for a projection whose matrix is declared elsewhere"
confidence: 0.85
domain: architecture
scope: project
date: 2026-09-03
---

# Probe the Transform, Do Not Restate Its Coefficients

## Action
When an inverse projection needs the forward transform's scale factors, recover
them by evaluating the forward operation at known probe points instead of
copying the matrix literals into the inverse. The two directions then read the
same source and cannot drift apart when the matrix changes.

## Pattern
For a transform that separates its axes, at a fixed value of the axis the
perspective divisor depends on:

1. Evaluate the forward product at the origin, at unit on axis A, and at unit
   on axis B, all at the fixed height.
2. Read the divisor off the origin probe's w component.
3. Read each axis scale off the difference between its unit probe and the origin
   probe.
4. Invert with those three numbers.

Document the structural assumption this relies on (which input axis each output
component depends on) and name the function for the specific transform it
inverts, so nobody mistakes it for a general matrix inverse.

## Evidence
PAN-02.1 (#59). The issue body supplied the literals directly -- `ndc_x = x *
2.19693 / (5 - y)` and `ndc_y = -fb * 3.3799 / (5 - y)` -- copied from
`getTopViewTransform()`. Probing instead means editing the matrix cannot silently
break the drag path, and it made the implementation robust to an open question
about `boost::qvm`'s multiplication convention: the probes go through the same
`point * transformMat` expression `toWindow` uses, so whatever that means, both
directions mean it identically.

Cost: the approach is only valid while the transform keeps its axis separation.
That is stated in the doc comment rather than left implicit, and the name
(`fromTopViewWindow`, not `fromWindow`) carries the limit.
