/*
 * Copyright 2025 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Sizes a resizable plugin editor to a display, then shrinks it once it is on
// screen if host chrome beside it (such as REAPER's FX-chain list) pushes its
// edge past the display's usable area.
class DisplayFittedEditorSize final : private juce::ComponentListener {
 public:
  struct Sizing {
    int width;
    int height;
    int minWidth;
    int minHeight;
    int maxWidth;
    int maxHeight;
  };

  // Space kept clear between the editor and the display edge for host chrome.
  static constexpr int kDisplayEdgeMargin = 16;

  // Even opening size and resize limits for `preferred` on a display whose
  // usable area is `userArea`.
  static Sizing initialSizing(juce::Point<int> preferred,
                              juce::Rectangle<int> userArea);

  // Even size that keeps `editorBounds` inside `userArea` less the edge
  // margin, never smaller than `minimum` or larger than the current size.
  // Both rectangles share one coordinate space.
  static juce::Point<int> sizeToFitArea(juce::Rectangle<int> editorBounds,
                                        juce::Rectangle<int> userArea,
                                        juce::Point<int> minimum);

  explicit DisplayFittedEditorSize(juce::AudioProcessorEditor& editor);
  ~DisplayFittedEditorSize() override;

  // Call from the editor constructor where its children already exist, since
  // setSize runs the editor's resized().
  void applyInitialSize(int preferredWidth, int preferredHeight);

 private:
  void componentParentHierarchyChanged(juce::Component&) override;
  void fitToDisplay();

  juce::AudioProcessorEditor& editor_;
  bool fitPending_ = false;
  bool fitted_ = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DisplayFittedEditorSize)
};
