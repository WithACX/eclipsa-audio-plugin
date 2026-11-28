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

// clang-format off
#include <components/components.h>

#include "components/src/EclipsaColours.h"
#include "components/src/EclipsaLookAndFeel.h"
#include "components/src/OutlineTitle.h"
#include "components/src/SelectionBox.h"
#include "components/src/TitledTextBox.h"
// clang-format on

#include <gtest/gtest.h>

#include <optional>
#include <utility>

namespace {

constexpr int kWidth = 200;
constexpr int kHeight = 60;
// All capitals, so the title's ink spans exactly the cap height.
const juce::String kTitle = "HEH";

// How far a pixel is from the background toward the title/outline grey.
float inkFraction(const juce::Colour pixel) {
  const float background =
      EclipsaColours::backgroundOffBlack.getPerceivedBrightness();
  const float ink = EclipsaColours::tabTextGrey.getPerceivedBrightness();
  return (pixel.getPerceivedBrightness() - background) / (ink - background);
}

// First and last rows of the column range [x0, x1), above the box's bottom
// edge, holding a pixel at least half ink.
std::optional<std::pair<int, int>> inkRows(const juce::Image& image,
                                           const int x0, const int x1) {
  std::optional<std::pair<int, int>> rows;
  for (int y = 0; y < image.getHeight() - 10; ++y) {
    for (int x = x0; x < x1; ++x) {
      if (inkFraction(image.getPixelAt(x, y)) >= 0.5f) {
        rows = {rows ? rows->first : y, y};
        break;
      }
    }
  }
  return rows;
}

// Checks the title's capitals are vertically centred on the top outline.
void expectTitleCentredOnOutline(const juce::Image& image, const int titleX) {
  const auto outline = inkRows(image, kWidth - 50, kWidth - 49);
  const auto title = inkRows(image, titleX, titleX + 25);
  ASSERT_TRUE(outline.has_value());
  ASSERT_TRUE(title.has_value());

  const float outlineCentre = outline->first + 0.5f;
  const float titleCentre = (title->first + title->second + 1) / 2.f;
  EXPECT_NEAR(titleCentre, outlineCentre, 0.75f);
}

juce::Image blankImage() {
  juce::Image image(juce::Image::ARGB, kWidth, kHeight, true);
  juce::Graphics(image).fillAll(EclipsaColours::backgroundOffBlack);
  return image;
}

}  // namespace

// Verifies a text box's title (e.g. "Save audio to") straddles its outline.
TEST(OutlineTitle, text_box_title_is_centred_on_outline) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  TitledTextBox box(kTitle);
  box.setSize(kWidth, kHeight);

  juce::Image image = blankImage();
  {
    juce::Graphics g(image);
    box.paintEntireComponent(g, true);
  }

  expectTitleCentredOnOutline(image, 15);
}

// Verifies a selection box's title (e.g. "Export Profile") straddles its
// outline.
TEST(OutlineTitle, selection_box_title_is_centred_on_outline) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  SelectionBoxLookAndFeel lookAndFeel(kTitle);
  juce::ComboBox comboBox;
  comboBox.setLookAndFeel(&lookAndFeel);

  juce::Image image = blankImage();
  {
    juce::Graphics g(image);
    lookAndFeel.drawComboBox(g, kWidth, kHeight, false, 0, 0, 0, 0, comboBox);
  }
  comboBox.setLookAndFeel(nullptr);

  expectTitleCentredOnOutline(image, 12);
}

// Verifies titles draw at the 12 pt Roboto Figma specifies.
TEST(OutlineTitle, font_is_12_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;

  const juce::Font font = OutlineTitle::font();

  EXPECT_EQ(font.getTypefaceName(), EclipsaLookAndFeel::kRoboto);
  EXPECT_NEAR(font.getHeightInPoints(), 12.0f, 0.01f);
}
