#!/usr/bin/env python3
"""
check_upstream_leak.py -- config-driven pre-push guard (DPM-03, objectives O3/O6).

Blocks a push to the UPSTREAM remote when it would carry any internal path, and
(when `release_scope: internal`) when it would push a tag upstream. Keyed on the
target remote, so internal files stay committed and versioned on the private repo
while never reaching the public upstream.

Self-contained by design: it runs inside a dev repo as its `pre-push` hook and
must NOT import a-cx-ai-config modules at runtime (the platform boundary). It
reads `.a-cx/github.yaml` directly for `scm.internal_paths` and `release_scope`,
unioning any configured internal paths with the built-in defaults (so a config
that omits a default path never silently allows it upstream). The defaults mirror
the ADR-0067 guard list; this generalizes that noise-guard to a config-driven,
confidentiality-focused guard.

Install (DPM-03.2): copied into a dev repo at `.a-cx/hooks/pre-push` with
`git config core.hooksPath .a-cx/hooks`, so the guard is versioned and travels
with the repo (never left untracked in `.git/hooks/`). Setting core.hooksPath
supersedes any `.git/hooks/pre-push`; the default paths below preserve the
ADR-0067 coverage, so nothing is lost by the switch.

Bypassable client-side (`--no-verify`, CI, the GitHub merge UI); a server-side
check is the real enforcement boundary. It is a path filter, not a secrets
scanner -- normal secrets hygiene still applies.

Also exposes a `classify` subcommand -- `check_upstream_leak.py classify <commit>`
-- that prints how one commit's paths map onto the guarded set
('empty'|'config-only'|'contribution'|'mixed'). This is the single shared
primitive behind the DPM-10 mixed-commit reminder and the DPM-11 upstream-PR
replay filter, so 'internal' means the same thing across the guard, the reminder,
and the filter.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple

ZERO_SHA = "0" * 40
SHA_RE = re.compile(r"^[0-9a-f]{40}([0-9a-f]{24})?$")  # sha1, or sha256 (64 hex)
UPSTREAM_REMOTE = os.environ.get("ACX_UPSTREAM_REMOTE", "upstream")

# Built-in defaults: the paths A-CX tooling writes into a dev repo (mirrors the
# ADR-0067 guard list). Config `scm.internal_paths` is UNIONED with these, never
# replaces them, so a default internal path is never silently allowed upstream.
#
# EXACT entries (A-CX-authored workflow files) match only their literal path -- a
# same-named file elsewhere (e.g. examples/ci/claude.yml) is unrelated and must
# not be blocked. Every other entry matches by basename anywhere (an A-CX
# instruction file is internal wherever it sits) or, for a trailing-slash entry,
# as a directory at any depth.
DEFAULT_INTERNAL_EXACT = [
    ".github/workflows/claude.yml",
    ".github/workflows/deploy-copilot-settings.yml",
    ".github/workflows/org-defaults-drift.yml",
    ".github/workflows/status-sync.yml",
    ".github/workflows/template-check.yml",
]
DEFAULT_INTERNAL_PATHS = [
    ".a-cx/", "docs/plan/", ".claude/", ".agents/",
    ".github/instructions/", ".github/ISSUE_TEMPLATE/", ".junie/",
    "AGENTS.md", "CLAUDE.md", "GEMINI.md",
    ".github/copilot-instructions.md", "pull_request_template.md",
] + DEFAULT_INTERNAL_EXACT


def load_guard_config(repo_root: Path) -> Tuple[List[str], str]:
    """Read scm.internal_paths + release_scope from .a-cx/github.yaml.

    Returns (internal_paths, release_scope). internal_paths is the union of the
    built-in defaults and any configured entries. release_scope defaults to
    'public' (no tag restriction). Absent/malformed config -> defaults only.
    """
    internal = list(DEFAULT_INTERNAL_PATHS)
    release_scope = "public"
    cfg_path = repo_root / ".a-cx" / "github.yaml"
    if not cfg_path.is_file():
        return internal, release_scope
    try:
        import yaml
    except ImportError:
        # Never fail-open on defaults, but WARN: any custom internal_paths /
        # release_scope: internal cannot be read without PyYAML.
        print(f"WARNING: PyYAML unavailable; {cfg_path} not read -- enforcing "
              "built-in internal-path defaults only (custom internal_paths and "
              "release_scope: internal are NOT applied).", file=sys.stderr)
        return internal, release_scope
    try:
        data = yaml.safe_load(cfg_path.read_text(encoding="utf-8")) or {}
    except Exception as exc:
        # Keep the defaults (never fail-open), but make the gap observable.
        print(f"WARNING: could not parse {cfg_path} ({exc}); enforcing built-in "
              "internal-path defaults only.", file=sys.stderr)
        return internal, release_scope
    if isinstance(data, dict):
        scm = data.get("scm") or {}
        if isinstance(scm, dict):
            for p in (scm.get("internal_paths") or []):
                if isinstance(p, str) and p.strip() and p.strip() not in internal:
                    internal.append(p.strip())
        rs = data.get("release_scope")
        if rs in ("internal", "public"):
            release_scope = rs
    return internal, release_scope


def is_guarded(path: str, internal_paths: List[str]) -> bool:
    """True when `path` matches an internal entry.

    - Directory entries (end in '/') match at ANY depth: at the repo root
      (`.a-cx/x`) or nested (`packages/app/.a-cx/x`). A root-only prefix match
      would let a nested internal dir leak upstream.
    - EXACT entries (the A-CX workflow files in DEFAULT_INTERNAL_EXACT) match only
      their literal path, so an unrelated same-named file elsewhere is not blocked.
    - Every other entry (instruction files like `CLAUDE.md` or
      `.github/copilot-instructions.md`, and config entries) matches the full path
      or its basename at any depth."""
    basename = path.rsplit("/", 1)[-1]
    for entry in internal_paths:
        if entry.endswith("/"):
            if path.startswith(entry) or ("/" + entry) in path:
                return True
        elif entry in DEFAULT_INTERNAL_EXACT:
            if path == entry:  # workflow files: literal path only
                return True
        elif path == entry or basename == entry.rsplit("/", 1)[-1]:
            return True
    return False


def changed_paths(local_sha: str, remote_sha: str) -> set:
    """Union of paths touched anywhere in the pushed range. Walks every commit
    (not just endpoints) so a path added and removed within the range is caught.
    Fails CLOSED: an uncomputable diff raises RuntimeError (caller blocks)."""
    if remote_sha == ZERO_SHA:
        cmd = ["git", "log", "--name-only", "--pretty=format:", local_sha]
    else:
        cmd = ["git", "log", "--name-only", "--pretty=format:",
               f"{remote_sha}..{local_sha}"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(
            f"could not compute the diff being pushed ({' '.join(cmd)}): "
            f"{result.stderr.strip()}"
        )
    return {line.strip() for line in result.stdout.splitlines() if line.strip()}


def commit_paths(commit: str) -> List[str]:
    """Paths touched by a SINGLE commit (not a range). Fails CLOSED: an
    uncomputable diff raises RuntimeError so callers never treat an error as
    'no guarded paths'."""
    # Reject an argument that could be parsed as a git option rather than a
    # revision (e.g. `--output=...`), mirroring the SHA validation the pre-push
    # path already does before splicing a value into a git command. The shipped
    # caller passes the literal "HEAD", but `classify` is a shared primitive for
    # future callers (the DPM-11 replay filter feeds it range SHAs).
    if commit.startswith("-"):
        raise RuntimeError(f"refusing commit-ish that looks like an option: {commit!r}")
    # --root so an initial (parentless) commit reports its created paths instead
    # of an empty diff; harmless for ordinary commits (still diffed vs parent).
    # --end-of-options is a second belt: everything after it is a revision/path,
    # never an option (git >= 2.24).
    cmd = ["git", "diff-tree", "--root", "--no-commit-id", "--name-only", "-r",
           "--end-of-options", commit]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(
            f"could not compute the diff for commit {commit} "
            f"({' '.join(cmd)}): {result.stderr.strip()}"
        )
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def classify_commit(commit: str, internal_paths: List[str]) -> str:
    """Classify one commit by whether its paths are guarded internal paths.

    The single shared primitive behind the DPM-10 mixed-commit reminder and the
    DPM-11 upstream-PR replay filter -- both reuse `is_guarded`/`load_guard_config`
    so 'internal' means one thing everywhere. Returns:
      - 'empty'         -- no file changes (e.g. a merge or empty commit)
      - 'config-only'   -- every changed path is a guarded internal path
      - 'contribution'  -- no changed path is guarded
      - 'mixed'         -- some guarded, some not (must never be split by tooling)
    """
    paths = commit_paths(commit)
    if not paths:
        return "empty"
    guarded = [p for p in paths if is_guarded(p, internal_paths)]
    if not guarded:
        return "contribution"
    if len(guarded) == len(paths):
        return "config-only"
    return "mixed"


def classify_main(argv: List[str]) -> int:
    """`check_upstream_leak.py classify <commit>` -- print the classification of
    one commit ('empty'|'config-only'|'contribution'|'mixed') to stdout and exit
    0. Reuses the same guarded-path config as the pre-push guard. Exit non-zero
    only on a usage or git error (callers fail closed)."""
    if not argv:
        print("usage: check_upstream_leak.py classify <commit>", file=sys.stderr)
        return 2
    commit = argv[0]
    internal_paths, _release_scope = load_guard_config(Path.cwd())
    try:
        print(classify_commit(commit, internal_paths))
    except RuntimeError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    return 0


def evaluate_ref(local_ref: str, local_sha: str, remote_sha: str,
                 internal_paths: List[str], release_scope: str) -> List[str]:
    """Return violation messages for one pushed ref (empty = clean).

    Two rules: (1) any internal path in the pushed range; (2) an internal-scope
    tag heading upstream (O6). `changed_paths` may raise RuntimeError, which the
    caller turns into a fail-closed block."""
    violations: List[str] = []
    if local_sha == ZERO_SHA:
        return violations  # ref deletion -- nothing pushed
    if local_ref.startswith("refs/tags/") and release_scope == "internal":
        tag = local_ref[len("refs/tags/"):]
        violations.append(
            f"internal-scope release tag '{tag}' must not be pushed upstream "
            f"(release_scope: internal)"
        )
    for p in sorted(changed_paths(local_sha, remote_sha)):
        if is_guarded(p, internal_paths):
            violations.append(f"internal path in pushed range: {p}")
    return violations


def main() -> int:
    if len(sys.argv) < 2:
        return 0
    # `classify` subcommand: per-commit classification for the DPM-10 reminder
    # and DPM-11 upstream-PR filter. Handled before the remote check because it
    # is a standalone query, not a pre-push invocation (which passes a remote).
    if sys.argv[1] == "classify":
        return classify_main(sys.argv[2:])
    remote_name = sys.argv[1]
    if remote_name != UPSTREAM_REMOTE:
        return 0  # only the upstream remote is guarded

    internal_paths, release_scope = load_guard_config(Path.cwd())
    violations: List[str] = []
    for line in sys.stdin:
        stripped = line.strip()
        if not stripped:
            continue  # trailing blank line from git -- not a ref update
        parts = stripped.split()
        # Fail CLOSED on an unparseable ref line, rather than skipping it (a
        # skipped line could be the only one, silently allowing the push).
        if len(parts) != 4:
            print(f"BLOCKED: malformed ref update from git: {stripped!r}",
                  file=sys.stderr)
            return 1
        local_ref, local_sha, _remote_ref, remote_sha = parts
        # Validate BOTH shas before either is spliced into a git revision range.
        if not (SHA_RE.match(local_sha) or local_sha == ZERO_SHA) or \
           not (SHA_RE.match(remote_sha) or remote_sha == ZERO_SHA):
            print(f"BLOCKED: malformed ref update from git: {stripped!r}",
                  file=sys.stderr)
            return 1
        try:
            violations += evaluate_ref(local_ref, local_sha, remote_sha,
                                       internal_paths, release_scope)
        except RuntimeError as exc:
            print(f"BLOCKED: {exc}", file=sys.stderr)
            print(f"Fetch '{remote_name}' and retry, or investigate before "
                  "pushing -- failing closed rather than allowing an unverified "
                  "push.", file=sys.stderr)
            return 1

    if violations:
        print(f"BLOCKED: push to '{remote_name}' would leak internal content:",
              file=sys.stderr)
        for v in sorted(set(violations)):
            print(f"  {v}", file=sys.stderr)
        print("Push to the internal remote instead, or cut upstream PRs from a "
              "branch that carries none of these paths.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
