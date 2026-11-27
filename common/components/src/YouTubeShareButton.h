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

#include "EclipsaColours.h"
#include "Icons.h"

// Pill button with the YouTube icon followed by white text, centered as one
// group on a YouTube-red fill. Drawn dimmed while disabled.
class YouTubeShareButton : public juce::Button {
 public:
  static constexpr int kWidth = 265;
  // Matches the outlined box of a titled field in a 65 px row.
  static constexpr int kHeight = 45;

  YouTubeShareButton()
      : juce::Button("Share To YouTube"),
        icon_(IconStore::getInstance().getYouTubeIcon()) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
  }

 protected:
  void paintButton(juce::Graphics& g, bool isMouseOverButton,
                   bool isButtonDown) override {
    const auto bounds = getLocalBounds().toFloat();
    const float alpha = isEnabled() ? 1.0f : 0.4f;

    juce::Colour fill = EclipsaColours::youtubeRed;
    if (isEnabled() && isButtonDown)
      fill = fill.darker(0.2f);
    else if (isEnabled() && isMouseOverButton)
      fill = fill.brighter(0.1f);
    g.setColour(fill.withMultipliedAlpha(alpha));
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

    const juce::Font font("Roboto", 15.0f, juce::Font::plain);
    const float textWidth = font.getStringWidthFloat(getButtonText());
    const float iconHeight = bounds.getHeight() * 0.4f;
    const float iconWidth =
        icon_.isNull() ? 0.0f
                       : iconHeight * icon_.getWidth() / icon_.getHeight();
    const float gap = 8.0f;
    const float x = bounds.getCentreX() - (iconWidth + gap + textWidth) * 0.5f;

    g.setOpacity(alpha);
    g.drawImage(
        icon_,
        {x, bounds.getCentreY() - iconHeight * 0.5f, iconWidth, iconHeight},
        juce::RectanglePlacement::centred);

    g.setColour(juce::Colours::white.withMultipliedAlpha(alpha));
    g.setFont(font);
    g.drawText(getButtonText(),
               juce::Rectangle<float>(x + iconWidth + gap, bounds.getY(),
                                      textWidth + 1.0f, bounds.getHeight()),
               juce::Justification::centredLeft, false);
  }

 private:
  juce::Image icon_;
};
