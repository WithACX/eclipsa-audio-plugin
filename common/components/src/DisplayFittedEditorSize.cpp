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

#include "DisplayFittedEditorSize.h"

namespace {

// Odd editor dimensions cause rendering artifacts.
int roundDownToEven(const int value) { return value & ~1; }
int roundUpToEven(const int value) { return (value + 1) & ~1; }

int fitLength(const int current, const int available, const int minimum) {
  if (current <= available) {
    return current;
  }
  return juce::jmax(minimum, roundDownToEven(available));
}

}  // namespace

DisplayFittedEditorSize::Sizing DisplayFittedEditorSize::initialSizing(
    const juce::Point<int> preferred, const juce::Rectangle<int> userArea) {
  Sizing sizing{};
  sizing.maxWidth = roundDownToEven(userArea.getWidth());
  sizing.maxHeight = roundDownToEven(userArea.getHeight());
  sizing.minWidth = roundUpToEven(
      juce::jmin(800, static_cast<int>(userArea.getWidth() * 0.5f)));
  sizing.minHeight = roundUpToEven(
      juce::jmin(500, static_cast<int>(userArea.getHeight() * 0.35f)));
  sizing.width = juce::jmin(roundUpToEven(preferred.x), sizing.maxWidth);
  sizing.height = juce::jmin(roundUpToEven(preferred.y), sizing.maxHeight);
  return sizing;
}

juce::Point<int> DisplayFittedEditorSize::sizeToFitArea(
    const juce::Rectangle<int> editorBounds,
    const juce::Rectangle<int> userArea, const juce::Point<int> minimum) {
  const int availableWidth =
      userArea.getRight() - kDisplayEdgeMargin - editorBounds.getX();
  const int availableHeight =
      userArea.getBottom() - kDisplayEdgeMargin - editorBounds.getY();
  return {fitLength(editorBounds.getWidth(), availableWidth, minimum.x),
          fitLength(editorBounds.getHeight(), availableHeight, minimum.y)};
}

DisplayFittedEditorSize::DisplayFittedEditorSize(
    juce::AudioProcessorEditor& editor)
    : editor_(editor) {
  editor_.addComponentListener(this);
}

DisplayFittedEditorSize::~DisplayFittedEditorSize() {
  editor_.removeComponentListener(this);
}

void DisplayFittedEditorSize::applyInitialSize(const int preferredWidth,
                                               const int preferredHeight) {
  editor_.setResizable(true, true);

  // No peer exists yet, so use the display the user just clicked on.
  const auto& displays = juce::Desktop::getInstance().getDisplays();
  const auto* display = displays.getDisplayForPoint(
      juce::Desktop::getInstance().getMousePosition());
  if (display == nullptr) {
    display = displays.getPrimaryDisplay();
  }

  if (display == nullptr) {
    editor_.setSize(1200, 650);
    editor_.setResizeLimits(800, 500, 1600, 900);
    return;
  }

  const Sizing sizing =
      initialSizing({preferredWidth, preferredHeight}, display->userArea);
  editor_.setSize(sizing.width, sizing.height);
  editor_.setResizeLimits(sizing.minWidth, sizing.minHeight, sizing.maxWidth,
                          sizing.maxHeight);
}

void DisplayFittedEditorSize::componentParentHierarchyChanged(
    juce::Component&) {
  if (fitted_ || fitPending_ || editor_.getPeer() == nullptr) {
    return;
  }

  // Deferred so the host finishes placing and sizing its window first.
  fitPending_ = true;
  juce::MessageManager::callAsync(
      [this,
       safeEditor = juce::Component::SafePointer<juce::Component>(&editor_)] {
        if (safeEditor != nullptr) {
          fitPending_ = false;
          fitToDisplay();
        }
      });
}

void DisplayFittedEditorSize::fitToDisplay() {
  if (editor_.getPeer() == nullptr) {
    return;
  }
  fitted_ = true;

  const auto* display =
      juce::Desktop::getInstance().getDisplays().getDisplayForRect(
          editor_.getScreenBounds());
  if (display == nullptr) {
    return;
  }

  // Editor-local coordinates, so a host scale factor does not skew the fit.
  const auto localUserArea = editor_.getLocalArea(nullptr, display->userArea);

  juce::Point<int> minimum{0, 0};
  if (auto* constrainer = editor_.getConstrainer()) {
    minimum = {constrainer->getMinimumWidth(), constrainer->getMinimumHeight()};
  }

  const auto fitted =
      sizeToFitArea(editor_.getLocalBounds(), localUserArea, minimum);
  if (fitted.x != editor_.getWidth() || fitted.y != editor_.getHeight()) {
    editor_.setSize(fitted.x, fitted.y);
  }
}
