---
id: verify-transform-math-before-long-build
trigger: "before relying on a C++ build to validate coordinate or matrix arithmetic"
confidence: 0.90
domain: testing
scope: project
date: 2026-09-03
---

# Verify Transform Math in Python Before the C++ Build

## Action
When adding or inverting coordinate/matrix math in this repo, reimplement the
arithmetic in a throwaway Python script and assert the property first. A full
CMake build here fetches and compiles protobuf, JUCE, boost, zeromq, libiamf and
libear before it reaches your file, so a sign error costs a whole build cycle to
discover.

## Pattern
1. Transcribe the transform matrix and the forward function verbatim into Python.
2. Reimplement the candidate inverse the same way the C++ will.
3. Assert the round-trip property over the full domain, not a sample.
4. Reproduce the C++ rounding exactly -- `std::lround` rounds half away from
   zero, Python's `round` is banker's rounding, so a naive port disagrees on
   .5 boundaries.
5. Only then write the C++ and let the unit test confirm it.

## Evidence
PAN-02.1 (#59) added `Coordinates::fromTopViewWindow`, the height-dependent
inverse of `toWindow`. The Python check ran 343 combinations (7 heights x 7 x 7
positions) with 0 failures and printed the perspective divisor as 6.0 at height
-50 and 4.0 at +50, which independently confirmed both `w = 5 - height` and that
`boost::qvm`'s `vec * mat` is the row-vector convention (`result_j = sum_i v_i *
m[i][j]`).

That last point was the real payoff: the inverse's correctness depends on which
multiplication convention qvm uses, and the two conventions give different
answers about which input axis the perspective divisor reads. Confirming it took
seconds in Python and would have taken a 40-minute build otherwise.
