---
id: elevation-surfaces-grep-strategy
trigger: "when scoping changes to elevation surface rendering or room view logic"
confidence: 0.80
domain: code-style
scope: project
date: 2026-08-13
---

# Elevation Surfaces: Systematic Grep Strategy for Room Views

## Action
When reviewing or planning changes to elevation surfaces (Flat, Tent, Arch, Dome, Curve), use a two-stage grep strategy to map the rendering pipeline: surface definition → painter functions → math library → icon assets.

## Evidence
- Observed 3 times in observer analysis (aut-20260813-56):
  - Line 19: `git ls-files | grep -iE 'elevation|room_view'` located all related files
  - Line 23: `grep -n 'Elevation\|Surface\|roomviewTranslucentWall'` found 80+ matches across painters
  - Line 27-28: `grep -n -A12 'getArchElevationPt\|getCurveElevationPt'` verified math function signatures
- Same exploration pattern used in prior elevation work (commit 152beae feat(room_views))
- Successfully located icon assets, headers, test files, implementation files in one systematic pass

## Why
Elevation surfaces have three distinct layers:
1. **Geometry definitions**: ElevationSurfaces.h (anchorsToPath, sampledEdgesToPath)
2. **Painters**: PerspectiveRoomViews.cpp (paintFlatElevation, paintTentElevation, etc., dispatched from setElevationPattern)
3. **Math library**: data_structures/src/Elevation.h (getArchElevationPt, getDomeElevationPtClamped, getCurveElevationPt with curve formulas)
4. **Assets**: common/components/icons/XXXElevation.png (visual indicators in the UI)

A change to one layer often requires verification across all four. Systematic grep prevents missing a call site.

## How to apply
When planning a room_views or elevation change:

1. **Find all elevation-related files** (5-second scan):
   ```bash
   git ls-files | grep -iE 'elevation|room_view'
   ```
   Expect: 5 icons, 4 test files, 4 .cpp/.h source pairs, 1 data structure header.

2. **Locate all painters** (find dispatch logic):
   ```bash
   grep -n 'paintFlatElevation\|paintTentElevation\|paintArchElevation\|paintDomeElevation\|paintCurveElevation' \
     common/components/src/room_views/*.cpp
   grep -n 'setElevationPattern\|currentElevation_' common/components/src/room_views/*.cpp
   ```
   Maps: where each painter is called, how currentElevation_ is set/changed.

3. **Verify math function usage** (trace to Elevation.h):
   ```bash
   grep -n 'getArchElevationPt\|getDomeElevationPtClamped\|getCurveElevationPt' \
     common/components/src/room_views/*.cpp
   ```
   Links painters to the math library; confirms parameter order and return types.

4. **Check path building** (ElevationSurfaces.h methods):
   ```bash
   grep -n 'anchorsToPath\|sampledEdgesToPath' common/components/src/room_views/*.cpp
   ```
   Shows which painters use which path builder; check if a new surface needs a new builder.

5. **Locate tests**:
   ```bash
   ls common/components/tests/*Elevation*test.cpp
   ```
   ElevationSurfaces_test.cpp and ElevationGeometry_test.cpp; ensure test coverage for new surface.

## Example: Adding a new elevation pattern
- Copy an existing painter (e.g. paintArchElevation) as a template
- Add new case to setElevationPattern() switch
- Add corresponding getXxxElevationPt() math to Elevation.h (with formula comments)
- Add new icon to icons/
- Write TEST_F in ElevationSurfaces_test.cpp verifying the new path builder
- Run ctest to verify; build VST3 for DAW visual testing

## Related
- [[Code exploration: grep then read]] (parent learning: grep first, then deep-read specific contexts)
- [[Systematic grep-then-build verification]] (validation: after grep-based changes, verify with build)
- [[vst3-build-verification-for-room-views]] (visual testing for elevation features)
