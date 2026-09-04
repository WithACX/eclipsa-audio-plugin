---
id: daw-cache-inspection
trigger: "when debugging why DAW doesn't pick up updated plugin builds or shows stale entry"
confidence: 0.8
domain: workflow
scope: project
date: 2026-09-04
---

# DAW Cache Inspection Pattern

## Action
When a plugin build is fresh but the DAW (e.g., REAPER) doesn't recognize changes, first confirm the installed BINARIES are fresh, then inspect the DAW's plugin cache files (e.g., `reaper-vstplugins_arm64.ini`, `reaper-auplugins_arm64.ini`) for Eclipsa entries and their mtimes, and force a re-scan from the DAW's own preferences.

## Evidence

- Observed 3+ times in session 736b1387
  - Event 25: Listed REAPER resource dir and checked VST path config
  - Event 26: Grepped REAPER cache files for Eclipsa entries (VST and AU)
  - Event 27: Found Eclipsa in VST cache dated Aug 14, AU cache dated Oct 29 (both stale relative to Sept 4 build)

## Pattern

REAPER caches plugin metadata in INI files:
- `~/Library/Application Support/REAPER/reaper-vstplugins_arm64.ini`
- `~/Library/Application Support/REAPER/reaper-auplugins_arm64.ini`

Check workflow:
1. `grep -i "eclipsa" $CACHE_FILE` to find entries
2. `ls -l $CACHE_FILE` to see mtime
3. If cache mtime is older than the build, REAPER has stale metadata
4. Force a re-scan through REAPER itself: Options > Preferences > Plug-ins >
   VST > Re-scan. Prefer this to hand-editing or deleting the cache file --
   the INI is REAPER's own state and it rewrites it on scan. REAPER normally
   re-scans a bundle whose timestamp or size changed, so a forced re-scan is
   belt-and-braces rather than always required.

Correction (reviewed by hand): the blocker in this session was the stale AU
BINARIES (Sept 3 components carrying none of the new handlers), not the stale
cache. The two are separate failures and it matters which one you are looking
at: stale binaries mean the DAW loads old code, while a stale cache means the
DAW does not list a plugin that is actually current. Verify the binaries first
(see the related learning below), then the cache.

## Related

- [[plugin-binary-verification]]: Verify binaries are fresh before checking DAW caches
