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
#include <components/components.h>

#include "components/src/ExportValidation.h"
#include "components/src/FilePickerTextBox.h"
#include "components/src/SelectionBox.h"
#include "components/src/SliderButton.h"
#include "components/src/TimeFormatSegmentSelector.h"
#include "components/src/TitledLabel.h"
#include "components/src/TitledTextBox.h"
#include "data_repository/implementation/AudioElementRepository.h"
#include "data_repository/implementation/FileExportRepository.h"
#include "data_repository/implementation/MixPresentationRepository.h"
#include "data_structures/src/FileExport.h"

// Time format options for export start/end times
enum class TimeFormat {
  HoursMinutesSeconds,  // 00:00:00 (HH:MM:SS)
  BarsBeats,            // 1.1.000 (Bars.Beats.Ticks)
  Timecode              // 00:00:00:00 (HH:MM:SS:FF)
};

class FileExportScreen : public juce::Component,
                         public juce::ValueTree::Listener,
                         private juce::AsyncUpdater {
 public:
  FileExportScreen(MainEditor& editor, RepositoryCollection repos,
                   FilePlaybackProcessorData& fpbData);

  ~FileExportScreen();

  void refreshComponents();

  void refreshFileExportComponents();

  void resized() override;

  void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
  void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
                                const juce::Identifier& property) override;
  void valueTreeChildAdded(juce::ValueTree& parentTree,
                           juce::ValueTree& childWhichHasBeenAdded) override;
  void valueTreeChildRemoved(juce::ValueTree& parentTree,
                             juce::ValueTree& childWhichHasBeenRemoved,
                             int indexFromWhichChildWasRemoved) override;

 private:
  // Three fixed-width columns: export parameters, export audio, validation.
  static constexpr int kMainColumnWidth = 400;
  static constexpr int kMainColumnPadding = 50;
  static constexpr int kMinContentWidth =
      3 * kMainColumnWidth + 2 * kMainColumnPadding;
  static constexpr int kHeaderHeight = 45;
  static constexpr int kContentTopPadding = 25;
  static constexpr int kContentBottomPadding = 25;
  static constexpr int kMeasureHeight = 100000;

  void handleAsyncUpdate() override;

  // The header stays pinned; everything below it scrolls inside viewport_ so
  // no export control is clipped when the editor is smaller than the content.
  void updateLayout();

  // Lays the export controls out in `bounds` (content_ coordinates) and
  // returns the height they need.
  int layoutContent(juce::Rectangle<int> bounds);

  void configureCustomCodecParameter(AudioCodec format);

  // Time format conversion methods
  juce::String timeToString(juce::int64 sampleCount, TimeFormat format);
  juce::int64 stringToSamples(juce::String val, TimeFormat format);

  // Helper methods for timing info and format availability
  void updateTimingInfoFromHost();
  bool isTimeFormatAvailable(TimeFormat format);

  bool validFileExportConfig(const FileExport& config);

  // Video muxing only handles IAMF output: any other format turns the mux
  // toggle off and disables it. The caller persists exportVideo.
  void updateMuxVideoAvailability(AudioFileFormat format);

  // Helper to expand, validate, and update an error label for paths.
  std::pair<juce::String, bool> validateAndShowPathError(
      const juce::String& inputPath, const bool mustExist,
      juce::Label& errorLabel);

  MainEditor& editor_;
  FileExportRepository* repository_;
  AudioElementRepository* aeRepository_;
  MixPresentationRepository* mpRepository_;
  FilePlaybackRepository* filePlaybackRepository_;

  /*
   * ==============================
   * Component Declarations
   *===============================
   */

  HeaderBar headerBar_;
  juce::Viewport viewport_;
  juce::Component content_;

  // Left side elements - Time inputs (use TitledTextBox like other fields)
  juce::Label exportParametersLabel_;
  TitledTextBox startTimer_;
  juce::Label startTimerErrorLabel_;
  TimeFormatSegmentSelector startFormatSegments_;
  juce::Label startTimeFormatLabel_;

  TitledTextBox endTimer_;
  juce::Label endTimerErrorLabel_;
  TimeFormatSegmentSelector endFormatSegments_;
  juce::Label endTimeFormatLabel_;

  // Left side elements - Export format selectors
  SelectionBox formatSelector_;
  SelectionBox codecSelector_;
  SelectionBox bitDepthSelector_;
  TitledLabel sampleRate_;
  TitledTextBox customCodecParameter_;
  juce::Label customCodecParameterErrorLabel_;
  TitledLabel mixPresentations_;
  TitledLabel audioElements_;

  // Time format state
  TimeFormat startTimeFormat_;
  TimeFormat endTimeFormat_;

  // Cached timing information from host
  juce::Optional<double> cachedBpm_;
  juce::Optional<juce::AudioPlayHead::TimeSignature> cachedTimeSignature_;
  juce::Optional<juce::AudioPlayHead::FrameRate> cachedFrameRate_;

  // Right side elements
  juce::Label exportAudioLabel_;
  SliderButton enableFileExport_;
  FilePickerTextBox exportPath_;
  juce::Label exportPathErrorLabel_;
  juce::ImageButton browseButton_;
  juce::ToggleButton exportAudioElementsToggle_;
  juce::Label exportAudioElementsLabel_;
  juce::Label muxVidoeLabel_;
  SliderButton muxVideoToggle_;
  FilePickerTextBox exportVideoFolder_;
  juce::Label exportVideoFolderErrorLabel_;
  juce::ImageButton browseVideoButton_;
  TitledTextBox videoSource_;
  juce::Label videoSourceErrorLabel_;
  juce::ImageButton browseVideoSourceButton_;

  // Player elements
  ExportValidationComponent exportValidation_;

  // File selection elements
  juce::FileChooser audioOutputSelect_;
  juce::FileChooser muxVideoSourceSelect_;
  juce::FileChooser muxVideoOutputSelect_;

  // Manual export button -- To be removed later
  juce::TextButton exportButton_;
  juce::Label warningLabel_;
};