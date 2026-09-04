---
id: clang-format-verification-workflow
trigger: "when formatting code before pushing to acx/dev"
confidence: 0.85
domain: workflow
scope: project
date: 2026-09-03
---

# Pinned Clang-Format Verification Workflow

## Action

After editing code, format with a **pinned clang-format 23.1.0** in a scratchpad venv before committing. CI enforces the exact version; this catches violations locally without a round-trip to CI.

## Evidence

- Observed 3+ times in session 3fad9120-e811 (lines 37–40, 43–44, 2026-09-03, issue #59)
- Local brew clang-format was 22.1.8; CI requires 23.1.0
- First formatting pass used local version → tree failed CI check (TREE_FORMAT_CLEAN=NO)
- Second pass: set up venv with `pip install clang-format==23.1.0` → all three directories passed (TREE_FORMAT_CLEAN=yes)
- This avoided pushing non-compliant code and discovering the mismatch post-PR

## How to Apply

### Step 1: Set Up Pinned Venv (once per session/worktree)

```bash
SP="$SCRATCHPAD_ROOT"  # or /private/tmp/claude-504/...
python3 -m venv "$SP/cf-venv"
"$SP/cf-venv/bin/pip" install --quiet 'clang-format==23.1.0'
```

### Step 2: Format Each Changed File

```bash
CF="$SP/cf-venv/bin/clang-format"
for f in edited_files...; do
  "$CF" --style=file:.clang-format -i "$f"
done
```

### Step 3: Verify Tree Passes

```bash
export PATH="$SP/cf-venv/bin:$PATH"
overall=0
for d in common rendererplugin audioelementplugin; do
  find "$d" -iname '*.h' -o -iname '*.cpp' | \
    xargs clang-format --style=file:.clang-format --dry-run -Werror -i 2>&1 && \
    echo "$d: clean" || {
      echo "$d: VIOLATIONS"
      overall=1
    }
done
echo "TREE_FORMAT_CLEAN=$([ $overall -eq 0 ] && echo yes || echo NO)"
```

**Expected output on success:**
```
common: clean
rendererplugin: clean
audioelementplugin: clean
TREE_FORMAT_CLEAN=yes
```

## Why This Matters

- `.acx-dev.yml` and `cmake-multi-platform.yml` both enforce `-Werror` with the exact pinned version
- Catching formatting violations **before** pushing saves a CI round-trip and prevents PR comments from bots
- The venv is scratchpad-local; it does not pollute the main repo or project `venv`

## Related Memories

- [[python-anchor-text-replacement-pattern]] -- use before running format check
