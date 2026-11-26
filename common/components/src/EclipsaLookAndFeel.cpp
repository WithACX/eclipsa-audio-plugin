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

#include "EclipsaLookAndFeel.h"

#include "EclipsaFonts.h"

EclipsaLookAndFeel::EclipsaLookAndFeel()
    : audiowide_(juce::Typeface::createSystemTypefaceFor(
          EclipsaFonts::AudiowideRegular_ttf,
          EclipsaFonts::AudiowideRegular_ttfSize)),
      roboto_(juce::Typeface::createSystemTypefaceFor(
          EclipsaFonts::RobotoRegular_ttf,
          EclipsaFonts::RobotoRegular_ttfSize)) {}

juce::Typeface::Ptr EclipsaLookAndFeel::getTypefaceForFont(
    const juce::Font& font) {
  const juce::String& family = font.getTypefaceName();
  if (family == kAudiowide && audiowide_ != nullptr) {
    return audiowide_;
  }
  if (family == kRoboto && roboto_ != nullptr) {
    return roboto_;
  }
  return juce::LookAndFeel_V4::getTypefaceForFont(font);
}

// JUCE caches each family's typeface on first use, so a lookup made before
// the default changed would otherwise keep serving the old face.
ScopedEclipsaDefaultLookAndFeel::Installation::Installation() {
  juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);
  juce::Typeface::clearTypefaceCache();
}

ScopedEclipsaDefaultLookAndFeel::Installation::~Installation() {
  juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
  juce::Typeface::clearTypefaceCache();
}
