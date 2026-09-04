---
id: parametric-expansion-2d-geometry
trigger: "when expanding height/elevation functions from 1D to 2D domains"
confidence: 0.80
domain: architecture
scope: project
date: 2026-08-14
---

# Parametric Expansion for 2D Geometry Functions

## Action
When a height or elevation function must support 2D surface variations (e.g., dome), systematically expand its signature to accept both left/right and front/back coordinates, then update call sites, lambdas, documentation, and tests in one coherent pass.

## Pattern
Tent, arch, and curve are 1D height fields (front/back alone); the dome falls away in both dimensions. When the dome's logic is added:

1. Expand function signature: `float roofHeightAt(float frontBack)` → `float roofHeightAt(float leftRight, float frontBack)`
2. Update all lambdas passed to split functions to match the new arity
3. Update docstrings to explain which patterns use which coordinates (e.g., "only dome needs both")
4. Add new test cases for the 2D case (dome crossing both connectors at once)
5. Implement the expanded logic with domain checks (e.g., radius squared for dome)

## Evidence
- Observed 4 times in session 7ec26130-d45c-4ca6-8362-40a98c8cc959 (2026-08-14):
  1. `elevationHeightAt()` signature in header and implementation
  2. `roofHeightAt()` in test helpers
  3. Lambdas in test call sites (3+ locations)
  4. New helper function `elevationVariesAcrossLeftRight()`

## Related Memories
- [[Refactoring with architecture docs]] -- explains geometry-specific documentation patterns
- [[Type change impact analysis]] -- systematic approach to tracing downstream impacts (applicable here for signature changes)
