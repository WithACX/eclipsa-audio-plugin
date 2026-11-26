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

#include "components/src/EclipsaLookAndFeel.h"
// clang-format on

#include <gtest/gtest.h>

#include <optional>

namespace {

juce::Font plainFont(const char* family) {
  return juce::Font(family, 12.0f, juce::Font::plain);
}

bool eclipsaIsDefault() {
  return dynamic_cast<EclipsaLookAndFeel*>(
             &juce::LookAndFeel::getDefaultLookAndFeel()) != nullptr;
}

}  // namespace

TEST(EclipsaLookAndFeel, serves_bundled_audiowide) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  EclipsaLookAndFeel lookAndFeel;

  const juce::Typeface::Ptr face =
      lookAndFeel.getTypefaceForFont(plainFont(EclipsaLookAndFeel::kAudiowide));

  ASSERT_NE(face, nullptr);
  EXPECT_EQ(face->getName(), "Audiowide");
}

TEST(EclipsaLookAndFeel, serves_bundled_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  EclipsaLookAndFeel lookAndFeel;

  const juce::Typeface::Ptr face =
      lookAndFeel.getTypefaceForFont(plainFont(EclipsaLookAndFeel::kRoboto));

  ASSERT_NE(face, nullptr);
  EXPECT_EQ(face->getName(), "Roboto");
}

TEST(EclipsaLookAndFeel, leaves_other_families_to_the_system) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  EclipsaLookAndFeel lookAndFeel;

  const juce::Typeface::Ptr face = lookAndFeel.getTypefaceForFont(
      plainFont(juce::Font::getDefaultMonospacedFontName().toRawUTF8()));

  ASSERT_NE(face, nullptr);
  EXPECT_NE(face->getName(), "Audiowide");
  EXPECT_NE(face->getName(), "Roboto");
}

// Call sites build fonts with juce::Font("Audiowide"/"Roboto", ...) and never
// name a LookAndFeel, so the scope must route those fonts to the bundled faces.
TEST(ScopedEclipsaDefaultLookAndFeel, routes_plain_fonts_to_bundled_faces) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;

  EXPECT_TRUE(eclipsaIsDefault());
  EXPECT_EQ(plainFont("Audiowide").getTypefacePtr()->getName(), "Audiowide");
  EXPECT_EQ(plainFont("Roboto").getTypefacePtr()->getName(), "Roboto");
}

TEST(ScopedEclipsaDefaultLookAndFeel, stays_installed_until_last_holder_goes) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  std::optional<ScopedEclipsaDefaultLookAndFeel> first;
  std::optional<ScopedEclipsaDefaultLookAndFeel> second;

  first.emplace();
  second.emplace();
  first.reset();
  EXPECT_TRUE(eclipsaIsDefault());

  second.reset();
  EXPECT_FALSE(eclipsaIsDefault());
}
