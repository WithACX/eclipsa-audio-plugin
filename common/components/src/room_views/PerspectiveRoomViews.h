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
#include <vector>

#include "PerspectiveRoomView.h"
#include "components/src/room_views/HeightIndicator.h"
#include "data_structures/src/AudioElementParameterTree.h"
#include "data_structures/src/AudioElementSpatialLayout.h"
#include "data_structures/src/Elevation.h"
#include "data_structures/src/RepositoryCollection.h"

class TopView : public PerspectiveRoomView {
 public:
  TopView(const SpeakerMonitorData& monitorData, RepositoryCollection repos);
  const float getTrackScaling(const Coordinates::Point4D pt) const override;
};

class SideView : public PerspectiveRoomView {
 public:
  SideView(const SpeakerMonitorData& monitorData, RepositoryCollection repos);
  const float getTrackScaling(const Coordinates::Point4D pt) const override;
};

class RearView : public PerspectiveRoomView {
 public:
  RearView(const SpeakerMonitorData& monitorData, RepositoryCollection repos);
  const float getTrackScaling(const Coordinates::Point4D pt) const override;
};

class IsoView : public PerspectiveRoomView {
 public:
  IsoView(const SpeakerMonitorData& monitorData, RepositoryCollection repos);
  void drawFace(const std::array<Coordinates::Point2D, 4>& faceVerts,
                const juce::Colour& c, juce::Graphics& g) override;
  const float getTrackScaling(const Coordinates::Point4D pt) const override;
};

// The audio element plugin's panner: a top-down (plan) projection of the room,
// with left/right on the horizontal screen axis and front/back on the
// vertical. Replaces the rear projection the panner used previously.
class AudioElementPluginTopView : public PerspectiveRoomView {
 public:
  AudioElementPluginTopView(const SpeakerMonitorData& monitorData);
  void paint(juce::Graphics& g) override;
  const float getTrackScaling(const Coordinates::Point4D pt) const override;
  void drawTrack(const DrawableTrack& track, juce::Graphics& g) override;
  void setElevationPattern(AudioElementSpatialLayout::Elevation elevation);
  void setFlatHeight(float height) {
    currentFlatHeight_ = Coordinates::toRoomNdc(0.f, 0.f, height).a[1];
  }

  // The panner writes the position parameters, so it needs the tree. A setter
  // rather than a constructor argument: the sibling views take only
  // SpeakerMonitorData, and this view stays constructible and paintable
  // without a tree -- it simply does not respond to input -- so it is no
  // harder to build than they are. PAN-02.2 and PAN-02.3 reuse this wiring
  // rather than adding their own.
  void setParameterTree(AudioElementParameterTree* tree) {
    parameterTree_ = tree;
  }

  // The first input handling in any room view. Drag only: the wheel and
  // keyboard belong to PAN-02.2, and a speaker click to PAN-02.3.
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

 private:
  // The window the room is currently projected into. One owner, so paint and
  // the drag's inverse projection can never disagree about the mapping.
  Coordinates::WindowData currentWindow() const;
  bool sourceMarkerContains(const juce::Point<float>& windowPoint) const;
  // The RangedAudioParameter behind a position parameter name. Gesture
  // bracketing needs the parameter itself; AudioElementParameterTree's
  // setters go through getParameterAsValue and emit no gesture markers.
  juce::RangedAudioParameter* positionParameter(
      const juce::String& parameterName) const;
  void writeDragPosition(const juce::Point<float>& windowPoint);
  float elevationHeightAt(float leftRight, float frontBack) const;
  Coordinates::Point4D indicatorPosition(
      const Coordinates::Point4D& sourceNdc) const;
  bool elevationClampsTheSource() const;
  bool elevationVariesAcrossLeftRight() const;
  void paintIndicatorRuns(const Coordinates::WindowData& window,
                          const std::vector<HeightIndicator::Segment>& runs,
                          float thickness, juce::Graphics& g);
  void paintFlatElevation(const Coordinates::WindowData& window,
                          juce::Graphics& g);
  void paintTentElevation(const Coordinates::WindowData& window,
                          juce::Graphics& g);
  void paintArchElevation(const Coordinates::WindowData& window,
                          juce::Graphics& g);
  void paintDomeElevation(const Coordinates::WindowData& window,
                          juce::Graphics& g);
  void paintCurveElevation(const Coordinates::WindowData& window,
                           juce::Graphics& g);

 private:
  // Initialized here, not left to the constructor: paint() reads both on the
  // first repaint, which happens before RoomViewScreen calls
  // setElevationPattern or setFlatHeight.
  AudioElementSpatialLayout::Elevation currentElevation_ =
      AudioElementSpatialLayout::Elevation::kNone;
  float currentFlatHeight_ = 0.f;

  // Null until RoomViewScreen wires it. Input handling is inert without it,
  // which is what keeps the tree optional.
  AudioElementParameterTree* parameterTree_ = nullptr;
  // True only between a press that landed on the source and its release, so
  // the gesture bracket can never outlive the drag it belongs to, and a press
  // that missed the source can never start writing.
  bool draggingSource_ = false;
};
