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

// Serves the Audiowide typeface compiled into the plugin, so a juce::Font that
// names it renders in Audiowide whether or not the font is installed; every
// weight and style maps to the one bundled face. On macOS a font that names
// Roboto gets the bundled Roboto the same way; elsewhere it renders in
// robotoSubstituteName() at its own size and style.
class EclipsaLookAndFeel final : public juce::LookAndFeel_V4 {
 public:
  static constexpr const char* kAudiowide = "Audiowide";
  static constexpr const char* kRoboto = "Roboto";

  // The family Roboto text draws in when the bundled face is not served:
  // Arial on Windows, the default sans-serif on Linux, Helvetica on macOS.
  static juce::String robotoSubstituteName();

  // Plain Roboto at a Figma point size; a juce::Font height also counts line
  // spacing, so it reads smaller than the same number in Figma.
  static juce::Font roboto(float points);

  EclipsaLookAndFeel();

  juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;

 private:
  const juce::Typeface::Ptr audiowide_;
  const juce::Typeface::Ptr roboto_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EclipsaLookAndFeel)
};

// JUCE resolves every Font through the process-wide default LookAndFeel, never
// through a component's own, so these mappings only reach the labels while
// an EclipsaLookAndFeel is the default. Each plugin editor holds one of these
// as its first member: the first holder installs one shared instance and the
// last one to go uninstalls it, so several open editors stay correct.
class ScopedEclipsaDefaultLookAndFeel {
 public:
  ScopedEclipsaDefaultLookAndFeel() = default;

 private:
  class Installation {
   public:
    Installation();
    ~Installation();

   private:
    EclipsaLookAndFeel lookAndFeel_;
  };

  juce::SharedResourcePointer<Installation> installation_;
};
