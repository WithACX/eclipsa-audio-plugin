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

#include "components/src/AEStripComponent.h"
#include "components/src/EclipsaLookAndFeel.h"
#include "components/src/ImageTextButton.h"
#include "components/src/SegmentedToggleButton.h"
#include "components/src/loudness_meter/LoudnessMeter.h"
// clang-format on

#include <gtest/gtest.h>

namespace {

class TestPlaybackMSRepo : public MSPlaybackRepository {
 public:
  TestPlaybackMSRepo()
      : MSPlaybackRepository(juce::ValueTree{PlaybackMS::kTreeType}) {}
};

// Checks `lookAndFeel` draws button text in 14 pt Roboto, as Figma specifies.
void expectRoboto14(juce::LookAndFeel& lookAndFeel) {
  juce::TextButton button;
  const juce::Font font = lookAndFeel.getTextButtonFont(button, 40);

  EXPECT_EQ(font.getTypefaceName(), EclipsaLookAndFeel::kRoboto);
  EXPECT_NEAR(font.getHeightInPoints(), 14.0f, 0.01f);
#if JUCE_MAC
  // Roboto's line height is (1900 + 500) / 2048 of its point size.
  EXPECT_NEAR(font.getHeight(), 14.0f * 2400.0f / 2048.0f, 0.01f);
#endif
}

}  // namespace

// Verifies the main screen's Export button text is 14 pt Roboto.
TEST(ButtonFonts, export_button_is_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  ExportImageTextButtonLookAndFeel lookAndFeel{juce::Image(), juce::Image()};

  expectRoboto14(lookAndFeel);
}

// Verifies the Edit and Reroute buttons' text is 14 pt Roboto.
TEST(ButtonFonts, edit_and_reroute_buttons_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  BlueTextBlackButtonLookAndFeel lookAndFeel{juce::Image()};

  expectRoboto14(lookAndFeel);
}

// Verifies segmented selectors (speakers, view, export times) use 14 pt Roboto.
TEST(ButtonFonts, segmented_selectors_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  STBLookAndFeel lookAndFeel;

  expectRoboto14(lookAndFeel);
}

// Verifies the mix strip solo and mute buttons use 14 pt Roboto.
TEST(ButtonFonts, strip_solo_mute_buttons_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  AEStripLookandFeel lookAndFeel;

  expectRoboto14(lookAndFeel);
}

// Verifies the loudness meter solo and mute buttons use 14 pt Roboto.
TEST(ButtonFonts, meter_solo_mute_buttons_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  TestPlaybackMSRepo repo;
  MeterLookAndFeel lookAndFeel{repo};

  expectRoboto14(lookAndFeel);
}

// Verifies the grey and cyan image buttons (Add Mix Presentation, tags,
// Delete) keep the 12 pt they drew at before Roboto was bundled.
TEST(ButtonFonts, grey_and_cyan_buttons_are_12_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  GreyImageTextButtonLookAndFeel grey{juce::Image()};
  CyanImageTextButtonLookAndFeel cyan{juce::Image()};
  juce::TextButton button;

  for (juce::LookAndFeel* lookAndFeel :
       std::initializer_list<juce::LookAndFeel*>{&grey, &cyan}) {
    const juce::Font font = lookAndFeel->getTextButtonFont(button, 40);
    EXPECT_EQ(font.getTypefaceName(), EclipsaLookAndFeel::kRoboto);
    EXPECT_NEAR(font.getHeightInPoints(), 12.0f, 0.01f);
  }
}
