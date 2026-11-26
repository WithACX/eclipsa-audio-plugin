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

#include <juce_core/juce_core.h>

#include <optional>

// Formats the player's "current / total" readout. `position` is the
// normalized playback position in [0, 1]; files of an hour or longer drop the
// minute zero-padding so the readout stays narrow.
inline juce::String formatPlaybackTime(const float position,
                                       const unsigned long long duration_s) {
  const float kCurrentTime = position * duration_s;
  const int kCurrMinutes = static_cast<int>(kCurrentTime) / 60;
  const int kCurrSeconds = static_cast<int>(kCurrentTime) % 60;
  const int kTotalMinutes = static_cast<int>(duration_s) / 60;
  const int kTotalSeconds = static_cast<int>(duration_s) % 60;
  const char* kFormat =
      duration_s >= 3600 ? "%d:%02d / %d:%02d" : "%02d:%02d / %02d:%02d";
  return juce::String::formatted(kFormat, kCurrMinutes, kCurrSeconds,
                                 kTotalMinutes, kTotalSeconds);
}

// Decides which position the seek bar and time readout show. While the user
// drags, both follow the cursor. After release they hold the released target
// until the processor reports a new position: the seek runs on a worker
// thread, and until it lands the processor still reports the pre-seek
// position, which would otherwise snap the readout back for a moment.
class PlaybackSeekReadout {
 public:
  void beginDrag() {
    dragging_ = true;
    pendingTarget_.reset();
  }

  // `processorPosition` must be read after the seek was submitted, so that
  // any later change to it can only come from the seek landing.
  void endDrag(const float target, const float processorPosition) {
    dragging_ = false;
    pendingTarget_ = target;
    positionAtRelease_ = processorPosition;
  }

  bool isDragging() const { return dragging_; }

  float displayPosition(const float sliderPosition,
                        const float processorPosition) {
    if (dragging_) {
      return sliderPosition;
    }
    if (pendingTarget_.has_value()) {
      if (processorPosition == positionAtRelease_) {
        return *pendingTarget_;
      }
      pendingTarget_.reset();
    }
    return processorPosition;
  }

 private:
  bool dragging_ = false;
  std::optional<float> pendingTarget_;
  float positionAtRelease_ = 0.0f;
};
