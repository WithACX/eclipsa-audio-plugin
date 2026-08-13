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

#include "PerspectiveRoomViews.h"

#include <cmath>
#include <vector>

#include "data_structures/src/RepositoryCollection.h"

namespace {

// The two shades of the room-view translucent grey the rear projection
// established. A pattern that presents more than one surface uses both, so
// adjacent surfaces read as separate planes instead of merging into one panel.
juce::Colour recedingSurface() {
  return EclipsaColours::roomviewTranslucentWall.brighter(0.2f);
}
juce::Colour facingSurface() {
  return EclipsaColours::roomviewTranslucentWall.brighter();
}

// Close a run of 3D anchor vertices into a window-space path with straight
// edges. Every elevation surface is built from anchors put through
// Coordinates::toWindow, so resizing the plugin window keeps the surface
// aligned to the drawn room instead of to a pixel constant.
juce::Path anchorsToPath(const Coordinates::Mat4& transformMat,
                         const Coordinates::WindowData& window,
                         const std::vector<Coordinates::Point4D>& anchors) {
  juce::Path path;
  if (anchors.empty()) {
    return path;
  }
  const Coordinates::Point2D start =
      Coordinates::toWindow(transformMat, window, anchors.front());
  path.startNewSubPath(start.a[0], start.a[1]);
  for (size_t i = 1; i < anchors.size(); ++i) {
    const Coordinates::Point2D pt =
        Coordinates::toWindow(transformMat, window, anchors[i]);
    path.lineTo(pt.a[0], pt.a[1]);
  }
  path.closeSubPath();
  return path;
}

// Stitch two sampled edges of one curved surface -- the left edge and the
// right edge of the room, sampled at the same front/back positions -- into a
// single filled surface spanning left to right. quadraticTo is kept from the
// rear projection's arch so a sampled edge stays smooth rather than showing
// its facets.
juce::Path sampledEdgesToPath(
    const Coordinates::Mat4& transformMat,
    const Coordinates::WindowData& window,
    const std::vector<Coordinates::Point4D>& leftEdge,
    const std::vector<Coordinates::Point4D>& rightEdge) {
  juce::Path path;
  if (leftEdge.size() < 3 || leftEdge.size() != rightEdge.size()) {
    return path;
  }

  std::vector<Coordinates::Point2D> left, right;
  left.reserve(leftEdge.size());
  right.reserve(rightEdge.size());
  for (size_t i = 0; i < leftEdge.size(); ++i) {
    left.push_back(Coordinates::toWindow(transformMat, window, leftEdge[i]));
    right.push_back(Coordinates::toWindow(transformMat, window, rightEdge[i]));
  }

  const size_t kLast = left.size() - 1;
  path.startNewSubPath(left[0].a[0], left[0].a[1]);
  for (size_t i = 1; i <= kLast - 1; ++i) {
    path.quadraticTo(left[i].a[0], left[i].a[1], left[i + 1].a[0],
                     left[i + 1].a[1]);
  }
  // Span the room at the far front/back bound, then walk the right edge back.
  path.lineTo(right[kLast].a[0], right[kLast].a[1]);
  for (size_t i = kLast - 1; i >= 1; --i) {
    path.quadraticTo(right[i].a[0], right[i].a[1], right[i - 1].a[0],
                     right[i - 1].a[1]);
  }
  // Closing the subpath spans the room at the near bound.
  path.closeSubPath();
  return path;
}

}  // namespace

TopView::TopView(const SpeakerMonitorData& monitorData,
                 RepositoryCollection repos)
    : PerspectiveRoomView(
          FaceLookup::getFaces(FaceLookup::kTop),
          Coordinates::getTopViewTransform(), {SpeakerLookup::kLFE},
          IconStore::getInstance().getTopIcon(), monitorData, repos) {};

const float TopView::getTrackScaling(const Coordinates::Point4D pt) const {
  return 0.35 * pt.a[FaceLookup::kAxisY] + 1.35;
};

SideView::SideView(const SpeakerMonitorData& monitorData,
                   RepositoryCollection repos)
    : PerspectiveRoomView(
          FaceLookup::getFaces(FaceLookup::kSide),
          Coordinates::getSideViewTransform(),
          {SpeakerLookup::kLS, SpeakerLookup::kLSS, SpeakerLookup::kLRS,
           SpeakerLookup::kLTR, SpeakerLookup::kLFE, SpeakerLookup::kFL,
           SpeakerLookup::kSIL},
          IconStore::getInstance().getLeftIcon(), monitorData, repos) {};

const float SideView::getTrackScaling(const Coordinates::Point4D pt) const {
  return -0.35 * pt.a[FaceLookup::kAxisX] + 1.35;
};

RearView::RearView(const SpeakerMonitorData& monitorData,
                   RepositoryCollection repos)
    : PerspectiveRoomView(
          FaceLookup::getFaces(FaceLookup::kRear),
          Coordinates::getRearViewTransform(),
          {SpeakerLookup::kLTB, SpeakerLookup::kRTB, SpeakerLookup::kLFE,
           SpeakerLookup::kTPBL, SpeakerLookup::kTPBR, SpeakerLookup::kBL,
           SpeakerLookup::kBR},
          IconStore::getInstance().getBackIcon(), monitorData, repos) {};

const float RearView::getTrackScaling(const Coordinates::Point4D pt) const {
  return 0.35 * pt.a[FaceLookup::kAxisZ] + 1.35;
};

IsoView::IsoView(const SpeakerMonitorData& monitorData,
                 RepositoryCollection repos)
    : PerspectiveRoomView(
          FaceLookup::getFaces(FaceLookup::kIso),
          Coordinates::getIsoViewTransform(), {SpeakerLookup::kLFE},
          IconStore::getInstance().getIsoIcon(), monitorData, repos) {};

void IsoView::drawFace(const std::array<Coordinates::Point2D, 4>& faceVerts,
                       const juce::Colour& c, juce::Graphics& g) {
  // Only draw outlines for non-transparent faces.
  if (c.getAlpha() == 1.f) {
    g.setColour(EclipsaColours::backgroundOffBlack);
    drawLine(faceVerts[0], faceVerts[1], g);
    drawLine(faceVerts[1], faceVerts[2], g);
    drawLine(faceVerts[2], faceVerts[3], g);
    drawLine(faceVerts[3], faceVerts[0], g);
  } else {
    // Draw one line for transparent faces to indicate where they join.
    g.setColour(EclipsaColours::backgroundOffBlack);
    drawLine(faceVerts[1], faceVerts[2], g, 1.f);
  }

  // Fill the face.
  juce::Path facePath;
  facePath.startNewSubPath(faceVerts[0].a[0], faceVerts[0].a[1]);
  facePath.lineTo(faceVerts[1].a[0], faceVerts[1].a[1]);
  facePath.lineTo(faceVerts[2].a[0], faceVerts[2].a[1]);
  facePath.lineTo(faceVerts[3].a[0], faceVerts[3].a[1]);
  facePath.closeSubPath();
  g.setColour(c);
  g.fillPath(facePath);
}

const float IsoView::getTrackScaling(const Coordinates::Point4D pt) const {
  return 1.35f;
};
AudioElementPluginTopView::AudioElementPluginTopView(
    const SpeakerMonitorData& monitorData)
    : PerspectiveRoomView(FaceLookup::getFaces(FaceLookup::kTop),
                          Coordinates::getTopViewTransform(),
                          {SpeakerLookup::kLFE}, {}, monitorData) {};

const float AudioElementPluginTopView::getTrackScaling(
    const Coordinates::Point4D pt) const {
  // Under a top-down projection the depth axis is NDC up, not NDC z.
  return 0.35 * pt.a[FaceLookup::kAxisY] + 1.35;
}

void AudioElementPluginTopView::drawTrack(const DrawableTrack& track,
                                          juce::Graphics& g) {
  // Determine the size of the outer track volume indicator based on the
  // loudness. Only draw if the track is not silent.
  const juce::Colour kTrackColour = getLoudnessColour(track.trackLoudness);
  float width;
  if (kTrackColour != EclipsaColours::speakerSilentFill) {
    float sf2 = 6 * (1 - std::abs(track.trackLoudness) / 60.f);
    width = 14.f * track.sizeScale * sf2;
    g.setColour(kTrackColour.withAlpha(0.5f));
    g.fillEllipse(track.pos.a[0] - width / 2, track.pos.a[1] - width / 2, width,
                  width);
  }

  // The panned audio element track center is blue independent of loudness.
  g.setColour(EclipsaColours::controlBlue);
  width = 14.f * track.sizeScale;
  g.fillEllipse(track.pos.a[0] - width / 2, track.pos.a[1] - width / 2, width,
                width);
}

void AudioElementPluginTopView::setElevationPattern(
    AudioElementSpatialLayout::Elevation elevation) {
  currentElevation_ = elevation;
}

void AudioElementPluginTopView::paint(juce::Graphics& g) {
  Coordinates::WindowData wData = {
      .leftCornerX = 0.0f,
      .bottomCornerY = (float)getHeight(),
      .width = (float)getWidth(),
      .height = (float)getHeight(),
  };

  PerspectiveRoomView::paint(g);

  switch (currentElevation_) {
    case AudioElementSpatialLayout::Elevation::kFlat:
      paintFlatElevation(wData, g);
      break;
    case AudioElementSpatialLayout::Elevation::kTent:
      paintTentElevation(wData, g);
      break;
    case AudioElementSpatialLayout::Elevation::kArch:
      paintArchElevation(wData, g);
      break;
    case AudioElementSpatialLayout::Elevation::kDome:
      paintDomeElevation(wData, g);
      break;
    case AudioElementSpatialLayout::Elevation::kCurve:
      paintCurveElevation(wData, g);
      break;
    default:
      break;
  }

  if (!transformedTracks_.empty()) {
    drawTrack(transformedTracks_[0], g);
  }
}

// The five elevation painters below are dispatched from paint above.
//
// The top view is not an orthographic plan view: getTopViewTransform() is a
// 45-degree perspective with the camera rotated 90 degrees about X, so its
// w = 5 - y. Height therefore scales the projection, and an elevation pattern
// reads as a 3D surface seen from above rather than as a flat outline -- which
// is why only Flat is a single panel here, and why surfaces the rear
// projection could omit as occluded are visible and must be drawn.
//
// Heights come from the statics in Elevation.h; no elevation curve is
// re-derived here. Those helpers take the front/back coordinate in a[1] and
// return the height in a[1] (see the axis note in Elevation.h).

void AudioElementPluginTopView::paintFlatElevation(
    const Coordinates::WindowData& window, juce::Graphics& g) {
  // The one pattern that is genuinely a single plane: a panel over the floor at
  // the height set through setFlatHeight. Height is indicated by the uniform
  // tint alone.
  const std::vector<Coordinates::Point4D> kFlatAnchors = {
      {-1.f, currentFlatHeight_, -1.f, 1.f},
      {1.f, currentFlatHeight_, -1.f, 1.f},
      {1.f, currentFlatHeight_, 1.f, 1.f},
      {-1.f, currentFlatHeight_, 1.f, 1.f},
  };

  g.setColour(facingSurface());
  g.fillPath(anchorsToPath(kTransformMat_, window, kFlatAnchors));
}

void AudioElementPluginTopView::paintTentElevation(
    const Coordinates::WindowData& window, juce::Graphics& g) {
  // Two sloping planes meeting at the ridge, off the rear projection's six
  // anchors: floor corners at (+/-1, -1, -/+1) and ridge points at
  // (+/-1, 1, 0). The rear projection drew only the front slope because the
  // back slope was occluded; from above both are visible.
  const Coordinates::Point4D kFrontFloorLeft = {-1.f, -1.f, -1.f, 1.f};
  const Coordinates::Point4D kFrontFloorRight = {1.f, -1.f, -1.f, 1.f};
  const Coordinates::Point4D kRidgeLeft = {-1.f, 1.f, 0.f, 1.f};
  const Coordinates::Point4D kRidgeRight = {1.f, 1.f, 0.f, 1.f};
  const Coordinates::Point4D kBackFloorRight = {1.f, -1.f, 1.f, 1.f};
  const Coordinates::Point4D kBackFloorLeft = {-1.f, -1.f, 1.f, 1.f};

  // Shade the two planes differently so the shared ridge reads as an edge
  // rather than the pair merging into one panel.
  g.setColour(recedingSurface());
  g.fillPath(anchorsToPath(
      kTransformMat_, window,
      {kBackFloorLeft, kBackFloorRight, kRidgeRight, kRidgeLeft}));

  g.setColour(facingSurface());
  g.fillPath(anchorsToPath(
      kTransformMat_, window,
      {kFrontFloorLeft, kFrontFloorRight, kRidgeRight, kRidgeLeft}));
}

void AudioElementPluginTopView::paintArchElevation(
    const Coordinates::WindowData& window, juce::Graphics& g) {
  // A curved surface, not a silhouette edge: sample the parabola along the
  // left and right room edges at the same front/back positions, then stitch
  // the two sampled edges into one filled surface spanning left to right.
  const int kNumSamples = 41;
  std::vector<Coordinates::Point4D> leftEdge, rightEdge;
  leftEdge.reserve(kNumSamples);
  rightEdge.reserve(kNumSamples);
  for (int i = 0; i < kNumSamples; ++i) {
    const float offset = i * 0.05f;
    const float frontBack = -1.f + offset;
    const float height =
        ElevationListener::getArchElevationPt({-1.f, frontBack, 0.f}).a[1];
    leftEdge.push_back({-1.f, height, frontBack, 1.f});
    rightEdge.push_back({1.f, height, frontBack, 1.f});
  }

  g.setColour(facingSurface());
  g.fillPath(sampledEdgesToPath(kTransformMat_, window, leftEdge, rightEdge));
}

void AudioElementPluginTopView::paintDomeElevation(
    const Coordinates::WindowData& window, juce::Graphics& g) {
  // A filled circle marking the dome's outer bound -- the region
  // getDomeElevationPtClamped clamps x and y into -- so the circle shows where
  // the source can actually sit. Sampling the boundary through that same
  // function is what ties the radius to the clamp rather than to a constant:
  // on the unit circle it returns the sampled (x, y) unclamped at floor
  // height.
  const int kNumSamples = 81;
  std::vector<Coordinates::Point4D> boundAnchors;
  boundAnchors.reserve(kNumSamples);
  for (int i = 0; i < kNumSamples; ++i) {
    const float theta =
        i * juce::MathConstants<float>::twoPi / (kNumSamples - 1);
    const Coordinates::Point3D boundPt =
        ElevationListener::getDomeElevationPtClamped(
            {std::cos(theta), std::sin(theta), 0.f}, {});
    boundAnchors.push_back({boundPt.a[0], boundPt.a[1], boundPt.a[2], 1.f});
  }

  // The bound sits on the floor, below the source, so it takes the receding
  // shade.
  g.setColour(recedingSurface());
  g.fillPath(anchorsToPath(kTransformMat_, window, boundAnchors));
}

void AudioElementPluginTopView::paintCurveElevation(
    const Coordinates::WindowData& window, juce::Graphics& g) {
  // The same treatment as Arch, against the logarithmic curve, at the existing
  // sample count.
  //
  // Sign convention: ElevationListener negates the front/back POSITION
  // PARAMETER before calling getCurveElevationPt, and room-view NDC z is that
  // same parameter negated (Coordinates::toRoomNdc). The value the listener
  // passes is therefore the NDC front/back coordinate itself, so sampling at
  // frontBack un-negated is what makes the drawn surface agree with the height
  // the listener derives. The curve is not symmetric about 0, so this sign
  // matters here where it does not for Tent, Arch, or Dome.
  const int kNumSamples = 32;
  std::vector<Coordinates::Point4D> leftEdge, rightEdge;
  leftEdge.reserve(kNumSamples);
  rightEdge.reserve(kNumSamples);
  for (int i = 0; i < kNumSamples; ++i) {
    const float frontBack = -1.f + i * 2.f / (kNumSamples - 1);
    const float height =
        ElevationListener::getCurveElevationPt({-1.f, frontBack, 0.f}).a[1];
    leftEdge.push_back({-1.f, height, frontBack, 1.f});
    rightEdge.push_back({1.f, height, frontBack, 1.f});
  }

  g.setColour(facingSurface());
  g.fillPath(sampledEdgesToPath(kTransformMat_, window, leftEdge, rightEdge));
}
