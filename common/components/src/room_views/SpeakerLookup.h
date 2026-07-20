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
#include "Coordinates.h"
#include "substream_rdr/substream_rdr_utils/Speakers.h"

/**
 * @brief Describes indvidual speakers in a room view. This includes a speaker's
 * label and location in Normalized Device Coordinates (NDC) which range from -1
 * to 1. Different speaker sets are queryable by AudioElementSpeakerLayout.
 *
 */
namespace SpeakerLookup {
enum SpeakerTag {
  kLB,
  kRB,
  kL,
  kR,
  kC,
  kLFE,
  kLS,
  kRS,
  kLSS,
  kRSS,
  kLRS,
  kRRS,
  kLTR,
  kRTR,
  kLTF,
  kRTF,
  kLTB,
  kRTB,
  kFL,
  kFR,
  kFC,
  kBL,
  kBR,
  kFLC,
  kFRC,
  kSIL,
  kSIR,
  kTPFL,
  kTPFR,
  kTPBL,
  kTPBR,
  kTPSIL,
  kTPSIR,
};
struct RoomViewSpeaker {
  Coordinates::Point4D pos;
  std::string name;
  SpeakerTag tag;
};

const RoomViewSpeaker kLeftBinaural{
    Coordinates::Point4D{{-0.12f, 0.08f, 0.02f, 1.0f}},
    "LB",
    kLB,
};
const RoomViewSpeaker kRightBinuaral{
    Coordinates::Point4D{{0.12f, 0.08f, 0.02f, 1.0f}},
    "RB",
    kRB,
};

const RoomViewSpeaker kLeft{
    Coordinates::Point4D{
        {-0.5f, 0.0f, -0.866f, 1.0f}},  // X = -sin(30), Y = cos(30)
    "L",
    kL,
};
const RoomViewSpeaker kRight{
    Coordinates::Point4D{{0.5f, 0.0f, -0.866f, 1.0f}},
    "R",
    kR,
};
const RoomViewSpeaker kCentre{
    Coordinates::Point4D{{0.0f, 0.0f, -1.0f, 1.0f}},
    "C",
    kC,
};
const RoomViewSpeaker kLeftSurround{
    Coordinates::Point4D{{-0.94f, 0.0f, 0.342f, 1.0f}},
    "Ls",
    kLS,
};
const RoomViewSpeaker kRightSurround{
    Coordinates::Point4D{{0.94f, 0.0f, 0.342f, 1.0f}},
    "Rs",
    kRS,
};
const RoomViewSpeaker kLeftSideSurround{
    Coordinates::Point4D{{-1.f, 0.0f, 0.f, 1.0f}},
    "Lss",
    kLSS,
};
const RoomViewSpeaker kRightSideSurround{
    Coordinates::Point4D{{1.f, 0.0f, 0.f, 1.0f}},
    "Rss",
    kRSS,
};
const RoomViewSpeaker kLeftRearSurround{
    Coordinates::Point4D{{-0.707f, 0.0f, 0.707f, 1.0f}},
    "Lrs",
    kLRS,
};
const RoomViewSpeaker kRightRearSurround{
    Coordinates::Point4D{{0.707f, 0.0f, 0.707f, 1.0f}},
    "Rrs",
    kRRS,

};
const RoomViewSpeaker kLeftTopRear{
    Coordinates::Point4D{{-.94f, 0.5f, 0.342f, 1.0f}},
    "Ltr",
    kLTR,
};
const RoomViewSpeaker kRightTopRear{
    Coordinates::Point4D{{0.94f, 0.5f, 0.342f, 1.0f}},
    "Rtr",
    kRTR,
};
const RoomViewSpeaker kLeftTopFront{
    Coordinates::Point4D{{-0.5f, 0.5f, -0.866f, 1.0f}},
    "Ltf",
    kLTF,
};
const RoomViewSpeaker kRightTopFront{
    Coordinates::Point4D{{0.5f, 0.5f, -0.866f, 1.0f}},
    "Rtf",
    kRTF,
};

// U+045 replaces U+030 in 7.1.4
const RoomViewSpeaker kLU045{
    Coordinates::Point4D{{-0.707f, 0.5f, -0.707f, 1.0f}},
    "Ltf",
    kLTF,
};
// U-045 replaces U+030 in 7.1.4
const RoomViewSpeaker kRU045{
    Coordinates::Point4D{{0.707f, 0.5f, -0.707f, 1.0f}},
    "Rtf",
    kRTF,
};

const RoomViewSpeaker kLeftTopBack{
    Coordinates::Point4D{{-0.707f, 0.5f, .707f, 1.0f}},
    "Ltr",
    kLTB,
};
const RoomViewSpeaker kRightTopBack{
    Coordinates::Point4D{{0.707f, 0.5f, .707f, 1.0f}},
    "Rtr",
    kRTB,
};
const RoomViewSpeaker kLowFreqEffects{
    Coordinates::Point4D{{0.0f, 0.0f, 0.0f, 1.0f}},
    "LFE",
    kLFE,
};
const RoomViewSpeaker kFrontLeft{
    Coordinates::Point4D{{-0.866f, 0.0f, -0.5f, 1.0f}},
    "Fl",
    kFL,
};
const RoomViewSpeaker kFrontRight{
    Coordinates::Point4D{{0.866f, 0.0f, -0.5f, 1.0f}},
    "Fr",
    kFR,
};
const RoomViewSpeaker kFrontCentre{
    Coordinates::Point4D{{0.0f, 0.0f, -1.0f, 1.0f}},
    "Fc",
    kFC,
};
const RoomViewSpeaker kBackLeft{
    Coordinates::Point4D{{-0.707f, 0.0f, 0.707f, 1.0f}},
    "Bl",
    kBL,
};
const RoomViewSpeaker kBackRight{
    Coordinates::Point4D{{0.707f, 0.0f, 0.707, 1.0f}},
    "Br",
    kBR,
};
const RoomViewSpeaker kFrontLeftCentre{
    Coordinates::Point4D{{-0.5f, 0.0f, -0.866f, 1.0f}},
    "Flc",
    kFLC,
};
const RoomViewSpeaker kFrontRightCentre{
    Coordinates::Point4D{{0.5f, 0.0f, -0.866f, 1.0f}},
    "Frc",
    kFRC,
};
const RoomViewSpeaker kSideLeft{
    Coordinates::Point4D{{-1.0f, 0.0f, 0.f, 1.0f}},
    "SiL",
    kSIL,
};
const RoomViewSpeaker kSideRight{
    Coordinates::Point4D{{1.0f, 0.0f, 0.f, 1.0f}},
    "SiR",
    kSIR,
};
const RoomViewSpeaker kTopFrontLeft{
    Coordinates::Point4D{{-0.707f, 0.5f, -0.707f, 1.0f}},
    "TpFl",
    kTPFL,
};
const RoomViewSpeaker kTopFrontRight{
    Coordinates::Point4D{{0.707f, 0.5f, -0.707f, 1.0f}},
    "TpFr",
    kTPFR,
};
const RoomViewSpeaker kTopBackLeft{
    Coordinates::Point4D{{-0.707f, 0.5f, 0.707f, 1.0f}},
    "TpBL",
    kTPBL,
};
const RoomViewSpeaker kTopBackRight{
    Coordinates::Point4D{{0.707f, 0.5f, 0.707f, 1.0f}},
    "TpBr",
    kTPBR,
};
const RoomViewSpeaker kTopSideLeft{
    Coordinates::Point4D{{-1.0f, 0.5f, 0.f, 1.0f}},
    "TpSiL",
    kTPSIL,
};
const RoomViewSpeaker kTopSideRight{
    Coordinates::Point4D{{1.0f, 0.5f, 0.f, 1.0f}},
    "TpSiR",
    kTPSIR,
};

// Returns a room view speaker set for a given speaker layout.
inline std::vector<RoomViewSpeaker> getRoomViewSpeakers(
    const Speakers::AudioElementSpeakerLayout layout) {
  using namespace Speakers;
  switch (layout) {
    case kMono:
      return {kCentre};
    case kStereo:
      return {
          kLeft,
          kRight,
      };
    case k3Point1Point2:
      return {kLeft,         kRight,         kCentre,        kLowFreqEffects,
              kLeftTopFront, kRightTopFront, kLowFreqEffects};
    case k5Point1:
      return {
          kLeft,           kRight,        kCentre,
          kLowFreqEffects, kLeftSurround, kRightSurround,
      };
    case k5Point1Point2:
      return {
          kLeft,         kRight,         kCentre,       kLowFreqEffects,
          kLeftSurround, kRightSurround, kLeftTopFront, kRightTopFront,
      };
    case k5Point1Point4:
      return {
          kLeft,         kRight,         kCentre,       kLowFreqEffects,
          kLeftSurround, kRightSurround, kLeftTopFront, kRightTopFront,
          kLeftTopRear,  kRightTopRear,
      };
    case k7Point1:
      return {
          kLeft,
          kRight,
          kCentre,
          kLowFreqEffects,
          kLeftSideSurround,
          kRightSideSurround,
          kLeftRearSurround,
          kRightRearSurround,
      };
    case k7Point1Point2:
      return {
          kLeft,
          kRight,
          kCentre,
          kLowFreqEffects,
          kLeftSideSurround,
          kRightSideSurround,
          kLeftRearSurround,
          kRightRearSurround,
          kLeftTopFront,
          kRightTopFront,
      };
    case k7Point1Point4:
      return {
          kLeft,
          kRight,
          kCentre,
          kLowFreqEffects,
          kLeftSideSurround,
          kRightSideSurround,
          kLeftRearSurround,
          kRightRearSurround,
          kLU045,
          kRU045,
          kLeftTopBack,
          kRightTopBack,
      };
    case kBinaural:
      return {kLeftBinaural, kRightBinuaral};
    case kExpl5Point1Point4Surround:
      return {
          kLeftSurround,
          kRightSurround,
      };
    case kExpl7Point1Point4SideSurround:
      return {
          kLeftSideSurround,
          kRightSideSurround,
      };
    case kExpl7Point1Point4RearSurround:
      return {
          kLeftRearSurround,
          kRightRearSurround,
      };
    case kExpl7Point1Point4TopFront:
      return {
          kLU045,
          kRU045,
      };
    case kExpl7Point1Point4TopBack:
      return {
          kLeftTopBack,
          kRightTopBack,
      };
    case kExpl7Point1Point4Top:
      return {
          kLU045,
          kRU045,
          kLeftTopBack,
          kRightTopBack,
      };
    case kExpl7Point1Point4Front:
      return {
          kLeft,
          kRight,
          kCentre,
      };
    case kExpl9Point1Point6:
      return {
          kFrontLeft,   kFrontRight,   kFrontLeftCentre, kLowFreqEffects,
          kBackLeft,    kBackRight,    kFrontLeftCentre, kFrontRightCentre,
          kSideLeft,    kSideRight,    kTopFrontLeft,    kTopFrontRight,
          kTopBackLeft, kTopBackRight, kTopSideLeft,     kTopSideRight,
      };
    case kExpl9Point1Point6Front:
      return {
          kFrontLeft,
          kFrontRight,
      };
    case kExpl9Point1Point6Side:
      return {
          kSideLeft,
          kSideRight,
      };
    case kExpl9Point1Point6TopSide:
      return {
          kTopSideLeft,
          kTopSideRight,
      };
    case kExpl9Point1Point6Top:
      return {
          kTopFrontLeft, kTopFrontRight, kTopBackLeft,
          kTopBackRight, kTopSideLeft,   kTopSideRight,
      };
    default:
      return {};
  }
}
}  // namespace SpeakerLookup