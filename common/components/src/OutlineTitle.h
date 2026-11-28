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

#include "CapCentredText.h"
#include "EclipsaLookAndFeel.h"

// Draws the title that sits on the top outline of a text box or selection box,
// with its capital letters centred on the outline's stroke.
namespace OutlineTitle {

inline juce::Font font() { return EclipsaLookAndFeel::roboto(12.f); }

// Draws `text` from `x` with its capitals centred on the line at `lineY`, over
// a `background` strip, `padding` wider on each side, that hides the line.
inline void draw(juce::Graphics& g, const juce::String& text, const float x,
                 const float lineY, const float padding,
                 const juce::Colour textColour, const juce::Colour background) {
  const juce::Font titleFont = font();
  const float baseline = lineY + CapCentredText::capHeight(titleFont) / 2.f;
  const float width = titleFont.getStringWidthFloat(text);

  g.setColour(background);
  g.fillRect(x - padding, baseline - titleFont.getAscent(),
             width + 2.f * padding, titleFont.getHeight());

  g.setColour(textColour);
  CapCentredText::draw(g, titleFont, text, x, lineY);
}

}  // namespace OutlineTitle
