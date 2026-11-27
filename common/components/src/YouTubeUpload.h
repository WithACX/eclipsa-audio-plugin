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

#include <juce_core/juce_core.h>

#include "data_structures/src/FileExport.h"

// Hands a muxed export off to YouTube's web upload flow. Browsers do not let a
// URL pre-select a file for upload, so the best a link can do is open the
// upload dialog and reveal the file for the user to drag in.
namespace YouTubeUpload {

// Redirects to YouTube Studio's upload dialog for whichever channel the user
// is signed in to, so no channel ID is needed.
constexpr char kUploadPageUrl[] = "https://www.youtube.com/upload";

// Returns the muxed video if muxing is enabled and the file exists on disk,
// otherwise a default-constructed juce::File.
inline juce::File findMuxedVideo(const FileExport& config) {
  if (!config.getExportVideo()) return {};
  const juce::String kPath =
      FileExport::expandTildePath(config.getVideoExportFolder());
  // juce::File asserts on relative paths.
  if (!juce::File::isAbsolutePath(kPath)) return {};
  const juce::File kVideo(kPath);
  return kVideo.existsAsFile() ? kVideo : juce::File();
}

// Opens the URL without taking focus from the frontmost app. Returns false if
// the browser could not be launched that way.
inline bool openInBackground(const juce::String& url) {
#if JUCE_MAC
  juce::ChildProcess open;
  if (!open.start(juce::StringArray{"/usr/bin/open", "-g", url})) return false;
  open.waitForProcessToFinish(3000);
  return open.getExitCode() == 0;
#else
  juce::ignoreUnused(url);
  return false;
#endif
}

// Reveals the muxed video (if any) and opens the upload dialog behind it, so
// the file manager window stays on top, ready to drag from. The reveal must
// come first: macOS only lets the frontmost app raise another app's window.
inline void openUploadPage(const FileExport& config) {
  const juce::File kVideo = findMuxedVideo(config);
  if (kVideo == juce::File()) {
    juce::URL(kUploadPageUrl).launchInDefaultBrowser();
    return;
  }
  kVideo.revealToUser();
  if (!openInBackground(kUploadPageUrl))
    juce::URL(kUploadPageUrl).launchInDefaultBrowser();
}

}  // namespace YouTubeUpload
