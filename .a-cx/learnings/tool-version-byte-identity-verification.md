---
id: tool-version-byte-identity-verification
trigger: "when a tool can be installed via multiple package managers (brew, pip, apt) and needs to be pinned"
confidence: 0.75
domain: process
scope: project
date: 2026-09-02
---

# Verify Tool Byte-Identity Across Package Managers Before Pinning

## Action
Before pinning a tool version to a specific package manager, verify it produces byte-identical output across all distributions (brew, pip, etc.) by running it on your full source tree.

## Evidence
- Observed 1 time in session 1c15f3d8, but executed 3+ sub-steps to complete
- Verified clang-format 23.1.0 from pip vs. brew on 331 files across common/, rendererplugin/, audioelementplugin/
- Result: zero byte differences confirmed, enabling confident pip-based CI pin

## Method

**Step 1: Install both versions**
```bash
# Brew version (system-wide)
brew install clang-format@23.1.0  # if versioned formula exists
# or extract from bottle if not
brew fetch --force --bottle-tag=arm64_sonoma clang-format
CF_BREW=$(find /opt/homebrew/Cellar -name clang-format -type f -perm -u+x | head -1)

# Pip version (isolated venv)
python3 -m venv /tmp/cf23
/tmp/cf23/bin/pip install 'clang-format==23.1.0'
CF_PIP=/tmp/cf23/bin/clang-format
```

**Step 2: Compare on full tree**
```bash
mismatch=0
for f in $(find common rendererplugin audioelementplugin -iname '*.h' -o -iname '*.cpp'); do
  if ! diff -q <("$CF_BREW" --style=file:.clang-format "$f") \
               <("$CF_PIP" --style=file:.clang-format "$f") >/dev/null 2>&1; then
    echo "DIFFERS: $f"
    mismatch=1
  fi
done
[ $mismatch -eq 0 ] && echo "✓ Identical on all $(find ... | wc -l) files"
```

## Why This Matters

- **CI portability:** If pip and brew outputs differ, CI and local developer environments diverge
- **Pin confidence:** Byte-identical verification proves the pin location doesn't matter for correctness
- **Troubleshooting:** Knowing versions are identical rules out "pip gives different output" when diagnosing formatting disagreements

## Gotchas

- Fetch + extract can be slow; run this once per major version
- clang-format writes to stdout; some versions may differ in warning/status output (filter with grep)
- Different platforms (macOS arm64 vs. x86, Linux) may ship subtly different binaries; test your platform
- Some tools don't have venv-safe installs; check for PEP 668 blocking pip

## When to skip
- Single-source tools (e.g., only available via brew): no comparison needed
- Development tools where output variance doesn't matter (linters with warnings)
- Tools you control the version of (vendored binaries, Docker images)

## When to use
- Pinning a tool to a CI job (the pin source matters for reproducibility)
- Switching package managers (e.g., brew → pip) to enable venv isolation
- Validating that a tool is not platform-specific in its output

Related: [[clang-format-pinned-23-1-0]], [[ci-simulation-yaml-extraction-pattern]]
