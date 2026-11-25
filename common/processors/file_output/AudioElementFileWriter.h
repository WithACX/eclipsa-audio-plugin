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
#include <juce_dsp/juce_dsp.h>

#include <memory>
#include <string>

#include "FileWriter.h"
#include "data_structures/src/AudioElement.h"

class AudioElementFileWriter {
 public:
  AudioElementFileWriter(const juce::String& filename, double sampleRate,
                         int bitDepth, AudioCodec codec, AudioElement& element)
      : element_(element),
        fileWriter_(std::make_unique<FileWriter>(
            filename, sampleRate,
            FileWriter::defaultWavChannelSet(element.getChannelCount()),
            element.getFirstChannel(), bitDepth, codec)) {}

  ~AudioElementFileWriter() { close(); }

  AudioElementFileWriter(const AudioElementFileWriter&) = delete;
  AudioElementFileWriter& operator=(const AudioElementFileWriter&) = delete;

  // A moved-from writer holds no FileWriter: its methods report "not open"
  // and its destructor closes nothing.
  AudioElementFileWriter(AudioElementFileWriter&&) noexcept = default;
  AudioElementFileWriter& operator=(AudioElementFileWriter&&) noexcept =
      default;

  // Returns whether the underlying file was successfully opened for writing.
  bool isOpen() const {
    return fileWriter_ != nullptr && fileWriter_->isOpen();
  }

  // Returns false on a write failure (disk full, WAV size limit, etc.).
  bool write(juce::AudioBuffer<float>& buffer) {
    return fileWriter_ != nullptr && fileWriter_->write(buffer);
  }

  // Returns false if the writer opened but failed to flush/close cleanly.
  bool close() { return fileWriter_ == nullptr || fileWriter_->close(); }

  AudioElement& getElement() { return element_; }

  std::string getFilePath() {
    return fileWriter_ != nullptr ? fileWriter_->getFilePath() : std::string();
  }

  juce::int64 getFramesWritten() {
    return fileWriter_ != nullptr ? fileWriter_->getFramesWritten() : 0;
  }

 private:
  AudioElement element_;  // Use a local copy to avoid updates elsewhere causing
                          // issues
  std::unique_ptr<FileWriter> fileWriter_;
};