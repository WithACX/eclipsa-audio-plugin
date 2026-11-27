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

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// JUCE centres a font's whole line height, so text sits lower than the centre
// of its capitals suggests. These place text by its capitals instead, which
// lines it up with icons and outlines the way Figma does.
namespace CapCentredText {

// Height of a capital letter above the baseline.
inline float capHeight(const juce::Font& font) {
  juce::GlyphArrangement glyphs;
  glyphs.addLineOfText(font, "H", 0.f, 0.f);
  juce::Path outline;
  glyphs.createPath(outline);
  return -outline.getBounds().getY();
}

// Draws `text` from `x`, in the current colour, with its capitals centred on
// `centreY`.
inline void draw(juce::Graphics& g, const juce::Font& font,
                 const juce::String& text, const float x, const float centreY) {
  juce::GlyphArrangement glyphs;
  glyphs.addLineOfText(font, text, x, centreY + capHeight(font) / 2.f);
  glyphs.draw(g);
}

}  // namespace CapCentredText

// A juce::Label whose text has its capitals centred vertically in the label.
class CapCentredLabel : public juce::Label {
 public:
  using juce::Label::Label;

  void paint(juce::Graphics& g) override {
    g.fillAll(findColour(backgroundColourId));
    if (isBeingEdited()) {
      return;
    }

    const juce::Font font = getFont();
    const auto area =
        getBorderSize().subtractedFrom(getLocalBounds()).toFloat();
    const float width = font.getStringWidthFloat(getText());
    const auto justification = getJustificationType();
    float x = area.getX();
    if (justification.testFlags(juce::Justification::horizontallyCentred)) {
      x = area.getCentreX() - width / 2.f;
    } else if (justification.testFlags(juce::Justification::right)) {
      x = area.getRight() - width;
    }

    g.setColour(
        findColour(textColourId).withMultipliedAlpha(isEnabled() ? 1.f : 0.5f));
    CapCentredText::draw(g, font, getText(), x, area.getCentreY());
  }
};
