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
#include <array>

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
