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

namespace {

// Only macOS draws Roboto text in the bundled face.
juce::Typeface::Ptr bundledRoboto() {
#if JUCE_MAC
  return juce::Typeface::createSystemTypefaceFor(
      EclipsaFonts::RobotoRegular_ttf, EclipsaFonts::RobotoRegular_ttfSize);
#else
  return nullptr;
#endif
}

}  // namespace

juce::String EclipsaLookAndFeel::robotoSubstituteName() {
  // Windows ships no Helvetica, so it gets Arial, which shares Helvetica's
  // widths and keeps layouts identical.
#if JUCE_MAC
  return "Helvetica";
#elif JUCE_WINDOWS
  return "Arial";
#else
  return juce::Font::getDefaultSansSerifFontName();
#endif
}

juce::Font EclipsaLookAndFeel::roboto(const float points) {
  return juce::Font(kRoboto, points, juce::Font::plain).withPointHeight(points);
}

EclipsaLookAndFeel::EclipsaLookAndFeel()
    : audiowide_(juce::Typeface::createSystemTypefaceFor(
          EclipsaFonts::AudiowideRegular_ttf,
          EclipsaFonts::AudiowideRegular_ttfSize)),
      roboto_(bundledRoboto()) {}

juce::Typeface::Ptr EclipsaLookAndFeel::getTypefaceForFont(
    const juce::Font& font) {
  const juce::String& family = font.getTypefaceName();
  if (family == kAudiowide && audiowide_ != nullptr) {
    return audiowide_;
  }
  if (family == kRoboto) {
    if (roboto_ != nullptr) {
      return roboto_;
    }
    juce::Font substitute(font);
    substitute.setTypefaceName(robotoSubstituteName());
    return juce::LookAndFeel_V4::getTypefaceForFont(substitute);
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
