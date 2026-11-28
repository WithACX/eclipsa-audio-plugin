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

#include <gtest/gtest.h>

#include "../src/screens/PresentationMonitorScreen.h"
#include "components/src/EclipsaLookAndFeel.h"

// Verifies the main screen's mix presentation tabs are 14 pt Roboto.
TEST(PresentationTabsLookAndFeel, tabs_are_14_point_roboto) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  const ScopedEclipsaDefaultLookAndFeel scope;
  PresentationTabsLookAndFeel lookAndFeel;
  juce::TabbedButtonBar bar(juce::TabbedButtonBar::TabsAtTop);
  juce::TabBarButton button("Mix", bar);

  const juce::Font font = lookAndFeel.getTabButtonFont(button, 30.f);

  EXPECT_EQ(font.getTypefaceName(), EclipsaLookAndFeel::kRoboto);
  EXPECT_NEAR(font.getHeightInPoints(), 14.f, 0.01f);
}
