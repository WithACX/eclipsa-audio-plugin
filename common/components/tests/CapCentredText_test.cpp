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

#include "components/src/CapCentredText.h"
#include "components/src/EclipsaColours.h"
#include "components/src/EclipsaLookAndFeel.h"
#include "components/src/ImageTextButton.h"
#include "components/tests/InkMeasure.h"
// clang-format on

#include <gtest/gtest.h>

namespace {

const juce::Colour kBackground = EclipsaColours::backgroundOffBlack;

juce::Image blankImage(const int width, const int height) {
  juce::Image image(juce::Image::ARGB, width, height, true);
  juce::Graphics(image).fillAll(kBackground);
  return image;
}

}  // namespace

// Verifies a CapCentredLabel centres its capitals vertically in its bounds.
TEST(CapCentredLabel, centres_capitals_in_bounds) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  CapCentredLabel label("Title", "HEH");
  label.setFont(EclipsaLookAndFeel::roboto(22.f));
  label.setColour(juce::Label::textColourId, juce::Colours::white);
  label.setBounds(0, 0, 200, 41);

  juce::Image image = blankImage(200, 41);
  {
    juce::Graphics g(image);
    label.paint(g);
  }

  const auto capsCentre = InkMeasure::inkCentreY(
      image, 0, 200, 0, 41, kBackground, juce::Colours::white);
  ASSERT_TRUE(capsCentre.has_value());
  EXPECT_NEAR(*capsCentre, 20.5f, 0.75f);
}

// Verifies the Edit and Reroute buttons centre their icon and text capitals
// on the same line.
TEST(BlueTextBlackButtonLookAndFeel, icon_and_text_share_centre_line) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  juce::Image icon(juce::Image::ARGB, 18, 18, true);
  icon.clear(icon.getBounds(), juce::Colours::white);
  BlueTextBlackButtonLookAndFeel lookAndFeel(icon);
  juce::TextButton button("EDIT");
  button.setLookAndFeel(&lookAndFeel);
  button.setSize(85, 40);

  juce::Image image = blankImage(85, 40);
  {
    juce::Graphics g(image);
    lookAndFeel.drawButtonText(g, button, false, false);
  }
  button.setLookAndFeel(nullptr);

  const juce::Colour textColour = EclipsaColours::selectCyan;
  const auto iconLeft = InkMeasure::firstInkColumn(image, 0, 40, kBackground,
                                                   juce::Colours::white);
  ASSERT_TRUE(iconLeft.has_value());
  const auto iconCentre =
      InkMeasure::inkCentreY(image, *iconLeft, *iconLeft + 10, 0, 40,
                             kBackground, juce::Colours::white);
  const auto capsCentre = InkMeasure::inkCentreY(image, *iconLeft + 22, 85, 0,
                                                 40, kBackground, textColour);
  ASSERT_TRUE(iconCentre.has_value());
  ASSERT_TRUE(capsCentre.has_value());
  EXPECT_NEAR(*capsCentre, *iconCentre, 0.75f);
  EXPECT_NEAR(*iconCentre, 20.f, 0.75f);
}
