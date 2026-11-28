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
#ifdef _WIN32
#include <errno.h>
#else
#include <sys/errno.h>
#endif

#include "../components.h"
#include "CapCentredText.h"
#include "EclipsaLookAndFeel.h"
#include "Icons.h"
#include "components/src/EclipsaColours.h"
#include "components/src/MainEditor.h"

/*
  Creates a header bar object, consisting of a title and a back button.
  For use by secondary screens which have a back button returning to the main
  screen
*/
class HeaderBarLookAndFeel : public juce::LookAndFeel_V4 {
 public:
  HeaderBarLookAndFeel() {
    setColour(juce::Label::textColourId, juce::Colours::white);

    setColour(juce::TextButton::ColourIds::buttonColourId,
              EclipsaColours::backgroundOffBlack);
    setColour(juce::TextButton::ColourIds::buttonOnColourId,
              juce::Colours::grey);

    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::ColourIds::textColourOnId,
              juce::Colours::white);
  }
};

class HeaderBar : public juce::Component {
  juce::String title_;
  juce::ImageButton backButton_;
  CapCentredLabel titleLabel_;
  HeaderBarLookAndFeel lookAndFeel_;

  static constexpr int kBackIconSize = 18;

 public:
  HeaderBar(juce::String title, MainEditor& editor) : juce::Component() {
    title_ = title;
    setLookAndFeel(&lookAndFeel_);
    backButton_.onClick = [this, &editor] {
      // Set the main screen to the routing screen
      editor.resetScreen();
    };
  }

  ~HeaderBar() override { setLookAndFeel(nullptr); }

  void paint(juce::Graphics& g) override {
    auto bounds = getLocalBounds();

    titleLabel_.setFont(EclipsaLookAndFeel::roboto(22.f));

    juce::Image resizedIcon =
        IconStore::getInstance().getBackArrowIcon().rescaled(
            kBackIconSize, kBackIconSize,
            juce::Graphics::highResamplingQuality);

    addAndMakeVisible(backButton_);
    backButton_.setImages(false, true, true, resizedIcon, 1.0f,
                          juce::Colours::transparentBlack, resizedIcon, 1.0f,
                          juce::Colours::grey, resizedIcon, 0.8f,
                          juce::Colours::white);

    // The title label spans the full height and centres its capitals, so a
    // vertically centred icon lines up with them.
    auto buttonBounds = bounds.removeFromLeft(kBackIconSize + 10);
    backButton_.setBounds(buttonBounds.withSizeKeepingCentre(
        buttonBounds.getWidth(), kBackIconSize));

    // Configure title label
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText(title_, juce::dontSendNotification);
    titleLabel_.setBounds(bounds.removeFromLeft(200));
    titleLabel_.setMinimumHorizontalScale(1.f);
  }
};