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

#include "components/src/YouTubeUpload.h"

#include <gtest/gtest.h>

namespace {

FileExport muxConfig(const juce::String& videoPath) {
  FileExport config;
  config.setExportVideo(true);
  config.setVideoExportFolder(videoPath);
  return config;
}

}  // namespace

// The upload URL must not name a channel, so it works for any signed-in user.
TEST(test_youtube_upload, uploadUrlIsChannelAgnostic) {
  EXPECT_STREQ(YouTubeUpload::kUploadPageUrl, "https://www.youtube.com/upload");
}

// An existing muxed file is returned when muxing is enabled.
TEST(test_youtube_upload, findsExistingMuxedVideo) {
  juce::TemporaryFile video(".mp4");
  ASSERT_TRUE(video.getFile().create().wasOk());

  const juce::File kFound = YouTubeUpload::findMuxedVideo(
      muxConfig(video.getFile().getFullPathName()));

  EXPECT_EQ(kFound, video.getFile());
}

// A stale file from an earlier mux is not offered once muxing is turned off.
TEST(test_youtube_upload, ignoresVideoWhenMuxDisabled) {
  juce::TemporaryFile video(".mp4");
  ASSERT_TRUE(video.getFile().create().wasOk());
  FileExport config = muxConfig(video.getFile().getFullPathName());
  config.setExportVideo(false);

  EXPECT_EQ(YouTubeUpload::findMuxedVideo(config), juce::File());
}

// A configured path with nothing on disk yet (export not run) returns no file.
TEST(test_youtube_upload, returnsNoFileWhenVideoMissing) {
  juce::TemporaryFile video(".mp4");
  ASSERT_FALSE(video.getFile().exists());

  EXPECT_EQ(YouTubeUpload::findMuxedVideo(
                muxConfig(video.getFile().getFullPathName())),
            juce::File());
}

// Empty and relative paths return no file instead of tripping juce::File's
// absolute-path assertion.
TEST(test_youtube_upload, returnsNoFileForEmptyOrRelativePath) {
  EXPECT_EQ(YouTubeUpload::findMuxedVideo(muxConfig("")), juce::File());
  EXPECT_EQ(YouTubeUpload::findMuxedVideo(muxConfig("video.mp4")),
            juce::File());
}

// A home-relative path is expanded before the existence check.
TEST(test_youtube_upload, expandsTildePath) {
  const juce::File kHome =
      juce::File::getSpecialLocation(juce::File::userHomeDirectory);
  juce::TemporaryFile video(kHome.getChildFile("youtube_upload_test.mp4"));
  ASSERT_TRUE(video.getFile().create().wasOk());

  const juce::String kTildePath = "~/" + video.getFile().getFileName();

  EXPECT_EQ(YouTubeUpload::findMuxedVideo(muxConfig(kTildePath)),
            video.getFile());
}
