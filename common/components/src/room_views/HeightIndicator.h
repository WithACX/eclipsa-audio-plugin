/*
 * Copyright 2025 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <algorithm>
#include <array>
#include <vector>

#include "Coordinates.h"

// Geometry for the audio element panner's height indicator: the room's
// horizontal cross-section at the source's height, plus the two leader lines
// tying the source to it.
//
// The vertical screen axis of the top-down panner carries front/back, so height
// has no axis of its own. This indicator is what makes it readable: the
// cross-section sits on the room's sides at the current height, and because
// getTopViewTransform() is a perspective projection whose w is 5 - height, the
// projected outline expands toward the walls as the source rises and contracts
// as it falls. That behavior is produced entirely by transforming these
// anchors; nothing here scales anything by hand.
//
// Like ElevationSurfaces.h, this lives in a header of its own rather than in an
// anonymous namespace inside PerspectiveRoomViews.cpp: a test can only reach it
// from here (HeightIndicator_test.cpp), and PerspectiveRoomViews.cpp is
// unity-built into components.cpp with twenty other sources, where an anonymous
// namespace is shared across the whole translation unit and a bare helper name
// could collide with one another source adds later.
namespace HeightIndicator {

// A straight edge, held as its two 3D anchors so the projection is applied to
// the endpoints rather than to an already-flattened screen line.
struct Segment {
  Coordinates::Point4D start;
  Coordinates::Point4D end;
};

/**
 * @brief The room's horizontal cross-section at one height, as four segments.
 *
 * The four anchors are the room bounds at that height -- (+/-1, height, +/-1)
 * in room-view NDC -- and the segments run around them in order, so drawing all
 * four puts a line along every side of the square. Outline only: the elevation
 * surfaces are the filled shapes, and this sits over them.
 *
 * @param height the source's height in room-view NDC (-1..1)
 * @return std::array<Segment, 4> the four sides, in draw order
 */
inline std::array<Segment, 4> crossSectionOutline(const float height) {
  const Coordinates::Point4D kFrontLeft = {-1.f, height, -1.f, 1.f};
  const Coordinates::Point4D kFrontRight = {1.f, height, -1.f, 1.f};
  const Coordinates::Point4D kBackRight = {1.f, height, 1.f, 1.f};
  const Coordinates::Point4D kBackLeft = {-1.f, height, 1.f, 1.f};

  return {Segment{kFrontLeft, kFrontRight}, Segment{kFrontRight, kBackRight},
          Segment{kBackRight, kBackLeft}, Segment{kBackLeft, kFrontLeft}};
}

// Runs of the indicator, split by which side of the elevation surface each one
// lies on. A painter's algorithm has no depth buffer, so the two lists ARE the
// draw order: `below` goes down before the surface is filled and is tinted by
// it, `above` goes down after and stays full strength.
//
// Named for the outline it was written for, and it now carries the connectors
// too -- both are split by the same walk, so one type keeps the caller from
// having to pair a different result shape with each.
struct SplitOutline {
  std::vector<Segment> below;
  std::vector<Segment> above;
};

/**
 * @brief A point partway along a segment, at parameter t in [0, 1].
 *
 * Height is constant along every outline side and along both connectors, so it
 * survives the interpolation unchanged; only the left/right and front/back
 * coordinates move. The interpolation covers all three components anyway, so
 * the split does not depend on that remaining true.
 *
 * @param segment the segment to walk
 * @param t the parameter, 0 at start and 1 at end
 * @return Coordinates::Point4D the interpolated anchor, w = 1
 */
inline Coordinates::Point4D pointAlong(const Segment& segment, const float t) {
  const auto kLerp = [t](const float from, const float to) {
    return from + (to - from) * t;
  };
  return {kLerp(segment.start.a[0], segment.end.a[0]),
          kLerp(segment.start.a[1], segment.end.a[1]),
          kLerp(segment.start.a[2], segment.end.a[2]), 1.f};
}

/**
 * @brief Split one segment where it crosses an elevation surface, appending its
 * runs to a split.
 *
 * The single walk behind every part of the indicator. The outline's sides and
 * the two connectors are all straight runs at the source's height, so they
 * cross a surface by the same rule and must be split by the same code: two
 * walks would be two places for the crossing rule to drift, and the outline and
 * a connector meeting at a point would eventually disagree about which side of
 * the roof that point is on.
 *
 * The three curved patterns are ruled surfaces sampled along the room's left
 * and right edges, so their boundary curves lie in the planes x = -1 and x = +1
 * -- the very planes the outline's left and right sides lie in. The crossing is
 * therefore a genuine incidence between two curves sharing a plane, and a
 * projective transform preserves incidence: the split point lands exactly where
 * the drawn surface edge meets the drawn geometry, at any window size. Nothing
 * here approximates a depth test.
 *
 * Sampling rather than solving each pattern in closed form is deliberate. It
 * matches the surface actually DRAWN, which is faceted at the painter's own
 * sample count, and it stays correct for a curve that crosses once (the
 * logarithmic pattern is monotonic) as readily as for one that crosses twice.
 *
 * @param segment the run to split
 * @param roofHeightAt the elevation surface's height at one front/back
 * position; return the floor (-1) for a pattern with no surface to pass under
 * @param samples positions tested along the segment, at least 2
 * @param into the split to append to
 */
template <typename RoofHeightFn>
inline void splitSegmentInto(const Segment& segment,
                             RoofHeightFn&& roofHeightAt, const int samples,
                             SplitOutline& into) {
  const int kSamples = std::max(2, samples);

  // A point is ABOVE when it is at or over the surface. Height is read from the
  // interpolated point rather than taken as a parameter, so a segment whose
  // ends sit at different heights would still be classified correctly -- none
  // does today, and the walk does not depend on that staying true.
  //
  // Ties resolve to above so a source resting exactly on the surface -- which
  // is where ElevationListener puts it for every pattern that clamps -- draws
  // over it rather than flickering between the two lists.
  const auto kIsAbove = [&](const float t) {
    const Coordinates::Point4D kAt = pointAlong(segment, t);
    return kAt.a[1] >= roofHeightAt(kAt.a[2]);
  };
  const auto kEmit = [&](const float from, const float to, const bool above) {
    // A crossing landing on an end would otherwise emit an empty run. The
    // connector starting ON the surface is exactly that case.
    if (to - from <= 1e-6f) {
      return;
    }
    (above ? into.above : into.below)
        .push_back(Segment{pointAlong(segment, from), pointAlong(segment, to)});
  };

  float runStart = 0.f;
  bool runAbove = kIsAbove(0.f);
  for (int i = 1; i < kSamples; ++i) {
    const float kPrev = static_cast<float>(i - 1) / (kSamples - 1);
    const float kHere = static_cast<float>(i) / (kSamples - 1);
    if (kIsAbove(kHere) == runAbove) {
      continue;
    }
    // Bisect the bracketing interval rather than interpolating the height
    // difference: the tent's ridge is a crease, so that difference is not
    // linear across a sample step containing it.
    float lo = kPrev, hi = kHere;
    for (int step = 0; step < 20; ++step) {
      const float kMid = 0.5f * (lo + hi);
      if (kIsAbove(kMid) == runAbove) {
        lo = kMid;
      } else {
        hi = kMid;
      }
    }
    const float kCrossing = 0.5f * (lo + hi);
    kEmit(runStart, kCrossing, runAbove);
    runStart = kCrossing;
    runAbove = !runAbove;
  }
  kEmit(runStart, 1.f, runAbove);
}

/**
 * @brief Split the cross-section outline where it crosses an elevation surface.
 *
 * The front and back sides go through the same walk as the other two rather
 * than being special-cased. They sit at front/back = -/+1 where all three
 * patterns are at or near the floor, so in practice they come back whole in
 * `above` -- but that is an outcome of the geometry, not an assumption baked
 * into the split.
 *
 * @param height the source's height in room-view NDC (-1..1)
 * @param roofHeightAt the elevation surface's height at one front/back position
 * @param samplesPerSide positions tested along each side, at least 2
 * @return SplitOutline the runs under the surface and the runs over it
 */
template <typename RoofHeightFn>
inline SplitOutline splitAtElevation(const float height,
                                     RoofHeightFn&& roofHeightAt,
                                     const int samplesPerSide = 41) {
  SplitOutline split;
  for (const Segment& side : crossSectionOutline(height)) {
    splitSegmentInto(side, roofHeightAt, samplesPerSide, split);
  }

  return split;
}

/**
 * @brief The two leader lines from a source to its cross-section outline.
 *
 * One runs to the outline's back edge (NDC z = +1, which the top-down transform
 * renders at the bottom of the view) at the source's own left/right position;
 * the other runs to the outline's right edge (NDC x = +1) at the source's own
 * front/back position. Both endpoints are built in 3D at the source's height
 * and share its height exactly, so they stay attached to the outline under the
 * perspective divide and on a window resize -- they are not screen-axis-aligned
 * lines drawn to a pixel constant.
 *
 * @param source the source position in room-view NDC, w = 1
 * @return std::array<Segment, 2> the back-edge connector, then the right-edge
 * connector
 */
inline std::array<Segment, 2> leaderLines(const Coordinates::Point4D& source) {
  const float kHeight = source.a[1];
  const Coordinates::Point4D kToBackEdge = {source.a[0], kHeight, 1.f, 1.f};
  const Coordinates::Point4D kToRightEdge = {1.f, kHeight, source.a[2], 1.f};

  return {Segment{source, kToBackEdge}, Segment{source, kToRightEdge}};
}

/**
 * @brief Split the connectors against an elevation surface.
 *
 * The two are NOT treated alike, because they do not meet the surface alike.
 *
 * The BACK-EDGE connector varies in front/back, so the surface height beneath
 * it changes along its length and it genuinely passes under a roof -- from a
 * source on the near slope it runs over the crest. It goes through the same
 * walk the outline does, for the same reason.
 *
 * The RIGHT-EDGE connector holds the source's front/back position along its
 * whole length. Every pattern that clamps the source is a height field in
 * front/back ALONE, so the surface beneath that line is at one height the whole
 * way -- the source's own. The line is therefore COINCIDENT with the surface
 * rather than above or below it, and it is drawn on top unconditionally.
 *
 * Classifying it by comparison instead is what made it flicker. A coincident
 * line resolves on the tie, and the source's height reaches this code from
 * integer position parameters scaled by 1/50, so quantisation nudges it a
 * fraction either side of the surface it is nominally resting on: the same line
 * came back above at one position and below at the next, tinting and untinting
 * as the source moved. No epsilon fixes that honestly -- the comparison is
 * being asked a question the geometry does not pose.
 *
 * @param source the source position in room-view NDC, w = 1
 * @param roofHeightAt the elevation surface's height at one front/back position
 * @param samplesPerLine positions tested along the back-edge connector, min 2
 * @return SplitOutline the runs under the surface and the runs over it
 */
template <typename RoofHeightFn>
inline SplitOutline splitLeaderLinesAtElevation(
    const Coordinates::Point4D& source, RoofHeightFn&& roofHeightAt,
    const int samplesPerLine = 41) {
  SplitOutline split;
  const std::array<Segment, 2> kLeaders = leaderLines(source);

  splitSegmentInto(kLeaders[0], roofHeightAt, samplesPerLine, split);
  split.above.push_back(kLeaders[1]);

  return split;
}

/**
 * @brief Project one segment's anchors into window coordinates.
 *
 * @param transformMat the view's projection
 * @param window the current window bounds
 * @param segment the segment to project
 * @return std::array<Coordinates::Point2D, 2> its start and end on screen
 */
inline std::array<Coordinates::Point2D, 2> projectSegment(
    const Coordinates::Mat4& transformMat,
    const Coordinates::WindowData& window, const Segment& segment) {
  return {Coordinates::toWindow(transformMat, window, segment.start),
          Coordinates::toWindow(transformMat, window, segment.end)};
}

}  // namespace HeightIndicator
