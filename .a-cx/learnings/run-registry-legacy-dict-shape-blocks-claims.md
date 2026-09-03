---
id: run-registry-legacy-dict-shape-blocks-claims
trigger: "when run_registry.py claim exits 2 MALFORMED"
confidence: 0.95
domain: tooling
scope: project
date: 2026-09-03
---

# A Legacy dict-Shaped registry.json Blocks Every Claim

## Action
When `run_registry.py claim` exits 2 with "must be a JSON list of claim entries,
got dict", convert the file from a dict keyed by issue number to a bare list of
its values. Back it up first. Every claim entry is preserved -- the dict's values
already are the entries -- so this is a shape migration, not a data decision.

## Pattern
```bash
cp .git/a-cx/runs/registry.json .git/a-cx/runs/registry.json.dict-backup
python3 - <<'PY'
import json, pathlib
p = pathlib.Path(".git/a-cx/runs/registry.json")
d = json.loads(p.read_text())
assert isinstance(d, dict)
entries = list(d.values())
assert all(isinstance(e, dict) and "issue" in e for e in entries)
p.write_text(json.dumps(entries, indent=2) + "\n")
PY
```

The registry lives in the git COMMON dir (`$(git rev-parse
--git-common-dir)/a-cx/runs/`), not in any worktree, because its job is to
coordinate across worktrees.

## Evidence
Hit at the start of the #59 delivery run. The file held 8 claims (issues 42, 41,
47, 43, 38, 55, 56, 57) as a dict and refused every claim outright. The script's
own docstring names this drift: the contract was prose-only until AUT-19, so
independent implementations wrote different shapes, and a live drain run once
found the same dict-instead-of-list condition.

The refusal is correct behavior -- guessing at the shape could lose claim data --
but it is a hard stop on an untracked coordination file, so recognizing it
quickly matters more than diagnosing it.
