---
id: python-anchor-text-replacement-pattern
trigger: "when making surgical multi-line code additions in C++"
confidence: 0.85
domain: code-style
scope: project
date: 2026-09-03
---

# Python Anchor-Text Replacement for Surgical Code Insertion

## Action

Use Python `pathlib` with a unique anchor string to insert multi-line code blocks atomically, avoiding offset/limit fragility in file editing.

## Evidence

- Observed 5+ times in session 3fad9120-e811 (2026-09-03, issue #59)
- Pattern: lines 5, 7, 11, 13, 19, 21, 51 of observed session
- Example: Added `fromTopViewWindow` declaration to header, implementation to .cpp, named constants, and mouse handlers using this pattern
- Zero failures; all anchors were unique and replacements atomic
- Alternative: direct `Edit` tool failed on offset calculations; this pattern proved more reliable

## Evidence Details

1. **Add Declaration (line 5)**
   - Anchor: `"PositionParameters fromRoomNdc(const Point4D& ndcPoint);\n"`
   - Inserted documentation + declaration after it
   - Verified grep output: declaration found at expected line

2. **Add Implementation (line 7)**
   - Anchor: `"Mat4 getIsoViewTransform() {"`
   - Inserted full function body before it
   - Verified sed output: function now present with correct range

3. **Add Constants + Helpers (line 13, 19, 21)**
   - Anchor: `"// The height indicator's line weights.\n..."`
   - Added marker diameter constant, grab margin, helper function
   - Replaced hardcoded `14.f` literals in `drawTrack` with named constant

4. **Numerical Verification (line 53)**
   - Before committing C++ tests, ran Python simulation
   - Confirmed 343 round-trip conversions succeeded with 0 failures
   - Gives confidence that test logic is sound

## How to Apply

In a worktree/session context, when adding multiple related code blocks:

```bash
PY="$(command -v python3 || command -v python)"
"$PY" - <<'PYEOF'
import pathlib
p = pathlib.Path("file.cpp")
s = p.read_text()

# Find a unique anchor that will not change
anchor = "unique_identifiable_string"
assert s.count(anchor) == 1, f"anchor not unique or not found"

# Build the addition(s)
addition = """
// New code here
"""

# Replace atomically
p.write_text(s.replace(anchor, addition + anchor))
print("file updated")
PYEOF
```

**Why this beats offset/limit:**
- No position calculation needed; anchor is the source of truth
- Atomic replacement: no partial updates or off-by-one bugs
- Clear assertion if anchor is ambiguous or missing
- Works across multiple additions in one script

## Related Memories

- [[clang-format-verification-workflow]] -- after insertion, run formatted verification
