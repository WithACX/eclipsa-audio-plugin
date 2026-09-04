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
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>

#include "data_structures/src/AudioElementSpatialLayout.h"

// Keyboard and scroll-wheel input mapping for the audio element panner.
//
// Held apart from AudioElementPluginTopView so it can be reached by tests:
// there is no JUCE UI test harness in this repo, so logic left inside the
// component is verifiable only by hand in a DAW.
//
// Placed in a header so it can be reached by tests.
namespace PannerInput {

// Which position parameter an input moves. kNone means the input is not a
// panner gesture and belongs to whatever else wants it.
enum class Axis { kNone, kLeftRight, kFrontBack, kHeight };

// One discrete input: the axis it moves and the signed number of parameter
// steps it moves it by.
struct Nudge {
  Axis axis = Axis::kNone;
  int steps = 0;
};

// A wheel notch arrives as one event carrying a fraction of a full scroll; a
// trackpad sends a stream of smaller values instead. Accumulating to this
// threshold gives both devices one step per notch-worth of travel.
constexpr float kWheelNotchDelta = 0.05f;

/**
 * @brief Map an arrow key to the position step it nudges.
 *
 * Front/back is signed against the SCREEN, not the parameter name. The top
 * view puts the front of the room at the top (the front speakers sit at
 * negative room-view NDC z) and Coordinates::toRoomNdc negates front/back on
 * the way in, so the front of the room is a POSITIVE front/back parameter and
 * the up arrow adds a step. Verify that against getTopViewTransform rather
 * than reading it off the axis name.
 *
 * @param keyCode a juce::KeyPress key code
 * @return Nudge the axis and signed steps, or Axis::kNone for any other key
 */
// Not a switch: JUCE's key codes are static const ints rather than constant
// expressions, so they cannot be case labels.
inline Nudge nudgeForKeyCode(const int keyCode) {
  if (keyCode == juce::KeyPress::leftKey) {
    return {Axis::kLeftRight, -1};
  }
  if (keyCode == juce::KeyPress::rightKey) {
    return {Axis::kLeftRight, 1};
  }
  if (keyCode == juce::KeyPress::upKey) {
    return {Axis::kFrontBack, 1};
  }
  if (keyCode == juce::KeyPress::downKey) {
    return {Axis::kFrontBack, -1};
  }
  return {};
}

/**
 * @brief Convert one wheel event into whole height steps.
 *
 * Carries the sub-notch remainder in the accumulator so a trackpad's stream of
 * small deltas adds up instead of being discarded, and resets on every step so
 * a long scroll cannot bank travel and fire a burst later.
 *
 * @param delta the event's deltaY
 * @param accumulator carried between events; reset when a step is emitted
 * @return int the signed step count, 0 while below a notch
 */
inline int stepsForWheelDelta(const float delta, float& accumulator) {
  accumulator += delta;
  if (std::abs(accumulator) < kWheelNotchDelta) {
    return 0;
  }
  const int kSteps = accumulator > 0.f ? 1 : -1;
  accumulator = 0.f;
  return kSteps;
}

/**
 * @brief Whether the elevation pattern derives height from position.
 *
 * ElevationListener recomputes z from x/y under Tent, Arch, Dome and Curve and
 * writes it back, so a wheel write in those modes is overwritten before it is
 * seen. It leaves z alone under kNone and kFlat -- the same two patterns for
 * which PositionSelectionScreen keeps the height dial enabled.
 *
 * @param elevation the active elevation pattern
 * @return bool true when the pattern owns height
 */
inline bool elevationOwnsHeight(
    const AudioElementSpatialLayout::Elevation elevation) {
  switch (elevation) {
    case AudioElementSpatialLayout::Elevation::kTent:
    case AudioElementSpatialLayout::Elevation::kArch:
    case AudioElementSpatialLayout::Elevation::kDome:
    case AudioElementSpatialLayout::Elevation::kCurve:
      return true;
    default:
      return false;
  }
}

}  // namespace PannerInput
