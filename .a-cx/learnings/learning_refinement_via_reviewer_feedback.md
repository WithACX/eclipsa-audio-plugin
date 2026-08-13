---
id: learning-refinement-captures-reviewer-feedback
trigger: "when a learning file documents a solution with alternative implementations"
confidence: 0.75
domain: process
scope: project
date: 2026-08-11
---

# Capture Reviewer Feedback in Learning Files as Implementation Trade-offs

## Action

When code review feedback changes how you implement a solution after writing initial code, update the associated learning file to document both the solution path you took AND the rejected alternatives with their trade-offs clearly named. Don't re-frame the learning as if only your final approach existed — capture that the alternative was considered, rejected on specific grounds, and why that trade-off is defensible.

## Evidence

- `dismissible-banner-clear-underlying-value-not-a-new-flag` learning was written initially with an extracted `applyDismiss()` helper function and full unit test coverage (event 6, 11: `ExportErrorBanner_test.cpp` had tests for the helper, then removed).
- Reviewer feedback preferred keeping the mutation inline in `onDismiss()` for simplicity (event 5: removed the static helper, inlined the 3-line mutation).
- Learning file was then edited 4 times (events 15, 16, 18) to document that both approaches (extracted helper with headless test coverage vs. inline for simplicity) are defensible, and asked rather than assumed which the reviewer prefers for small one-line mutations.
- Result: learning file now captures the trade-off explicitly, not as a prescriptive rule but as a judgment call with visible reasoning.

## How to apply

1. Document alternative solutions you considered (especially ones you initially wrote and then changed based on feedback).
2. Name the trade-offs explicitly: "Extracted helper buys headless testability; inline buys simplicity for one-line mutation."
3. State the deciding factor: "Ask rather than assume" for borderline cases, or "Always extract when X" for clear wins.
4. Avoid re-framing the learning as if only the final implementation existed — visibility into the decision process makes the learning more useful to others facing the same choice.

## Related

- [[dismissible-banner-clear-underlying-value-not-a-new-flag]]
- Code review feedback shapes architecture; capture that influence in learnings so others understand the constraints.
