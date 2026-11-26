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

// clang-format off
#include <components/components.h>

#include "components/src/AudioFilePlayer.h"
#include "components/src/PlaybackSeekReadout.h"
// clang-format on

#include <gtest/gtest.h>

TEST(PlaybackTimeFormat, pads_minutes_below_one_hour) {
  EXPECT_EQ(formatPlaybackTime(0.0f, 0), "00:00 / 00:00");
  EXPECT_EQ(formatPlaybackTime(0.5f, 125), "01:02 / 02:05");
  EXPECT_EQ(formatPlaybackTime(1.0f, 3599), "59:59 / 59:59");
}

TEST(PlaybackTimeFormat, drops_minute_padding_from_one_hour) {
  EXPECT_EQ(formatPlaybackTime(0.0f, 3600), "0:00 / 60:00");
  EXPECT_EQ(formatPlaybackTime(0.5f, 7200), "60:00 / 120:00");
}

TEST(PlaybackSeekReadout, follows_processor_when_idle) {
  PlaybackSeekReadout readout;
  EXPECT_FALSE(readout.isDragging());
  EXPECT_FLOAT_EQ(readout.displayPosition(0.9f, 0.25f), 0.25f);
}

TEST(PlaybackSeekReadout, follows_slider_while_dragging) {
  PlaybackSeekReadout readout;
  readout.beginDrag();
  EXPECT_TRUE(readout.isDragging());
  EXPECT_FLOAT_EQ(readout.displayPosition(0.4f, 0.1f), 0.4f);
  EXPECT_FLOAT_EQ(readout.displayPosition(0.7f, 0.2f), 0.7f);
}

TEST(PlaybackSeekReadout, holds_release_target_until_processor_moves) {
  PlaybackSeekReadout readout;
  readout.beginDrag();
  readout.endDrag(0.7f, 0.1f);
  EXPECT_FALSE(readout.isDragging());
  EXPECT_FLOAT_EQ(readout.displayPosition(0.0f, 0.1f), 0.7f);
  EXPECT_FLOAT_EQ(readout.displayPosition(0.0f, 0.7f), 0.7f);
  // Once the processor has moved on, it is the source of truth again, even
  // if it later reports the pre-seek position.
  EXPECT_FLOAT_EQ(readout.displayPosition(0.0f, 0.71f), 0.71f);
  EXPECT_FLOAT_EQ(readout.displayPosition(0.0f, 0.1f), 0.1f);
}

TEST(PlaybackSeekReadout, new_drag_discards_pending_target) {
  PlaybackSeekReadout readout;
  readout.endDrag(0.7f, 0.1f);
  readout.beginDrag();
  EXPECT_FLOAT_EQ(readout.displayPosition(0.3f, 0.1f), 0.3f);
}

namespace {
class SeekCounter : public juce::ValueTree::Listener {
 public:
  void valueTreePropertyChanged(juce::ValueTree&,
                                const juce::Identifier& property) override {
    if (property == FilePlayback::kSeekPosition) {
      ++count;
    }
  }
  int count = 0;
};

class AudioFilePlayerSeekTest : public ::testing::Test {
 protected:
  AudioFilePlayerSeekTest() : fpbr_(juce::ValueTree{"test"}) {
    fpbData_.processorState.update(FilePlayback::ProcessorState::kPaused);
    fpbData_.fileDuration_s.update(125);
    fpbData_.currFilePosition.update(0.1f);
    player_ = std::make_unique<AudioFilePlayer>(fpbr_, fpbData_);
    for (int i = 0; i < player_->getNumChildComponents(); ++i) {
      auto* child = player_->getChildComponent(i);
      if (auto* slider = dynamic_cast<juce::Slider*>(child);
          slider != nullptr && slider->getMaximum() == 1.0) {
        seekSlider_ = slider;
      }
      if (auto* label = dynamic_cast<juce::Label*>(child);
          label != nullptr && label->getName() == "timeLabel") {
        timeLabel_ = label;
      }
    }
    // JUCE binds listeners to a ValueTree handle, so keep one alive.
    observedTree_ = fpbr_.getTree();
    observedTree_.addListener(&seekCounter_);
  }

  ~AudioFilePlayerSeekTest() override {
    observedTree_.removeListener(&seekCounter_);
  }

  // Mirrors the callbacks juce::Slider fires for a press, drag and release.
  void dragTo(const std::vector<double>& positions) {
    seekSlider_->onDragStart();
    for (const double kPos : positions) {
      seekSlider_->setValue(kPos, juce::sendNotificationSync);
    }
  }
  void release() { seekSlider_->onDragEnd(); }
  juce::String readout() const { return timeLabel_->getText(); }

  const juce::ScopedJuceInitialiser_GUI kJuce_;
  FilePlaybackRepository fpbr_;
  FilePlaybackProcessorData fpbData_;
  std::unique_ptr<AudioFilePlayer> player_;
  juce::Slider* seekSlider_ = nullptr;
  juce::Label* timeLabel_ = nullptr;
  juce::ValueTree observedTree_;
  SeekCounter seekCounter_;
};
}  // namespace

TEST_F(AudioFilePlayerSeekTest, finds_seek_bar_and_readout) {
  ASSERT_NE(seekSlider_, nullptr);
  ASSERT_NE(timeLabel_, nullptr);
  player_->timerCallback();
  EXPECT_EQ(readout(), "00:12 / 02:05");
}

TEST_F(AudioFilePlayerSeekTest, readout_tracks_cursor_during_drag) {
  ASSERT_NE(seekSlider_, nullptr);
  dragTo({0.5});
  EXPECT_EQ(readout(), "01:02 / 02:05");
  dragTo({0.8});
  EXPECT_EQ(readout(), "01:40 / 02:05");

  // The periodic refresh must not pull the readout back to the processor's
  // position while the cursor is held.
  player_->timerCallback();
  EXPECT_EQ(readout(), "01:40 / 02:05");
  EXPECT_EQ(seekCounter_.count, 0);
}

TEST_F(AudioFilePlayerSeekTest, release_seeks_to_shown_position_and_holds_it) {
  ASSERT_NE(seekSlider_, nullptr);
  dragTo({0.3, 0.8});
  release();
  EXPECT_EQ(seekCounter_.count, 1);
  EXPECT_FLOAT_EQ(fpbr_.get().getSeekPosition(), 0.8f);
  EXPECT_EQ(readout(), "01:40 / 02:05");

  // The processor still reports the pre-seek position until the seek lands.
  player_->timerCallback();
  EXPECT_EQ(readout(), "01:40 / 02:05");
  EXPECT_FLOAT_EQ(static_cast<float>(seekSlider_->getValue()), 0.8f);

  fpbData_.currFilePosition.update(0.9f);
  player_->timerCallback();
  EXPECT_EQ(readout(), "01:52 / 02:05");
}

TEST_F(AudioFilePlayerSeekTest, release_on_previous_target_seeks_again) {
  ASSERT_NE(seekSlider_, nullptr);
  dragTo({0.6});
  release();
  ASSERT_EQ(seekCounter_.count, 1);

  // Playback moved on from the first seek; releasing on the same spot must
  // still send the processor back there.
  fpbData_.currFilePosition.update(0.75f);
  player_->timerCallback();
  dragTo({0.2, 0.6});
  release();
  EXPECT_EQ(seekCounter_.count, 2);
  EXPECT_FLOAT_EQ(fpbr_.get().getSeekPosition(), 0.6f);
}
