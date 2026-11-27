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

#include "components/src/HeaderBar.h"

#include <gtest/gtest.h>

#include "../src/RendererProcessor.h"
#include "components/src/EclipsaColours.h"
#include "components/src/EclipsaLookAndFeel.h"
#include "components/tests/InkMeasure.h"

namespace {

class StubEditor : public MainEditor {
 public:
  explicit StubEditor(juce::AudioProcessor& p) : MainEditor(p) {}
  void setScreen(juce::Component&) override {}
  void resetScreen() override {}
};

}  // namespace

// Verifies a page title's capitals line up with the back icon's centre.
TEST(HeaderBar, title_capitals_centred_on_back_icon) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  RendererProcessor processor;
  StubEditor editor(processor);
  HeaderBar header("EXPORT", editor);
  header.setSize(400, 45);

  const juce::Colour background = EclipsaColours::backgroundOffBlack;
  juce::Image image(juce::Image::ARGB, 400, 45, true);
  {
    juce::Graphics g(image);
    g.fillAll(background);
    header.paintEntireComponent(g, true);
  }

  const auto iconCentre = InkMeasure::inkCentreY(
      image, 0, 28, 0, 45, background, juce::Colours::white);
  const auto capsCentre = InkMeasure::inkCentreY(
      image, 28, 228, 0, 45, background, juce::Colours::white);
  ASSERT_TRUE(iconCentre.has_value());
  ASSERT_TRUE(capsCentre.has_value());
  EXPECT_NEAR(*capsCentre, *iconCentre, 0.75f);
}

// Verifies page titles draw in 22 pt Roboto.
TEST(HeaderBar, title_is_22_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  RendererProcessor processor;
  StubEditor editor(processor);
  HeaderBar header("Export options", editor);
  header.setSize(400, 45);
  juce::Image image(juce::Image::ARGB, 400, 45, true);
  {
    juce::Graphics g(image);
    header.paintEntireComponent(g, true);
  }

  const auto* label = dynamic_cast<juce::Label*>(header.getChildComponent(1));
  ASSERT_NE(label, nullptr);
  EXPECT_EQ(label->getFont().getTypefaceName(), EclipsaLookAndFeel::kRoboto);
  EXPECT_NEAR(label->getFont().getHeightInPoints(), 22.f, 0.01f);
}
