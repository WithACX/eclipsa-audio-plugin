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

#include <components/components.h>
#include <gtest/gtest.h>

namespace {

constexpr int kMargin = DisplayFittedEditorSize::kDisplayEdgeMargin;

// A 1680x1050 pt display less a 33 pt macOS menu bar.
const juce::Rectangle<int> kBuiltInUserArea{0, 33, 1680, 1017};

}  // namespace

// Verifies the preferred size is kept when it fits the display.
TEST(DisplayFittedEditorSize, initial_sizing_keeps_preferred_size_that_fits) {
  const auto sizing =
      DisplayFittedEditorSize::initialSizing({1600, 752}, kBuiltInUserArea);
  EXPECT_EQ(sizing.width, 1600);
  EXPECT_EQ(sizing.height, 752);
  EXPECT_EQ(sizing.minWidth, 800);
  EXPECT_EQ(sizing.minHeight, 356);
  EXPECT_EQ(sizing.maxWidth, 1680);
  EXPECT_EQ(sizing.maxHeight, 1016);
}

// Verifies a preferred size larger than the display is capped to an even size
// inside it.
TEST(DisplayFittedEditorSize, initial_sizing_caps_to_display_with_even_sizes) {
  const auto sizing = DisplayFittedEditorSize::initialSizing(
      {1600, 752}, juce::Rectangle<int>{0, 25, 1279, 699});
  EXPECT_EQ(sizing.width, 1278);
  EXPECT_EQ(sizing.height, 698);
  EXPECT_LE(sizing.width, 1279);
  EXPECT_LE(sizing.height, 699);
}

// Verifies an editor already inside the display keeps its size.
TEST(DisplayFittedEditorSize, fit_keeps_editor_inside_display) {
  const auto size = DisplayFittedEditorSize::sizeToFitArea(
      {20, 100, 1552, 724}, kBuiltInUserArea, {800, 356});
  EXPECT_EQ(size, juce::Point<int>(1552, 724));
}

// Verifies an editor pushed right by a host panel (REAPER's FX-chain list) is
// narrowed so its right edge stays inside the display.
TEST(DisplayFittedEditorSize, fit_narrows_editor_pushed_past_right_edge) {
  const auto size = DisplayFittedEditorSize::sizeToFitArea(
      {239, 216, 1552, 724}, kBuiltInUserArea, {800, 356});
  EXPECT_EQ(size, juce::Point<int>(1680 - kMargin - 239 - 1, 724));
  EXPECT_LE(239 + size.x, kBuiltInUserArea.getRight() - kMargin);
}

// Verifies the fit works on a secondary display to the right of the primary.
TEST(DisplayFittedEditorSize, fit_uses_secondary_display_coordinates) {
  const juce::Rectangle<int> external{1680, 0, 1920, 1055};
  const auto size = DisplayFittedEditorSize::sizeToFitArea(
      {1865 + 239, 89, 1600, 752}, external, {800, 356});
  EXPECT_EQ(size.x, 3600 - kMargin - (1865 + 239));
  EXPECT_EQ(size.y, 752);
}

// Verifies an editor running past the bottom edge is shortened to an even
// height.
TEST(DisplayFittedEditorSize, fit_shortens_editor_past_bottom_edge) {
  const auto size = DisplayFittedEditorSize::sizeToFitArea(
      {0, 400, 1552, 724}, kBuiltInUserArea, {800, 356});
  EXPECT_EQ(size.y, 1050 - kMargin - 400);
  EXPECT_EQ(size.y % 2, 0);
}

// Verifies the fit never shrinks below the resize minimum.
TEST(DisplayFittedEditorSize, fit_stops_at_minimum_size) {
  const auto size = DisplayFittedEditorSize::sizeToFitArea(
      {1400, 900, 1552, 724}, kBuiltInUserArea, {800, 356});
  EXPECT_EQ(size, juce::Point<int>(800, 356));
}
