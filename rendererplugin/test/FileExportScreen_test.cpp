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

#include "../src/screens/FileExportScreen.h"

#include <gtest/gtest.h>

#include "../src/RendererProcessor.h"

namespace {

class StubEditor : public MainEditor {
 public:
  explicit StubEditor(juce::AudioProcessor& p) : MainEditor(p) {}
  void setScreen(juce::Component&) override {}
  void resetScreen() override {}
};

// RendererEditor gives the screen its own size minus 80 px of side padding
// and 112 px of title chrome, and each visible warning banner takes 35 px.
juce::Rectangle<int> screenBoundsForEditor(int editorWidth, int editorHeight,
                                           int visibleBanners) {
  return {0, 0, editorWidth - 80, editorHeight - 112 - 35 * visibleBanners};
}

juce::Viewport* findViewport(juce::Component& screen) {
  for (juce::Component* child : screen.getChildren()) {
    if (auto* viewport = dynamic_cast<juce::Viewport*>(child)) {
      return viewport;
    }
  }
  return nullptr;
}

// Empty labels (error and warning text with nothing to say yet) are allowed
// zero bounds; every other visible control must have a real area.
bool isEmptyLabel(const juce::Component& c) {
  const auto* label = dynamic_cast<const juce::Label*>(&c);
  return label != nullptr && label->getText().isEmpty();
}

// Asserts every visible export control lies inside the scrollable content,
// so the viewport can scroll it into reach. Returns how many controls were
// laid out with a real area.
int checkLayout(FileExportScreen& screen, const juce::Rectangle<int>& size) {
  int visibleControls = 0;
  screen.setBounds(size);

  juce::Viewport* viewport = findViewport(screen);
  EXPECT_NE(viewport, nullptr) << "export controls are not scrollable";
  if (viewport == nullptr) {
    return visibleControls;
  }
  EXPECT_TRUE(screen.getLocalBounds().contains(viewport->getBounds()));
  EXPECT_FALSE(viewport->getBounds().isEmpty());

  juce::Component* content = viewport->getViewedComponent();
  EXPECT_NE(content, nullptr);
  if (content == nullptr) {
    return visibleControls;
  }

  for (juce::Component* control : content->getChildren()) {
    if (!control->isVisible()) {
      continue;
    }
    const auto bounds = control->getBounds();
    if (bounds.isEmpty()) {
      EXPECT_TRUE(isEmptyLabel(*control))
          << control->getName() << " collapsed to zero size at "
          << size.toString();
      continue;
    }
    EXPECT_TRUE(content->getLocalBounds().contains(bounds))
        << control->getName() << " at " << bounds.toString()
        << " falls outside the scrollable content "
        << content->getLocalBounds().toString() << " at " << size.toString();
    ++visibleControls;
  }
  return visibleControls;
}

juce::Component* findContentChild(FileExportScreen& screen,
                                  const juce::String& name) {
  juce::Viewport* viewport = findViewport(screen);
  if (viewport == nullptr || viewport->getViewedComponent() == nullptr) {
    return nullptr;
  }
  for (juce::Component* child : viewport->getViewedComponent()->getChildren()) {
    if (child->getName() == name) {
      return child;
    }
  }
  return nullptr;
}

const juce::StringArray kMuxVideoControls = {
    "MuxVideoLbl",          "MuxVideoToggle",    "VideoSource",
    "BrowseVideoSourceBtn", "ExportVideoFolder", "BrowseVideoBtn"};

// A control never added to the content counts as hidden.
void expectMuxVideoSectionShown(FileExportScreen& screen, bool shown) {
  for (const auto& name : kMuxVideoControls) {
    juce::Component* control = findContentChild(screen, name);
    const bool kVisible = control != nullptr && control->isVisible();
    EXPECT_EQ(kVisible, shown) << name;
  }
}

juce::Button* findMuxVideoToggle(FileExportScreen& screen) {
  return dynamic_cast<juce::Button*>(
      findContentChild(screen, "MuxVideoToggle"));
}

// Picks `label` in the format selector as a user would, firing its onChange.
void selectFormat(FileExportScreen& screen, const juce::String& label) {
  juce::Viewport* viewport = findViewport(screen);
  ASSERT_NE(viewport, nullptr);
  for (juce::Component* child : viewport->getViewedComponent()->getChildren()) {
    auto* selector = dynamic_cast<SelectionBox*>(child);
    if (selector == nullptr) {
      continue;
    }
    auto* comboBox = const_cast<juce::ComboBox*>(selector->getComboBox());
    for (int i = 0; i < comboBox->getNumItems(); ++i) {
      if (comboBox->getItemText(i) == label) {
        comboBox->setSelectedItemIndex(i, juce::sendNotificationSync);
        return;
      }
    }
  }
  FAIL() << "no format option " << label;
}

}  // namespace

// At 1024x768 and at the enforced minimum editor size, every export
// control stays laid out at full size and reachable by scrolling.
TEST(FileExportScreen, controls_stay_reachable_at_small_sizes) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  RendererProcessor processor;

  // Export enabled shows the most controls (the video mux rows).
  FileExportRepository& fileExportRepo = processor.getFileExportRepository();
  FileExport config = fileExportRepo.get();
  config.setExportAudio(true);
  fileExportRepo.update(config);

  StubEditor editor(processor);
  FileExportScreen screen(editor, processor.getRepositories(),
                          processor.getFilePlaybackProcessorData());

  // Default 1600x752 editor: the reference with no scrolling needed.
  const int kReferenceControls =
      checkLayout(screen, screenBoundsForEditor(1600, 752, 0));
  ASSERT_GT(kReferenceControls, 0);

  const juce::Rectangle<int> kSmallSizes[] = {
      // 1024x768 display minus a Windows taskbar, all four banners shown.
      screenBoundsForEditor(1024, 728, 4),
      screenBoundsForEditor(1024, 728, 0),
      // The fallback resize limit.
      screenBoundsForEditor(800, 500, 0),
      // RendererEditor's minimum on a 1024x768 display: 50% x 35%.
      screenBoundsForEditor(512, 256, 0),
  };
  for (const auto& size : kSmallSizes) {
    EXPECT_EQ(checkLayout(screen, size), kReferenceControls)
        << "controls lost at " << size.toString();
  }

  // At the smallest size the content overflows, so it must scroll both ways.
  checkLayout(screen, screenBoundsForEditor(512, 256, 0));
  juce::Viewport* viewport = findViewport(screen);
  ASSERT_NE(viewport, nullptr);
  EXPECT_GT(viewport->getViewedComponent()->getHeight(),
            viewport->getMaximumVisibleHeight());
  EXPECT_GT(viewport->getViewedComponent()->getWidth(),
            viewport->getMaximumVisibleWidth());
  EXPECT_TRUE(viewport->isVerticalScrollBarShown());
  EXPECT_TRUE(viewport->isHorizontalScrollBarShown());
}

// Video muxing only handles IAMF, so a WAV format hides the whole mux video
// section and stores the mux setting off; IAMF shows it again.
TEST(FileExportScreen, wav_format_hides_mux_video_section) {
  const juce::ScopedJuceInitialiser_GUI kJuce;
  RendererProcessor processor;

  FileExportRepository& fileExportRepo = processor.getFileExportRepository();
  FileExport config = fileExportRepo.get();
  config.setExportAudio(true);
  config.setAudioFileFormat(AudioFileFormat::IAMF);
  config.setExportVideo(true);
  fileExportRepo.update(config);

  StubEditor editor(processor);
  const juce::Rectangle<int> kSize = screenBoundsForEditor(1600, 752, 0);
  {
    FileExportScreen screen(editor, processor.getRepositories(),
                            processor.getFilePlaybackProcessorData());
    screen.setBounds(kSize);
    expectMuxVideoSectionShown(screen, true);
    juce::Button* toggle = findMuxVideoToggle(screen);
    ASSERT_NE(toggle, nullptr);
    EXPECT_TRUE(toggle->isEnabled());
    EXPECT_TRUE(toggle->getToggleState());

    for (const char* wavFormat : {"Binaural WAV", "Room WAV"}) {
      selectFormat(screen, wavFormat);
      expectMuxVideoSectionShown(screen, false);
      EXPECT_FALSE(toggle->getToggleState()) << wavFormat;
      EXPECT_FALSE(fileExportRepo.get().getExportVideo()) << wavFormat;
      // The rows below the hidden section stay laid out and reachable.
      EXPECT_GT(checkLayout(screen, kSize), 0) << wavFormat;
    }

    selectFormat(screen, "IAMF");
    expectMuxVideoSectionShown(screen, true);
    EXPECT_TRUE(toggle->isEnabled());
    EXPECT_FALSE(fileExportRepo.get().getExportVideo());

    selectFormat(screen, "Room WAV");
  }

  // Reopening the screen on a stored WAV format keeps the section hidden and
  // the mux setting off.
  ASSERT_EQ(fileExportRepo.get().getAudioFileFormat(), AudioFileFormat::WAV);
  FileExportScreen reopened(editor, processor.getRepositories(),
                            processor.getFilePlaybackProcessorData());
  reopened.setBounds(kSize);
  expectMuxVideoSectionShown(reopened, false);
  EXPECT_FALSE(fileExportRepo.get().getExportVideo());
}
