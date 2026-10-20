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

#include "processors/file_output/WavFileOutputProcessor.h"

#include <gtest/gtest.h>
#include <juce_data_structures/juce_data_structures.h>

#include <filesystem>
#include <functional>
#include <vector>

#include "data_repository/implementation/FileExportRepository.h"
#include "data_repository/implementation/RoomSetupRepository.h"
#include "data_structures/src/FileExport.h"

namespace {
class TestFileExportRepository : public FileExportRepository {
 public:
  TestFileExportRepository() : FileExportRepository(juce::ValueTree{"test"}) {}
};

class TestRoomSetupRepository : public RoomSetupRepository {
 public:
  TestRoomSetupRepository() : RoomSetupRepository(juce::ValueTree{"test"}) {}
};

// Wraps a real FileWriter (so construction still performs a real open
// against a real, writable path) but lets a test force write()/close() to
// report failure afterward -- the only way to exercise the
// write/close-after-successful-open branches in WavFileOutputProcessor,
// since production only reaches them via disk-full/WAV-4GB-cap conditions.
class FakeFileWriter : public FileWriter {
 public:
  using FileWriter::FileWriter;

  bool write(juce::AudioBuffer<float>& buffer) override {
    ++writeCallCount;
    if (forceWriteFailure) return false;
    return FileWriter::write(buffer);
  }

  bool close() override {
    if (forceCloseFailure) return false;
    return FileWriter::close();
  }

  bool forceWriteFailure = false;
  bool forceCloseFailure = false;
  int writeCallCount = 0;
};

// Queues WavFileOutputProcessor's deferred repository updates (see
// deferRepositoryUpdate()/postToMessageThread()) instead of posting them to
// juce::MessageManager::callAsync -- there is no running JUCE message loop
// in this test binary to pump (runDispatchLoopUntil isn't even compiled in;
// JUCE_MODAL_LOOPS_PERMITTED is 0 for this plugin build). Running the tasks
// immediately/synchronously here would defeat the very thing being tested:
// both the deadlock and the clobbering hazard deferRepositoryUpdate() exists
// to avoid only manifest when a write happens nested inside the triggering
// update() call's own call stack. Queuing and draining explicitly via
// runPendingTasks() -- called only after that outer call has fully
// returned -- reproduces the real timing guarantee callAsync provides,
// without needing a real message loop. Production code always goes through
// the real base-class implementation.
class TestableWavFileOutputProcessor : public WavFileOutputProcessor {
 public:
  using WavFileOutputProcessor::WavFileOutputProcessor;

  void runPendingTasks() {
    std::vector<std::function<void()>> tasks;
    tasks.swap(pendingTasks_);
    for (auto& task : tasks) task();
  }

  size_t pendingTaskCount() const { return pendingTasks_.size(); }

  // Set before triggering an export start so the FileWriter it opens is a
  // FakeFileWriter; the returned pointer is only valid for the lifetime of
  // that export (WavFileOutputProcessor deletes it on close).
  bool forceWriteFailure = false;
  bool forceCloseFailure = false;
  FakeFileWriter* lastFakeWriter = nullptr;

 protected:
  void postToMessageThread(std::function<void()> task) override {
    pendingTasks_.push_back(std::move(task));
  }

  FileWriter* createFileWriter(const juce::String& filename, double sampleRate,
                               const juce::AudioChannelSet& channelSet,
                               int firstChannel, int bitDepth,
                               AudioCodec codec) override {
    auto* fake = new FakeFileWriter(filename, sampleRate, channelSet,
                                    firstChannel, bitDepth, codec);
    fake->forceWriteFailure = forceWriteFailure;
    fake->forceCloseFailure = forceCloseFailure;
    lastFakeWriter = fake;
    return fake;
  }

 private:
  std::vector<std::function<void()>> pendingTasks_;
};

class WavFileOutputTests : public ::testing::Test {
 protected:
  TestFileExportRepository fileExportRepository;
  TestRoomSetupRepository roomSetupRepository;
  TestableWavFileOutputProcessor processor{fileExportRepository,
                                           roomSetupRepository};
};
}  // namespace

// WavFileOutputProcessor registers itself as a listener on
// fileExportRepository (see its constructor); FileExportRepository::update()
// fires that listener synchronously on the calling thread. Toggling
// ManualExport through the repository -- exactly how production code (the
// export button in FileExportScreen.cpp) drives this processor -- exercises
// that same re-entrant path. Before the deferRepositoryUpdate() fix, an open
// failure here recorded the ExportError via a repository update issued
// synchronously from inside that listener callback, which re-entered
// setNonRealtime()'s non-reentrant lock_ and deadlocked. This test hanging
// (rather than completing) is the regression signal for that bug.
TEST_F(WavFileOutputTests,
       open_failure_via_manual_export_toggle_does_not_deadlock) {
  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(16);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile("/invalid_path/test.wav");
  config.setManualExport(false);
  fileExportRepository.update(config);

  // Flipping ManualExport to true synchronously triggers this processor's
  // own valueTreePropertyChanged -> checkManualExportStartStop ->
  // setNonRealtime path.
  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_NE(fileExportRepository.get().getExportError(), ExportError::kNoError);

  // Stopping the (failed) render must also complete without hanging, and
  // must not crash on the null/never-opened writer.
  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();
}

// Happy-path regression guard mirroring FileOutputTests' equivalent: a
// successful open/close on a valid path must leave ExportError at kNoError.
TEST_F(WavFileOutputTests, successful_export_leaves_no_error) {
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() / "wav_output_test.wav";
  std::filesystem::remove(kOutPath);

  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(16);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile(kOutPath.string());
  config.setManualExport(false);
  fileExportRepository.update(config);

  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_EQ(fileExportRepository.get().getExportError(), ExportError::kNoError);

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_TRUE(std::filesystem::exists(kOutPath));
  std::filesystem::remove(kOutPath);
}

// Regression test for a subtler hazard than the deadlock above: the real
// export button handler (rendererplugin/src/screens/FileExportScreen.cpp)
// does `config = repo.get(); config.setManualExport(!...); repo.update(config)`
// -- a read-modify-write of a FULL FileExport snapshot. Before
// deferRepositoryUpdate(), a synchronous classification write issued from
// inside the listener callback this triggers got silently overwritten
// moments later, when that same caller's copyPropertiesFrom loop went on to
// reapply its own stale (pre-attempt) exportError property (FileExport.h
// declares manualExport before exportError, so the loop processes
// exportError second, in every call). This test reproduces that exact
// caller pattern across a failed attempt followed by a successful retry,
// and asserts the retry's success is neither clobbered by, nor leaks, the
// prior failure.
TEST_F(WavFileOutputTests, retry_after_failure_does_not_clobber_or_leak_error) {
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() / "wav_retry_test.wav";
  std::filesystem::remove(kOutPath);

  // First attempt: invalid path, fails.
  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(16);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile("/invalid_path/test.wav");
  config.setManualExport(false);
  fileExportRepository.update(config);

  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();
  EXPECT_NE(fileExportRepository.get().getExportError(), ExportError::kNoError);

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  // Second attempt, mirroring FileExportScreen.cpp's export button handler
  // exactly: the snapshot captured here still holds the failed ExportError
  // from the first attempt, before flipping ManualExport and writing it
  // back with everything else it doesn't intend to change.
  config = fileExportRepository.get();
  config.setExportFile(kOutPath.string());
  config.setManualExport(!config.getManualExport());
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_EQ(fileExportRepository.get().getExportError(), ExportError::kNoError);

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_TRUE(std::filesystem::exists(kOutPath));
  std::filesystem::remove(kOutPath);
}

// Regression for recordWriteFailureIfAny() used to call deferRepositoryUpdate()
// unconditionally on every failing processBlock, ensuring
// persistent failures don't queue more than once. This reproduces the
// open-succeeds-then-every-write-fails shape via FakeFileWriter and asserts
// exactly one closure is queued no matter how many blocks fail.
TEST_F(WavFileOutputTests,
       many_failed_writes_after_successful_open_queue_a_single_update) {
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() / "wav_write_failure_test.wav";
  std::filesystem::remove(kOutPath);

  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(16);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile(kOutPath.string());
  config.setManualExport(false);
  fileExportRepository.update(config);

  processor.forceWriteFailure = true;
  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();  // Drain the export-start clean-slate reset.
  EXPECT_EQ(fileExportRepository.get().getExportError(), ExportError::kNoError);
  ASSERT_NE(processor.lastFakeWriter, nullptr);
  EXPECT_TRUE(processor.lastFakeWriter->isOpen());

  juce::AudioBuffer<float> buffer(2, 64);
  buffer.clear();
  juce::MidiBuffer midi;
  for (int i = 0; i < 5; ++i) {
    processor.processBlock(buffer, midi);
  }

  EXPECT_EQ(processor.lastFakeWriter->writeCallCount, 5);
  // The fix under test: 5 failing blocks must queue exactly one
  // deferRepositoryUpdate() closure, not one per block.
  EXPECT_EQ(processor.pendingTaskCount(), 1u);

  processor.runPendingTasks();
  EXPECT_EQ(fileExportRepository.get().getExportError(),
            ExportError::kFileWriteFailed);

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();
  std::filesystem::remove(kOutPath);
}

// Companion regression test covering the sibling untested branch flagged in
// the same review: a close() failure after a successful open and successful
// writes must still be recorded as kFileWriteFailed.
TEST_F(WavFileOutputTests, close_failure_after_successful_open_is_recorded) {
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() / "wav_close_failure_test.wav";
  std::filesystem::remove(kOutPath);

  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(16);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile(kOutPath.string());
  config.setManualExport(false);
  fileExportRepository.update(config);

  processor.forceCloseFailure = true;
  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();
  EXPECT_EQ(fileExportRepository.get().getExportError(), ExportError::kNoError);

  juce::AudioBuffer<float> buffer(2, 64);
  buffer.clear();
  juce::MidiBuffer midi;
  processor.processBlock(buffer, midi);  // A successful write.

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  EXPECT_EQ(fileExportRepository.get().getExportError(),
            ExportError::kFileWriteFailed);
  std::filesystem::remove(kOutPath);
}

namespace {
using ChannelType = juce::AudioChannelSet::ChannelType;

// Fills rendered channel i with this value, so each written channel shows
// which rendered channel it came from.
float ituMarker(int ituIndex) { return (ituIndex + 1) / 32.f; }

std::unique_ptr<juce::AudioFormatReader> readWav(
    const std::filesystem::path& path) {
  return std::unique_ptr<juce::AudioFormatReader>(
      juce::WavAudioFormat().createReaderFor(
          new juce::FileInputStream(juce::File(path.string())), true));
}

struct RoomWavCase {
  Speakers::AudioElementSpeakerLayout layout;
  std::vector<ChannelType> expectedSet;
  // Rendered (ITU order) channel expected in each written channel.
  std::vector<int> expectedSources;
};

// Keeps the test names ctest registers free of the parameter's raw bytes.
void PrintTo(const RoomWavCase& roomWavCase, std::ostream* os) {
  *os << roomWavCase.layout.toString();
}

const ChannelType L = ChannelType::left, R = ChannelType::right,
                  C = ChannelType::centre, LFE = ChannelType::LFE,
                  Ls = ChannelType::leftSurround,
                  Rs = ChannelType::rightSurround,
                  Lss = ChannelType::leftSurroundSide,
                  Rss = ChannelType::rightSurroundSide,
                  Ltf = ChannelType::topFrontLeft,
                  Rtf = ChannelType::topFrontRight,
                  Ltr = ChannelType::topRearLeft,
                  Rtr = ChannelType::topRearRight;

juce::AudioChannelSet makeSet(const std::vector<ChannelType>& types) {
  juce::AudioChannelSet set;
  for (const ChannelType kType : types) set.addChannel(kType);
  return set;
}

class WavFileOutputRoomOrder
    : public WavFileOutputTests,
      public ::testing::WithParamInterface<RoomWavCase> {};
}  // namespace

// Room WAV writes each room layout's rendered ITU channels in the export set's
// getChannelTypes() order, with that set as the file's channel mask.
TEST_P(WavFileOutputRoomOrder, writes_channels_in_export_set_order) {
  const RoomWavCase& kCase = GetParam();
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() /
      ("wav_room_order_" + kCase.layout.toString().toStdString() + ".wav");
  std::filesystem::remove(kOutPath);

  RoomSetup roomSetup = roomSetupRepository.get();
  for (const RoomLayout& kOption : speakerLayoutConfigurationOptions) {
    if (kOption.getRoomSpeakerLayout() == kCase.layout) {
      roomSetup.setSpeakerLayout(kOption);
    }
  }
  roomSetupRepository.update(roomSetup);
  ASSERT_EQ(roomSetupRepository.get().getSpeakerLayout().getRoomSpeakerLayout(),
            kCase.layout);

  FileExport config = fileExportRepository.get();
  config.setAudioFileFormat(AudioFileFormat::WAV);
  config.setExportAudio(true);
  config.setSampleRate(48000);
  config.setBitDepth(24);
  config.setAudioCodec(AudioCodec::LPCM);
  config.setExportFile(kOutPath.string());
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.prepareToPlay(48000, 64);

  config.setManualExport(true);
  fileExportRepository.update(config);
  processor.runPendingTasks();

  const int kNumChannels = static_cast<int>(kCase.expectedSources.size());
  juce::AudioBuffer<float> buffer(kNumChannels, 64);
  for (int ch = 0; ch < kNumChannels; ++ch) {
    juce::FloatVectorOperations::fill(buffer.getWritePointer(ch), ituMarker(ch),
                                      buffer.getNumSamples());
  }
  juce::MidiBuffer midi;
  processor.processBlock(buffer, midi);

  config = fileExportRepository.get();
  config.setManualExport(false);
  fileExportRepository.update(config);
  processor.runPendingTasks();
  EXPECT_EQ(fileExportRepository.get().getExportError(), ExportError::kNoError);

  auto reader = readWav(kOutPath);
  ASSERT_NE(reader, nullptr);
  ASSERT_EQ(static_cast<int>(reader->numChannels), kNumChannels);
  EXPECT_EQ(reader->getChannelLayout(), makeSet(kCase.expectedSet));
  juce::AudioBuffer<float> readBack(kNumChannels, 64);
  reader->read(&readBack, 0, 64, 0, true, true);
  for (int ch = 0; ch < kNumChannels; ++ch) {
    EXPECT_NEAR(readBack.getSample(ch, 10),
                ituMarker(kCase.expectedSources[ch]), 1e-4f)
        << "written channel " << ch;
  }
  reader.reset();
  std::filesystem::remove(kOutPath);
}

INSTANTIATE_TEST_SUITE_P(
    RoomLayouts, WavFileOutputRoomOrder,
    ::testing::Values(
        RoomWavCase{Speakers::kStereo, {L, R}, {0, 1}},
        RoomWavCase{Speakers::k3Point1Point2,
                    {L, R, C, LFE, Ltf, Rtf},
                    {0, 1, 2, 3, 4, 5}},
        RoomWavCase{
            Speakers::k5Point1, {L, R, C, LFE, Ls, Rs}, {0, 1, 2, 3, 4, 5}},
        RoomWavCase{Speakers::k5Point1Point2,
                    {L, R, C, LFE, Ls, Rs, Ltf, Rtf},
                    {0, 1, 2, 3, 4, 5, 6, 7}},
        RoomWavCase{Speakers::k5Point1Point4,
                    {L, R, C, LFE, Ls, Rs, Ltf, Rtf, Ltr, Rtr},
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}},
        RoomWavCase{Speakers::k7Point1,
                    {L, R, C, LFE, Ls, Rs, Lss, Rss},
                    {0, 1, 2, 3, 6, 7, 4, 5}},
        RoomWavCase{Speakers::k7Point1Point2,
                    {L, R, C, LFE, Ls, Rs, Lss, Rss, Ltf, Rtf},
                    {0, 1, 2, 3, 6, 7, 4, 5, 8, 9}},
        RoomWavCase{Speakers::k7Point1Point4,
                    {L, R, C, LFE, Ls, Rs, Lss, Rss, Ltf, Rtf, Ltr, Rtr},
                    {0, 1, 2, 3, 6, 7, 4, 5, 8, 9, 10, 11}}),
    [](const ::testing::TestParamInfo<RoomWavCase>& info) {
      return info.param.layout.toString()
          .replaceCharacter('.', 'p')
          .retainCharacters(
              "0123456789abcdefghijklmnopqrstuvwxyz"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZp")
          .toStdString();
    });

// FileWriter writes the WAVE-mapped 5.1.2, 7.1, 7.1.2 and 7.1.4 sets, and a
// reader recovers each set from the file's channel mask.
TEST(WavFileOutputFileWriter, wave_mapped_sets_read_back_from_channel_mask) {
  const std::vector<std::vector<ChannelType>> kSets = {
      {L, R, C, LFE, Ls, Rs, Ltf, Rtf},
      {L, R, C, LFE, Ls, Rs, Lss, Rss},
      {L, R, C, LFE, Ls, Rs, Lss, Rss, Ltf, Rtf},
      {L, R, C, LFE, Ls, Rs, Lss, Rss, Ltf, Rtf, Ltr, Rtr}};
  for (const auto& kTypes : kSets) {
    const juce::AudioChannelSet kSet = makeSet(kTypes);
    const std::filesystem::path kOutPath =
        std::filesystem::current_path() /
        ("wav_mask_" + std::to_string(kSet.size()) + ".wav");
    std::filesystem::remove(kOutPath);
    {
      FileWriter writer(kOutPath.string(), 48000, kSet, 0, 24,
                        AudioCodec::LPCM);
      ASSERT_TRUE(writer.isOpen()) << kSet.getDescription();
      juce::AudioBuffer<float> buffer(kSet.size(), 64);
      buffer.clear();
      EXPECT_TRUE(writer.write(buffer));
      EXPECT_TRUE(writer.close());
    }
    auto reader = readWav(kOutPath);
    ASSERT_NE(reader, nullptr);
    EXPECT_EQ(reader->getChannelLayout(), kSet) << kSet.getDescription();
    reader.reset();
    std::filesystem::remove(kOutPath);
  }
}

// FileWriter::write refuses a buffer with fewer channels than
// firstChannel + numChannels instead of reading past it.
TEST(WavFileOutputFileWriter,
     write_rejects_buffer_narrower_than_channel_range) {
  const std::filesystem::path kOutPath =
      std::filesystem::current_path() / "wav_narrow_buffer.wav";
  std::filesystem::remove(kOutPath);
  FileWriter writer(kOutPath.string(), 48000, juce::AudioChannelSet::stereo(),
                    1, 24, AudioCodec::LPCM);
  ASSERT_TRUE(writer.isOpen());
  juce::AudioBuffer<float> buffer(2, 64);
  buffer.clear();
  EXPECT_FALSE(writer.write(buffer));
  EXPECT_EQ(writer.getFramesWritten(), 0);
  writer.close();
  std::filesystem::remove(kOutPath);
}
