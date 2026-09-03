---
id: elevation-surface-batch-exploration
trigger: "when exploring elevation surface painters or height functions across multiple elevation types"
confidence: 0.8
domain: code-style
scope: project
date: 2026-08-14
---

# Elevation Surface Batch Exploration via Bash

## Action
When exploring elevation surfaces (tent, arch, dome, curve, flat), use bash `sed` to extract multiple related functions in one command rather than separate Read calls -- combine painters and height functions in the same sed invocation to map the full rendering chain.

## Evidence
- Observed 5 consecutive times in session 7ec26130-d45c-4ca6-8362-40a98c8cc959 (2026-08-14)
- Lines 13-22 of observer log: exploring tent, flat, dome, curve, arch painters and height functions
- Pattern: `sed -n '/pattern1/,/^}/p' file && echo && sed -n '/pattern2/,/^}/p' file` combined in single bash call
- Each sed extracts a complete function definition (painter or height function) without Read overhead
- User targeted both painters (paintTentElevation, paintDomeElevation, etc.) and helper functions (getTentElevationPt, getDomeElevationPtClamped) in same exploration pass

## Why
- Bash sed is faster than repeated Read calls for extracting definitions (no file loading per range)
- Extracting related functions (painter + height function + boundary sampler) in one go reduces context switching
- Elevation surface code lives in two files (PerspectiveRoomViews.cpp and Elevation.h) -- batching across both files keeps the flow coherent
- Works well for surfaces with symmetric structure (tent, arch, dome, curve all follow painter → height function → visual boundary pattern)

## How to Apply
When exploring a new elevation surface type or tracing rendering dependencies:
1. Start with the painter function in PerspectiveRoomViews.cpp
2. Extract both the painter AND its corresponding height function in ONE sed invocation
3. Include the boundary sampler or clamping logic (if present) in the same call
4. Chain painters of similar shapes (e.g., tent + arch, or dome + curve) in a single bash command with multiple sed ranges separated by `echo`
5. Only Read a file if you need context beyond the function definition (e.g., constants, includes, comments not on the function line)
