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

#include <juce_graphics/juce_graphics.h>

#include <optional>

// Measures where text and icons land in a rendered image, for tests that
// check vertical alignment.
namespace InkMeasure {

// A pixel counts as ink when some channel differs from the background by at
// least half of the ink's own difference.
inline bool isInk(const juce::Colour pixel, const juce::Colour background,
                  const juce::Colour ink) {
  const auto diff = [](const juce::Colour a, const juce::Colour b) {
    return std::max({std::abs(a.getRed() - b.getRed()),
                     std::abs(a.getGreen() - b.getGreen()),
                     std::abs(a.getBlue() - b.getBlue())});
  };
  return diff(pixel, background) * 2 >= diff(ink, background);
}

// Vertical centre of the ink in columns [x0, x1) and rows [y0, y1).
inline std::optional<float> inkCentreY(const juce::Image& image, const int x0,
                                       const int x1, const int y0, const int y1,
                                       const juce::Colour background,
                                       const juce::Colour ink) {
  std::optional<int> first;
  int last = 0;
  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      if (isInk(image.getPixelAt(x, y), background, ink)) {
        first = first.value_or(y);
        last = y;
        break;
      }
    }
  }
  if (!first) {
    return std::nullopt;
  }
  return (*first + last + 1) / 2.f;
}

// Leftmost column in rows [y0, y1) holding ink.
inline std::optional<int> firstInkColumn(const juce::Image& image, const int y0,
                                         const int y1,
                                         const juce::Colour background,
                                         const juce::Colour ink) {
  for (int x = 0; x < image.getWidth(); ++x) {
    for (int y = y0; y < y1; ++y) {
      if (isInk(image.getPixelAt(x, y), background, ink)) {
        return x;
      }
    }
  }
  return std::nullopt;
}

}  // namespace InkMeasure
