---
id: constexpr-view-transforms-must-live-in-the-header
trigger: "when a test or any file outside components.cpp calls Coordinates::get*ViewTransform() and the link fails with an undefined symbol"
confidence: 0.9
domain: architecture
scope: project
date: 2026-08-13
---
# constexpr room-view transforms must be defined in Coordinates.h, not the .cpp

## Action

Define `constexpr` functions in the header. `Coordinates.h` declared
`getRearViewTransform`, `getSideViewTransform`, and `getTopViewTransform` while
`Coordinates.cpp` held the definitions -- a `constexpr` function has to be
defined in every translation unit that calls it, so every caller outside that
file got an undefined symbol at link time.

Nothing in the plugin noticed, because `common/components/components.cpp`
unity-builds `Coordinates.cpp`, `PerspectiveRoomView.cpp`, and
`PerspectiveRoomViews.cpp` into one translation unit where the definition is
visible. A standalone GoogleTest TU is the first caller outside it, so the
breakage only surfaces the moment someone writes a test that needs a real view
transform:

```
Undefined symbols for architecture arm64:
  "Coordinates::getTopViewTransform()", referenced from:
      HeightIndicatorTest_..._Test::TestBody() in HeightIndicator_test.cpp.o
```

`getIsoViewTransform` is not `constexpr` (it composes matrices at run time) and
correctly stays in the `.cpp`.

## Evidence

- Hit while delivering PAN-01.3 (#57): the new `HeightIndicator_test.cpp`
  asserts the perspective expansion against the real `getTopViewTransform()`
  rather than a duplicated matrix, and failed to link until the three
  definitions moved into `Coordinates.h`.
- The unity build is why this survived unnoticed: `components.cpp` lines 33-35
  include the three room_view sources directly.
