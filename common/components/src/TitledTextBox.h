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

#ifdef _WIN32
#include <errno.h>
#else
#include <sys/errno.h>
#endif

#include "../components.h"
#include "OutlineTitle.h"

class TitledTextBox : public juce::Component {
  juce::String title_;
  juce::TextEditor textEditor_;
  juce::Colour outlineColour_;
  int titleBuffer_;
  std::function<void()> onReturnCallback_;
  std::function<void()> onFocusLostCallback_;
  bool suppressFocusLost_ = false;

 public:
  TitledTextBox(juce::String title)
      : juce::Component(), title_(title), textEditor_(), titleBuffer_(20) {
    // Set the outline colour to the enabled colour state
    resetLookAndFeel();

    // The editor overlaps the title, so it draws no background or outline of
    // its own. With no border or top indent, its line centres in its bounds.
    textEditor_.setJustification(juce::Justification::centredLeft);
    textEditor_.setBorder(juce::BorderSize<int>(0));
    textEditor_.setIndents(textEditor_.getLeftIndent(), 0);
    textEditor_.setColour(juce::TextEditor::backgroundColourId,
                          juce::Colours::transparentBlack);
    textEditor_.setColour(juce::TextEditor::outlineColourId,
                          juce::Colours::transparentBlack);
    textEditor_.setColour(juce::TextEditor::focusedOutlineColourId,
                          juce::Colours::transparentBlack);
    textEditor_.setColour(juce::TextEditor::textColourId,
                          EclipsaColours::headingGrey);
    textEditor_.setFont(EclipsaLookAndFeel::roboto(14.f));
    textEditor_.onReturnKey = [this]() { handleReturnKey(); };
    textEditor_.onFocusLost = [this]() { handleFocusLost(); };
  }

  ~TitledTextBox() override {
    setOnReturnCallback(nullptr);
    setOnFocusLostCallback(nullptr);
  }

  void setText(juce::String text) { textEditor_.setText(text); }

  juce::String getText() { return textEditor_.getText(); }

  void setTitle(juce::String title) { textEditor_.setTitle(title); }

  const juce::TextEditor* const getTextEditor() const { return &textEditor_; }

  void paint(juce::Graphics& g) override {
    // This component is drawn by drawing an outline box
    // Then drawing a label over the top left corner of the box with the title
    // Then drawing a text box inside the outline

    auto bounds = getLocalBounds();

    // Fill the background
    g.fillAll(EclipsaColours::backgroundOffBlack);

    // Draw the outline
    auto cornerSize = 5.0f;
    // Add some buffering space for where the title will be higher then the
    // outline
    juce::Rectangle<int> boxBounds = bounds.withTrimmedTop(titleBuffer_);
    g.setColour(outlineColour_);
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize,
                           1.0f);

    OutlineTitle::draw(g, title_, boxBounds.getX() + 15.f,
                       boxBounds.getY() + 0.5f, 5.f, outlineColour_,
                       EclipsaColours::backgroundOffBlack);

    addAndMakeVisible(textEditor_);
    textEditor_.setBounds(boxBounds.reduced(10, 1));
    textEditor_.setMultiLine(false);
  }

  void onTextChanged(std::function<void()> callback) {
    textEditor_.onTextChange = callback;
  }

  void setOnReturnCallback(std::function<void()> callback) {
    onReturnCallback_ = callback;
  }

  void setOnFocusLostCallback(std::function<void()> callback) {
    onFocusLostCallback_ = callback;
  }

  void setOnEscapeKeyCallback(std::function<void()> callback) {
    textEditor_.onEscapeKey = callback;
  }

  void setInputRestrictions(const int& maxLength,
                            const juce::String& allowedCharacters) {
    textEditor_.setInputRestrictions(maxLength, allowedCharacters);
  }

  void updateOutlineColour(juce::Colour colour) {
    outlineColour_ = colour;
    repaint();
  }

  void dimLookAndFeel() {
    updateOutlineColour(EclipsaColours::tabTextGrey.withAlpha(0.4f));
  }

  void resetLookAndFeel() { updateOutlineColour(EclipsaColours::tabTextGrey); }

  bool textEditorIsFocused() { return textEditor_.hasKeyboardFocus(true); }

  void setReadOnly(const bool isReadOnly) {
    textEditor_.setAccessible(!isReadOnly);
    textEditor_.setReadOnly(isReadOnly);
    textEditor_.setCaretVisible(!isReadOnly);
  }

  void reduceTitleBuffer(int amount) { titleBuffer_ -= amount; }

 private:
  void handleReturnKey() {
    if (onReturnCallback_) {
      onReturnCallback_();
    }

    suppressFocusLost_ = true;
    textEditor_.giveAwayKeyboardFocus();

    if (textEditor_.hasKeyboardFocus(true)) {
      suppressFocusLost_ = false;
    }
  }

  void handleFocusLost() {
    if (suppressFocusLost_) {
      suppressFocusLost_ = false;
      return;
    }

    if (onFocusLostCallback_) {
      onFocusLostCallback_();
    }
  }
};
