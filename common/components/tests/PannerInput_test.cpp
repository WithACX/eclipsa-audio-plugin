// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// The panner's keyboard and scroll-wheel input mapping.
//
// The front/back nudge sign is the reason this file exists. It is signed
// against the SCREEN, and the chain that decides it runs through three places
// (the front speakers' room-view NDC z, toRoomNdc's negation of front/back,
// and getTopViewTransform's own sign), so it is not readable off the axis
// name. windowX and windowY below assert it against the real transform rather
// than against a restatement of any one of those coefficients -- get it wrong
// and the up arrow drives the source toward the back of the room, which is
// silent in the plugin and only shows up in a DAW.

// Pull in the umbrella header first: several components/src headers include
// components.h themselves and only resolve correctly once the umbrella has
// fully loaded once. See the same note in Coordinates_test.cpp.
// clang-format off
#include <components/components.h>

#include "components/src/room_views/Coordinates.h"
#include "components/src/room_views/PannerInput.h"
// clang-format on

#include <gtest/gtest.h>

namespace {

using Elevation = AudioElementSpatialLayout::Elevation;

// Matches the shape paint() builds from the component bounds.
const Coordinates::WindowData kWindow = {.leftCornerX = 0.f,
                                         .bottomCornerY = 400.f,
                                         .width = 400.f,
                                         .height = 400.f};

// The window y a position parameter triple projects to. Window y grows
// downward, so a SMALLER value is higher on screen.
float windowY(const int leftRight, const int frontBack, const int height) {
  return Coordinates::toWindow(
             Coordinates::getTopViewTransform(), kWindow,
             Coordinates::toRoomNdc((float)leftRight, (float)frontBack,
                                    (float)height))
      .a[1];
}

// The window x a position parameter triple projects to.
float windowX(const int leftRight, const int frontBack, const int height) {
  return Coordinates::toWindow(
             Coordinates::getTopViewTransform(), kWindow,
             Coordinates::toRoomNdc((float)leftRight, (float)frontBack,
                                    (float)height))
      .a[0];
}

}  // namespace

// The left and right arrows move the source one step along left/right.
TEST(PannerInputTest, arrowKeysNudgeLeftRightOneStep) {
  const PannerInput::Nudge kLeft =
      PannerInput::nudgeForKeyCode(juce::KeyPress::leftKey);
  const PannerInput::Nudge kRight =
      PannerInput::nudgeForKeyCode(juce::KeyPress::rightKey);

  EXPECT_EQ(kLeft.axis, PannerInput::Axis::kLeftRight);
  EXPECT_EQ(kLeft.steps, -1);
  EXPECT_EQ(kRight.axis, PannerInput::Axis::kLeftRight);
  EXPECT_EQ(kRight.steps, 1);
}

// The up and down arrows move the source one step along front/back.
TEST(PannerInputTest, arrowKeysNudgeFrontBackOneStep) {
  const PannerInput::Nudge kUp =
      PannerInput::nudgeForKeyCode(juce::KeyPress::upKey);
  const PannerInput::Nudge kDown =
      PannerInput::nudgeForKeyCode(juce::KeyPress::downKey);

  EXPECT_EQ(kUp.axis, PannerInput::Axis::kFrontBack);
  EXPECT_EQ(kDown.axis, PannerInput::Axis::kFrontBack);
  EXPECT_EQ(kUp.steps, -kDown.steps);
}

// The up arrow moves the source toward the top of the view and the down arrow
// toward the bottom, under the real top-view transform.
TEST(PannerInputTest, upArrowMovesTheSourceTowardTheTopOfTheView) {
  const int kUpSteps =
      PannerInput::nudgeForKeyCode(juce::KeyPress::upKey).steps;
  const int kDownSteps =
      PannerInput::nudgeForKeyCode(juce::KeyPress::downKey).steps;

  EXPECT_LT(windowY(0, kUpSteps, 0), windowY(0, 0, 0));
  EXPECT_GT(windowY(0, kDownSteps, 0), windowY(0, 0, 0));
}

// The top of the view is the front of the room, so the up arrow agrees with
// where the front speakers are drawn.
TEST(PannerInputTest, theTopOfTheViewIsTheFrontOfTheRoom) {
  // The centre speaker is the room's front, at room-view NDC z -1.
  const float kCentreSpeakerWindowY =
      Coordinates::toWindow(Coordinates::getTopViewTransform(), kWindow,
                            Coordinates::Point4D{0.f, 0.f, -1.f, 1.f})
          .a[1];
  const int kUpSteps =
      PannerInput::nudgeForKeyCode(juce::KeyPress::upKey).steps;

  EXPECT_LT(kCentreSpeakerWindowY, windowY(0, 0, 0));
  EXPECT_LT(windowY(0, kUpSteps, 0), windowY(0, 0, 0));
}

// The right arrow moves the source toward the right of the view.
TEST(PannerInputTest, rightArrowMovesTheSourceTowardTheRightOfTheView) {
  const int kRightSteps =
      PannerInput::nudgeForKeyCode(juce::KeyPress::rightKey).steps;

  EXPECT_GT(windowX(kRightSteps, 0, 0), windowX(0, 0, 0));
}

// A key that is not an arrow is not a panner gesture and is left alone.
TEST(PannerInputTest, otherKeysAreNotNudges) {
  for (const int kKeyCode :
       {(int)juce::KeyPress::spaceKey, (int)juce::KeyPress::returnKey,
        (int)juce::KeyPress::tabKey, (int)'a'}) {
    const PannerInput::Nudge kNudge = PannerInput::nudgeForKeyCode(kKeyCode);
    EXPECT_EQ(kNudge.axis, PannerInput::Axis::kNone);
    EXPECT_EQ(kNudge.steps, 0);
  }
}

// A wheel notch emits exactly one height step.
TEST(PannerInputTest, oneWheelNotchIsOneStep) {
  float accumulator = 0.f;

  EXPECT_EQ(PannerInput::stepsForWheelDelta(0.1f, accumulator), 1);
  EXPECT_EQ(PannerInput::stepsForWheelDelta(-0.1f, accumulator), -1);
}

// Successive notches each emit one step rather than banking travel into a
// later burst.
TEST(PannerInputTest, successiveNotchesEachEmitOneStep) {
  float accumulator = 0.f;

  for (int notch = 0; notch < 5; ++notch) {
    EXPECT_EQ(PannerInput::stepsForWheelDelta(0.15f, accumulator), 1);
  }
}

// A trackpad's sub-notch deltas accumulate to one step instead of being
// discarded.
TEST(PannerInputTest, subNotchDeltasAccumulateToOneStep) {
  float accumulator = 0.f;
  const float kSmall = PannerInput::kWheelNotchDelta / 4.f;

  EXPECT_EQ(PannerInput::stepsForWheelDelta(kSmall, accumulator), 0);
  EXPECT_EQ(PannerInput::stepsForWheelDelta(kSmall, accumulator), 0);
  EXPECT_EQ(PannerInput::stepsForWheelDelta(kSmall, accumulator), 0);
  EXPECT_EQ(PannerInput::stepsForWheelDelta(kSmall, accumulator), 1);
}

// Reversing direction below a notch cancels rather than emitting a step.
TEST(PannerInputTest, reversedSubNotchTravelCancels) {
  float accumulator = 0.f;
  const float kSmall = PannerInput::kWheelNotchDelta / 2.f;

  EXPECT_EQ(PannerInput::stepsForWheelDelta(kSmall, accumulator), 0);
  EXPECT_EQ(PannerInput::stepsForWheelDelta(-kSmall, accumulator), 0);
  EXPECT_FLOAT_EQ(accumulator, 0.f);
}

// A step resets the accumulator, so no remainder carries into the next event.
TEST(PannerInputTest, emittingAStepResetsTheAccumulator) {
  float accumulator = 0.f;

  EXPECT_EQ(PannerInput::stepsForWheelDelta(1.f, accumulator), 1);
  EXPECT_FLOAT_EQ(accumulator, 0.f);
}

// The four derived elevation patterns own height, so the wheel stays out of
// them.
TEST(PannerInputTest, derivedElevationPatternsOwnHeight) {
  EXPECT_TRUE(PannerInput::elevationOwnsHeight(Elevation::kTent));
  EXPECT_TRUE(PannerInput::elevationOwnsHeight(Elevation::kArch));
  EXPECT_TRUE(PannerInput::elevationOwnsHeight(Elevation::kDome));
  EXPECT_TRUE(PannerInput::elevationOwnsHeight(Elevation::kCurve));
}

// kNone and kFlat leave height to the user, the same two patterns for which
// PositionSelectionScreen keeps the height dial enabled.
TEST(PannerInputTest, flatAndNoPatternLeaveHeightToTheUser) {
  EXPECT_FALSE(PannerInput::elevationOwnsHeight(Elevation::kNone));
  EXPECT_FALSE(PannerInput::elevationOwnsHeight(Elevation::kFlat));
}
