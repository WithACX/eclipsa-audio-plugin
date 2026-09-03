---
id: ci-simulation-yaml-extraction-pattern
trigger: "when reproducing CI failures locally or verifying workflow changes before pushing"
confidence: 0.80
domain: workflow
scope: project
date: 2026-09-02
---

# Extract and Simulate CI Locally via YAML Parsing

## Action
Extract the exact `run` block from a GitHub workflow step as a shell script, then execute it locally to reproduce CI state without pushing and waiting for runners.

## Evidence
- Observed 4 times in session 1c15f3d8 on eclipsa-audio-plugin PR #129 debugging
- Pattern: `python3 -m yaml.safe_load()` → extract step `run` → `bash` the result
- Verified workflow edits in `acx-dev-ci.yml` twice before pushing

## Method

**Extract:**
```bash
python3 -c "
import yaml, sys
d = yaml.safe_load(open('.github/workflows/acx-dev-ci.yml'))
step = [s for s in d['jobs']['clang-format']['steps'] if s.get('name')=='Check Formatting'][0]
print(step['run'])
" > /tmp/ci_step.sh
```

**Simulate:**
```bash
export RUNNER_TEMP=/tmp/runner_temp  # match GitHub's env
bash /tmp/ci_step.sh
echo "EXIT=$?"
```

**Why this works:**
- Exact reproduction of the step's bash (including multi-line `run:` blocks)
- Can inject test environment vars (`RUNNER_TEMP`, `GITHUB_ENV`, etc.)
- Fails or succeeds identically to CI before the 10-minute runner wait
- Catches logic errors, version mismatches, missing dependencies

## Gotchas
- Multi-line `run:` blocks need careful quoting when embedded in Python strings
- `$GITHUB_ENV` and `$RUNNER_TEMP` must be set or the step silently uses wrong paths
- YAML parser will fail if the step's `run:` is malformed; syntax errors surface immediately
- Works best for deterministic steps (format checks, linting); flaky tests may still pass locally

## When to use
- Before committing workflow changes
- When diagnosing why a specific CI job fails
- To verify tool versions or environment setup without pushing
- When iterating on CI fix (edit → test locally → commit once green)

Related: [[clang-format-pinned-23-1-0]], [[ctest-names-by-gtest-suite-not-cmake-target]]
