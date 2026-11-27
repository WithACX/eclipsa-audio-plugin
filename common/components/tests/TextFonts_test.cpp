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
#include "components/src/SelectionBox.h"
#include "components/src/TitledTextBox.h"
#include "components/src/WarningBannerBase.h"
#include "components/src/loudness_meter/LoudnessStats.h"
#include "components/tests/InkMeasure.h"
// clang-format on

#include <gtest/gtest.h>

namespace {

const juce::Colour kBackground = EclipsaColours::backgroundOffBlack;
constexpr int kWidth = 200;
constexpr int kHeight = 60;
// Outlines start 20 px down, so the inside of the box centres on y = 40.
constexpr float kBoxCentreY = 40.f;

void expectPoints(const juce::Font& font, const float points) {
  EXPECT_EQ(font.getTypefaceName(), EclipsaLookAndFeel::kRoboto);
  EXPECT_NEAR(font.getHeightInPoints(), points, 0.01f);
}

juce::Image blankImage() {
  juce::Image image(juce::Image::ARGB, kWidth, kHeight, true);
  juce::Graphics(image).fillAll(kBackground);
  return image;
}

}  // namespace

// Verifies text typed into a text box is 14 pt Roboto.
TEST(TextFonts, text_box_text_is_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  TitledTextBox box("Title");

  expectPoints(box.getTextEditor()->getFont(), 14.f);
}

// Verifies a text box centres its text's capitals inside its outline.
TEST(TextFonts, text_box_text_is_centred_in_outline) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  TitledTextBox box("");
  box.setText("HEH");
  box.setSize(kWidth, kHeight);

  juce::Image image = blankImage();
  {
    juce::Graphics g(image);
    box.paintEntireComponent(g, true);
  }

  const auto capsCentre =
      InkMeasure::inkCentreY(image, 15, 100, 22, kHeight - 2, kBackground,
                             EclipsaColours::headingGrey);
  ASSERT_TRUE(capsCentre.has_value());
  EXPECT_NEAR(*capsCentre, kBoxCentreY, 1.f);
}

// Verifies a dropdown's selected value and its menu items are 14 pt Roboto.
TEST(TextFonts, dropdown_text_is_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  SelectionBoxLookAndFeel lookAndFeel("Title");
  juce::ComboBox comboBox;

  expectPoints(lookAndFeel.getComboBoxFont(comboBox), 14.f);
  expectPoints(lookAndFeel.getPopupMenuFont(), 14.f);
}

// Verifies a dropdown centres its selected value's capitals inside its
// outline.
TEST(TextFonts, dropdown_value_is_centred_in_outline) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  SelectionBoxLookAndFeel lookAndFeel("");
  juce::ComboBox comboBox;
  comboBox.setLookAndFeel(&lookAndFeel);
  comboBox.addItem("HEH", 1);
  comboBox.setSelectedId(1, juce::dontSendNotification);

  juce::Image image = blankImage();
  {
    juce::Graphics g(image);
    lookAndFeel.drawComboBox(g, kWidth, kHeight, false, 0, 0, 0, 0, comboBox);
  }
  comboBox.setLookAndFeel(nullptr);

  const auto capsCentre =
      InkMeasure::inkCentreY(image, 15, 100, 22, kHeight - 2, kBackground,
                             EclipsaColours::headingGrey);
  ASSERT_TRUE(capsCentre.has_value());
  EXPECT_NEAR(*capsCentre, kBoxCentreY, 1.f);
}

// Verifies the loudness readouts beside the Loudness Standard box are 14 pt
// Roboto.
TEST(TextFonts, loudness_readouts_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  SpeakerMonitorData monitorData;
  LoudnessStats stats(monitorData);

  int labels = 0;
  for (juce::Component* child : stats.getChildren()) {
    if (const auto* label = dynamic_cast<juce::Label*>(child)) {
      expectPoints(label->getFont(), 14.f);
      ++labels;
    }
  }
  EXPECT_EQ(labels, 12);
}

// Verifies the yellow warning banners' message is 14 pt Roboto.
TEST(TextFonts, warning_banner_text_is_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;

  expectPoints(WarningBannerBase::messageFont(), 14.f);
}

// Verifies a dropdown's value draws in the same colour as text typed into a
// text box.
TEST(TextFonts, dropdown_text_colour_matches_text_box) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  SelectionBoxLookAndFeel lookAndFeel("Title");
  TitledTextBox box("Title");

  EXPECT_EQ(lookAndFeel.findColour(juce::ComboBox::textColourId),
            box.getTextEditor()->findColour(juce::TextEditor::textColourId));
}
