Closes WithACX/eclipsa-audio-planning#56

[PAN-01.2] Fills in the five elevation painters that PAN-01.1 (#55) stubbed on
`AudioElementPluginTopView`, under the top-down projection.

## Why the shapes are not the rear view's

`Coordinates::getTopViewTransform()` is a 45-degree perspective with the camera
rotated 90 degrees about X, so its `w = 5 - y`: height scales the projection and
an elevation pattern reads as a 3D surface seen from above, not as a flat
outline. Surfaces the rear projection could omit as occluded are visible here
and are drawn.

| Pattern | Shape drawn |
|---|---|
| Flat | One grey panel over the floor at the height `setFlatHeight` carries; the only genuinely single plane. |
| Tent | Two sloping planes off the rear projection's six anchors -- floor corners at (+/-1, -1, -/+1), ridge at (+/-1, 1, 0) -- in the two shades so the ridge reads as an edge. |
| Arch | The parabola sampled along the left and right room edges (41 samples, offset `i * 0.05f`) and stitched into one filled surface; `quadraticTo` kept so the edge stays smooth. |
| Curve | The same treatment against the logarithmic curve, at its existing 32 samples over -1..1. |
| Dome | A filled circle at the `getDomeElevationPtClamped` boundary, sampled through that same function so the radius tracks the clamp rather than a constant. |

Every surface is built from 3D anchor vertices put through
`Coordinates::toWindow(kTransformMat_, window, ...)`, so resizing the plugin
window keeps them aligned to the drawn room. Heights come from the statics in
`Elevation.h`; no elevation curve is re-derived in the view. Signatures and the
`switch` dispatch in `paint()` are unchanged, and the `// TODO(PAN-01.2)`
markers are gone.

**The one sign that matters.** `ElevationListener::parameterChanged` negates the
front/back POSITION PARAMETER before calling `getCurveElevationPt`, and
room-view NDC z is that same parameter negated (`Coordinates::toRoomNdc`), so
the value the listener passes is the NDC front/back coordinate itself. The curve
painter therefore samples un-negated. Tent, arch, and dome are even in that
coordinate, so the sign is immaterial for them -- and there is now a test saying
so.

## Tests

Two new files, covering the two halves that can drift silently.

`common/components/tests/ElevationGeometry_test.cpp` (10 cases) pins the geometry
the painters SAMPLE: the dome's clamp boundary is the unit circle at floor height
and pulls an outside point back onto it, the arch and tent are symmetric about the
room middle while the curve is monotonic and is not, the curve stays inside the
room at both bounds (including the front bound where the logarithm is undefined),
and the sampled runs describe curves rather than planes.

`common/components/tests/ElevationSurfaces_test.cpp` (7 cases) pins the path
ASSEMBLY -- added in response to the review below, together with the
`ElevationSurfaces.h` extraction that makes it reachable. Paths are built against
an identity transform, where `toWindow` reduces to a plain NDC-to-window map and
every expected coordinate is exact: a four-anchor quad lands on the projected
corners, the stitched surface spans both room edges, each edge's samples are
consumed exactly once (2 * (N - 2) quadratic segments plus one far span and one
closing span, at 3, 32, 41 and 81 samples), and the two refusals -- fewer than
three samples, mismatched edge lengths -- hold. Those are the inputs where the
reverse index walk would otherwise wrap.

The painters themselves are still not reachable from a test -- this repo has no UI
test harness (#32) -- so the visual criteria (that Flat reads as one panel, that
the tent ridge is legible, that the toggle updates immediately) are verified by
hand in PAN-01.4 (WithACX/eclipsa-audio-planning#58), as the plan's O5 states.

- `ctest`, run serially: **290/290 passed, 0 failed** (273 before this change).
- `clang-format --style=file:.clang-format --dry-run -Werror` across `common/`,
  `rendererplugin/`, and `audioelementplugin/`: clean tree-wide, not just on the
  diff.
- `cmake --build --target eclipsa_tests`: exit 0. The LTO weak-symbol `ld`
  warnings on `RendererProcessor::kFilePlaybackKey` are pre-existing and
  unrelated.

## Also in this branch: one `chore(a-cx)` commit, unrelated by design

`91ec3f9 chore(a-cx): capture 5 learnings` is the base of this branch. It is an
A-CX carry-forward: internal work product left uncommitted in the primary clone
by earlier sessions, adopted here so it is not stranded. It is **unrelated to
this issue** and is separated from the code commit deliberately. Its contents
were written in another task's context and have not been through any review
aimed at this PR, so read them as content rather than ticking their presence:

- `.a-cx/learnings/autonomous-preflight-workflow-sequence.md`
- `.a-cx/learnings/autonomous-unit-ledger-reconciliation.md`
- `.a-cx/learnings/gh-token-read-project-scope-gap.md`
- `.a-cx/learnings/verify-process-config-before-major-workflows.md`
- `.a-cx/learnings/vst3-manual-testing-build.md`

## Automated review already run on this branch

Three reviewers ran against this diff before the PR was marked ready. Their
verdicts, and what changed in response:

| Dimension | Verdict | Outcome |
|---|---|---|
| C++ | APPROVE | No CRITICAL or HIGH. Loop bounds, the curve sign convention, and the NaN/`sqrt` domain edges were each traced and proven safe rather than assumed. |
| Code | WARNING -> addressed | One IN-SCOPE HIGH: the new path-assembly code had no direct test while the new test file covered only untouched `Elevation.h` math. Fixed by the `ElevationSurfaces.h` extraction and `ElevationSurfaces_test.cpp` above. |
| Security | PASS (code), findings on the carried learnings | Zero findings in the code: memory safety, the unsigned reverse loop, NaN/infinity propagation, and allocation in the 60 Hz paint path were each checked and are clean. |

**Read the carried `.a-cx/learnings/` files before merging.** The security review
found, in `gh-token-read-project-scope-gap.md`, an `## Action` section that states
a GitHub OAuth scope grant as a standalone imperative, at `confidence: 0.95` and
`scope: org-wide`, and that records the live token's granted scopes verbatim.
Later sessions read `.a-cx/learnings/` and apply what they find, so that file
would stand as a self-applying authorization for a credential change. It is filed
as WithACX/eclipsa-audio-planning#68. It is carry-forward content, unrelated to
this issue and not authored by it, so **stripping the `chore(a-cx): capture 5
learnings` commit is a reasonable thing to ask for** if you would rather that file
did not land as written.

## Found while delivering, filed not fixed

WithACX/eclipsa-audio-planning#67 -- `PerspectiveRoomView`'s three display flags
(`displaySpeakers_`, `displayTracks_`, `displayLabels_`) are declared with no
initializer and set by neither constructor, and `paint()` reads them. The audio
element panner never calls `setDisplayTracks` at all, so `if (displayTracks_)`
reads an indeterminate `bool` on every panner paint. Pre-existing since the
initial release; out of scope here and not touched by this PR.

## Cross-repo closing link

The tracker is a separate repo (`tracker.repo` in `.a-cx/github.yaml`), so
GitHub will not auto-close WithACX/eclipsa-audio-planning#56 on merge. Close it
by hand, or leave it to a later sweep.
