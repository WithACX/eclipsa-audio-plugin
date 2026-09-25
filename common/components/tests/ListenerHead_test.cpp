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

// Pull in the umbrella header first: several components/src headers include
// components.h themselves and only resolve correctly once the umbrella has
// fully loaded once. See the same note in Coordinates_test.cpp.
// clang-format off
#include <components/components.h>

#include "components/src/room_views/ListenerHead.h"
// clang-format on

#include <gtest/gtest.h>

namespace {

using Elevation = AudioElementSpatialLayout::Elevation;

// Surfaces above the room's centre draw over the head.
TEST(PannerHeadOrder, HeadFirstUnderRaisedPatterns) {
  for (const Elevation kElevation : {Elevation::kTent, Elevation::kArch,
                                     Elevation::kDome, Elevation::kCurve}) {
    EXPECT_TRUE(ListenerHead::drawnBeforeElevation(kElevation, 0.f));
  }
}

// With no surface, the head goes where a surface would draw over it.
TEST(PannerHeadOrder, HeadFirstWithNoPattern) {
  EXPECT_TRUE(ListenerHead::drawnBeforeElevation(Elevation::kNone, 0.f));
}

// A flat surface below the head is drawn first so the head sits over it.
TEST(PannerHeadOrder, SurfaceFirstForFlatBelowTheHead) {
  EXPECT_FALSE(ListenerHead::drawnBeforeElevation(Elevation::kFlat, -1.f));
  EXPECT_FALSE(ListenerHead::drawnBeforeElevation(Elevation::kFlat, -0.02f));
}

// A flat surface at or above the head draws over it.
TEST(PannerHeadOrder, HeadFirstForFlatAtOrAboveTheHead) {
  EXPECT_TRUE(ListenerHead::drawnBeforeElevation(Elevation::kFlat, 0.f));
  EXPECT_TRUE(ListenerHead::drawnBeforeElevation(Elevation::kFlat, 0.02f));
  EXPECT_TRUE(ListenerHead::drawnBeforeElevation(Elevation::kFlat, 1.f));
}

}  // namespace
