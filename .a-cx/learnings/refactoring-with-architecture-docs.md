---
id: refactoring-with-architecture-docs
trigger: "when refactoring code that carries geometric or algorithmic intent"
confidence: 0.78
domain: code-style
scope: project
date: 2026-08-14
---

# Refactor Code and Docstrings Together

## Action
Update docstrings and comments **as part of the refactor commit**, not as a separate step. Explain the design decision, not just what the code does.

## Evidence
- Observed 8 times in session 7ec26130 (height-indicator branch)
- Pattern: every code refactor paired with docstring/comment updates
  - Lines 3-4: docstring for splitAtElevation refactored to explain it now handles both outline and connectors
  - Lines 7-8: private method signatures updated with comment explaining shared walk pattern
  - Lines 19-22: paintHeightIndicatorConnectors deleted, comment moved to unified paintIndicatorRuns explaining why one function for both
  - Lines 27-30: SplitOutline struct comment updated; pointAlong docstring updated to mention connectors
- Pattern is project-specific: docs explain geometry/projective invariants, not generic intent
  - "The crossing is therefore a genuine incidence between two curves sharing a plane, and a projective transform preserves incidence"
  - "A painter's algorithm has no depth buffer, so the two lists ARE the draw order"

## When to Apply
- Room-view geometry, elevation surfaces, panner features, or any feature with mathematical/geometric intent
- When refactoring introduces new responsibilities or unifies old ones (e.g., outline + connectors → single split function)
- When the "why" is non-obvious from the code alone (e.g., why bisect instead of interpolate the height difference?)

## When NOT to Apply
- Pure style/naming refactors (no algorithmic change, no doc needed)
- Build-breaking fixes where docs and tests are already correct
