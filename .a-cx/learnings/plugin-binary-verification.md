---
id: plugin-binary-verification
trigger: "when verifying or debugging plugin behavior after build"
confidence: 0.85
domain: workflow
scope: project
date: 2026-09-04
---

# Plugin Binary Verification Workflow

## Action
After building VST3 or AU plugins, verify the installed bundles carry the expected changes by comparing SHA256 checksums of the executable binaries against freshly built artifacts, then confirm new handler symbols are present using `nm -C`.

## Evidence

- Observed 3 times in session 736b1387
  - Event 15: Compared SHA256 hashes of built vs installed VST3 binaries, matched mtimes
  - Event 17: Ran `nm -C` to find specific method symbols in installed binary
  - Event 30: Verified all four bundles (VST3 Element, VST3 Renderer, AU Element, AU Renderer) with hash match + symbol count

## Pattern

Each check follows the structure:
1. Locate built binary: `$BUILT/Contents/MacOS/...`
2. Locate installed copy: `~/Library/Audio/Plug-Ins/{VST3,Components}/...`
3. Compare: `shasum -a 256` on both executables
4. Verify symbols: `nm -C <binary> | grep <handler-name>`
5. Check mtimes: `stat -f '%Sm'` to confirm fresh install

This check is what caught the stale AU copies (Sept 3, predating the change) against a fresh VST3 (Sept 4); the fix was to rebuild the AU targets, refreshing both formats without deleting anything. This pattern is critical for audio plugin development where DAWs cache bundles.

## Related

- [[daw-cache-inspection]]: After binary verification, inspect DAW caches to see if stale entries block discovery
