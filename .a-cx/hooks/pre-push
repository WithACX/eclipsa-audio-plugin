#!/usr/bin/env python3
"""
check_upstream_leak.py -- config-driven pre-push guard.

Blocks a push to the UPSTREAM remote when it would carry any internal path, and
(when `release_scope: internal`) when it would push a tag upstream. Keyed on the
target remote, so internal files stay committed and versioned on the private repo
while never reaching the public upstream.

Self-contained by design: it runs inside a dev repo as its `pre-push` hook and
must NOT import A-CX source-repo modules at runtime (the platform boundary). It
reads `.a-cx/github.yaml` directly for `scm.internal_paths`, `scm.external_paths`
and `release_scope`, unioning any configured internal paths with the built-in
defaults (so a config that omits a default path never silently allows it
upstream). The defaults mirror the original fork guard list; this generalizes that
noise-guard to a config-driven, confidentiality-focused guard.

`scm.external_paths` is the one SUBTRACTION from the guarded
set: an EXACT repo-root-relative file path a repo declares as its own rather than
A-CX's, so a fork that maintains its own `CLAUDE.md`/`AGENTS.md` is not told its
own file is internal. It is deliberately narrow -- exact paths only, never a
directory or a bare basename, never anything on the hard floor
(`EXCLUSION_FLOOR_DIRS`, less the exact paths `EXCLUSION_FLOOR_HELD_OUT_FILES`
holds out, plus every `DEFAULT_INTERNAL_EXACT` entry) -- it is
refused loudly at load time when it is not, and every application of it is
printed, so a relaxation is never silent. It lives HERE, in the one classifier,
so all four consumers (the pre-push block, the mixed-commit reminder, the
upstream-PR replay filter, and the internal-artifact capture) inherit one definition
rather than growing a second path list each.

Install: copied into a dev repo at `.a-cx/hooks/pre-push` with
`git config core.hooksPath .a-cx/hooks`, so the guard is versioned and travels
with the repo (never left untracked in `.git/hooks/`). Setting core.hooksPath
supersedes any `.git/hooks/pre-push`; the default paths below preserve the
original fork guard coverage, so nothing is lost by the switch.

Bypassable client-side (`--no-verify`, CI, the GitHub merge UI); a server-side
check is the real enforcement boundary. It is a path filter, not a secrets
scanner -- normal secrets hygiene still applies.

Also exposes a `classify` subcommand -- `check_upstream_leak.py classify <commit>`
-- that prints how one commit's paths map onto the guarded set
('empty'|'config-only'|'contribution'|'mixed'). This is the single shared
primitive behind the mixed-commit reminder and the upstream-PR
replay filter, so 'internal' means the same thing across the guard, the reminder,
and the filter.

And a `classify-worktree` subcommand -- `check_upstream_leak.py classify-worktree`
-- which prints one `guarded:<path>` / `contribution:<path>` / `scratch:<path>`
line per staged, unstaged or untracked change, through that same matcher.
`classify` needs a commit that already exists, so it cannot answer "which
UNCOMMITTED paths are internal?" -- the question a caller must answer to decide
what to stage. It is read-only; the internal-artifact capture command consumes
the guarded bucket to commit strays as `chore(a-cx): ...`, so guarded work
product never shares a commit with contribution code.

`scratch:` is the third bucket and exists ONLY on this uncommitted path:
a transient `.a-cx/tmp-*` file is neither work product to capture nor
contribution to commit, so calling it either one is wrong in a way that costs
delivery. See `is_transient_scratch` for the full reasoning and for why the rule
is not an ignore rule.

`classify` and `classify-worktree` are RESERVED first arguments: a git remote by
either name would be dispatched as the subcommand rather than push-checked. That
fails closed (the push aborts on a usage error instead of going unchecked), but
do not name a remote either word.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from functools import lru_cache
from pathlib import Path
from typing import FrozenSet, List, Optional, Sequence, Set, Tuple

ZERO_SHA = "0" * 40
SHA_RE = re.compile(r"^[0-9a-f]{40}([0-9a-f]{24})?$")  # sha1, or sha256 (64 hex)
UPSTREAM_REMOTE = os.environ.get("ACX_UPSTREAM_REMOTE", "upstream")

# Built-in defaults: the paths A-CX tooling writes into a dev repo (mirrors the
# original fork guard list). Config `scm.internal_paths` is UNIONED with these, never
# replaces them, so a default internal path is never silently allowed upstream.
#
# EXACT entries (A-CX-authored workflow files) match only their literal path -- a
# same-named file elsewhere (e.g. examples/ci/claude.yml) is unrelated and must
# not be blocked. Every other entry matches by basename anywhere (an A-CX
# instruction file is internal wherever it sits) or, for a trailing-slash entry,
# as a directory at any depth.
DEFAULT_INTERNAL_EXACT = [
    ".github/workflows/acx-marketplace-check.yml",
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

# The transient-scratch class, matched on the UNCOMMITTED path only --
# `is_transient_scratch` below owns the rule and the reasoning. Kept as two plain
# strings rather than a glob or a regex because both halves are matched
# structurally (a basename prefix, and a `.a-cx` parent at any depth), the same
# way `is_guarded` matches its own entries.
SCRATCH_ROOT = ".a-cx"
SCRATCH_BASENAME_PREFIX = "tmp-"

# A capability marker, not a feature flag. `capture_internal_artifacts.py`
# imports this module and can be pointed (via ACX_LEAK_GUARD) at an older installed
# copy whose `load_guard_config` still returns a 2-tuple. That caller checks for
# this name and refuses with a named reason, rather than dying on an arity error.
SUPPORTS_EXTERNAL_PATHS = True

# Directories held OUT of the derived floor below, by name and with the reason
# here in code. `.github/ISSUE_TEMPLATE/` is guarded by default because
# A-CX writes issue templates there, but a repo can legitimately own its own --
# excluding one exact template file is a real use of the escape hatch rather than
# an oversight, so this entry stays excludable one file at a time.
EXCLUSION_FLOOR_HELD_OUT_DIRS = [
    ".github/ISSUE_TEMPLATE/",
]

# The hard floor `scm.external_paths` can never cross: directories whose contents
# are A-CX internal by construction, matched at ANY depth (mirroring is_guarded's
# directory semantics, so `packages/app/.a-cx/x` cannot slip under). BOTH halves of
# the floor are DERIVED rather than re-listed, so adding an A-CX path to the
# defaults can never leave it excludable: the exact half is DEFAULT_INTERNAL_EXACT
# itself, and the directory half is every trailing-slash entry of
# DEFAULT_INTERNAL_PATHS minus the held-out entries above. The directory half was
# hand-listed until it was derived and had drifted from the defaults by two entries.
#
# Config-supplied directories (`scm.internal_paths`) deliberately do NOT reach the
# floor. Its criterion is "internal by construction", which is a property of A-CX's
# own content and not of whatever a consuming repo chose to guard; a repo that
# guards a directory and then excludes one exact file under it is doing precisely
# what `external_paths` exists for, and flooring it would let that repo lock itself
# out of its own escape hatch. Reaching config paths would also widen
# `validate_exclusion`'s signature, which takes no internal paths today, for no
# safety gained.
EXCLUSION_FLOOR_DIRS = [
    path for path in DEFAULT_INTERNAL_PATHS
    if path.endswith("/") and path not in EXCLUSION_FLOOR_HELD_OUT_DIRS
]

# The FILE-level hold-out: one exact repo-root path that a repo may
# declare in `scm.external_paths` even though its directory is on the floor
# above. Held out by file rather than by directory, which is the whole point --
# adding `.claude/` to EXCLUSION_FLOOR_HELD_OUT_DIRS would have made every path
# beside it declarable too.
#
# `.claude/settings.json` is the case that forced it. In a DEV repo `.claude/` is
# A-CX-internal by construction -- the plugin wiring A-CX writes there is not that
# repo's own content -- so the directory belongs on the floor and stays there. In
# the A-CX source repo itself the same file is TRACKED, reviewed, contributed source: a
# PR whose entire fix IS a settings.json edit had its deliverable rerouted into a
# `chore(a-cx)` capture commit, because the classifier gave the repo no way to say
# so. That is a real distinction the blanket rule could not express, and this is
# the narrowest thing that expresses it.
#
# What stays floored, deliberately: `.claude/settings.local.json` (operator-local
# permissions), `.claude/rules/`, `.claude/agents/`, and everything else under the
# directory. Nested spellings stay floored too -- `packages/app/.claude/
# settings.json` is refused -- because the motivating case is the root file and a
# name matched at any depth would unfloor it across a whole monorepo.
#
# It relaxes NOTHING on its own. The path is still guarded by default in every
# repo; it becomes contribution only where that repo's own `scm.external_paths`
# declares it, and `guard_verdict`'s applied-exclusion notice still prints every
# application. Only the DIRECTORY floor is held out -- `DEFAULT_INTERNAL_EXACT`
# still refuses unconditionally, so an A-CX-authored workflow file could not
# become declarable by being listed here, and neither does the bare-ancestor
# floor below, so the `.claude` mount itself stays unexcludable however this list
# grows.
EXCLUSION_FLOOR_HELD_OUT_FILES = [
    ".claude/settings.json",
]

# One process may compute the guarded bucket more than once (the capture command
# reads it to decide, then again immediately before staging). Applied exclusions
# are noted once per process so a second read does not look like a second
# relaxation.
_NOTED_EXCLUSIONS: Set[str] = set()


def _refuse_exclusion(entry, reason: str) -> None:
    """Report a refused `scm.external_paths` entry. The entry is DROPPED, so the
    path it named stays guarded -- refusals always fail in the safe direction, and
    are printed because an author who wrote one believes a relaxation is live."""
    print(f"WARNING: refusing scm.external_paths entry {entry!r}: {reason} The "
          "path stays guarded.", file=sys.stderr)


def validate_exclusion(entry) -> Optional[str]:
    """Normalize one `scm.external_paths` entry, or refuse it and return None.

    Exclusions are EXACT repo-root-relative file paths, matched by equality
    against the paths git reports. Every other form is refused with a named
    reason rather than interpreted, because each interpretation would widen the
    relaxation beyond the one file the author meant:

    - a directory entry (`secret/`) would unguard a whole subtree;
    - a bare basename (`CLAUDE.md`) reads, next to internal entries that DO match
      by basename at any depth, as "unguard this name everywhere" -- so it is
      refused and the root file is written `./CLAUDE.md`, which cannot be read
      any other way. The anchor is accepted on nested entries too, so an author
      never has to remember where it is legal;
    - a bare ANCESTOR of floored content (`./.github`, `./docs`) reads as one exact
      path but names a whole mount: git reports that single path when the
      directory is a symlink or a submodule, so unguarding it carries everything
      beneath it. Refused, and the exact file under it named instead;
    - a glob, pathspec magic, a backslash, an absolute path or a `..`/`.`/empty
      segment can never equal a git-reported path, so accepting it would be a
      silent no-op the author reads as protection -- except `..`, which is an
      outright floor-evasion attempt.

    The hard floor refuses an entry under `EXCLUSION_FLOOR_DIRS` or equal to a
    `DEFAULT_INTERNAL_EXACT` entry, with one bounded exception:
    `EXCLUSION_FLOOR_HELD_OUT_FILES` names exact repo-root paths that stay
    declarable despite the DIRECTORY floor. That list documents its own reasoning;
    the exact-entry floor is never held out.

    Returns the normalized path (no leading `./`) on success.
    """
    if not isinstance(entry, str):
        _refuse_exclusion(entry, f"entries must be strings, got "
                                 f"{type(entry).__name__}.")
        return None
    value = entry.strip()
    if not value:
        _refuse_exclusion(entry, "the entry is empty.")
        return None
    # Every NON-PRINTABLE character, not just the ASCII C0 controls: U+2028,
    # U+2029, U+0085 and U+007F render as line breaks or garbage in a terminal, so
    # an entry carrying one could split or forge a line in the operator-facing
    # refusal and NOTE output. A path git reports with such a character can still
    # be guarded; it just cannot be excluded.
    if any(not ch.isprintable() for ch in value):
        _refuse_exclusion(entry, "the entry contains a non-printable character.")
        return None
    if "\\" in value:
        _refuse_exclusion(entry, "git paths use '/' -- a backslash (Windows "
                                 "separator) is never part of a tracked path.")
        return None
    if value.endswith("/"):
        _refuse_exclusion(entry, "a directory entry would unguard everything "
                                 "under it; name each exact file instead.")
        return None
    if value.startswith("/") or re.match(r"^[A-Za-z]:[\\/]", value):
        _refuse_exclusion(entry, "an absolute path never matches a "
                                 "repo-relative path; write './<path>' for a "
                                 "root-level file.")
        return None
    if value.startswith(":"):
        _refuse_exclusion(entry, "pathspec magic is not accepted -- exact paths "
                                 "only.")
        return None
    if any(ch in value for ch in ("*", "?", "[")):
        _refuse_exclusion(entry, "glob patterns are not accepted -- exact paths "
                                 "only.")
        return None
    anchored = value.startswith("./")
    if anchored:
        value = value[2:]  # the one accepted anchor, normalized away
    if not value:
        _refuse_exclusion(entry, "the entry names no path.")
        return None
    segments = value.split("/")
    if any(seg in ("", ".", "..") for seg in segments):
        _refuse_exclusion(entry, "the path has an empty, '.' or '..' segment; "
                                 "give a canonical repo-relative path.")
        return None
    # A single-segment entry is only unambiguous when the author anchored it: the
    # anchor is what distinguishes "the repo-root file" from "this name anywhere".
    if len(segments) == 1 and not anchored:
        # repr() the interpolated value, like the entry itself: the reason string
        # is printed with plain {reason}, so a raw value here would be the one
        # unescaped path in the message.
        _refuse_exclusion(
            entry, "a bare filename is not an exact path and would unguard that "
                   "name at every depth; write " + repr("./" + value) + " for "
                   "the repo-root file, or " + repr("dir/" + value)
                   + " for a nested one.")
        return None
    # Floor, checked on the NORMALIZED value so an anchored or nested spelling
    # cannot walk under it. The file-level hold-out skips the DIRECTORY
    # half only, and is itself checked on the normalized value, so `./x` and `x`
    # resolve to one entry and nothing nested can reach it. The
    # DEFAULT_INTERNAL_EXACT half below still applies unconditionally.
    #
    # It skips exactly ONE floor entry -- the held-out file's own parent -- rather
    # than the whole loop. With one entry the two are behaviorally identical, but
    # skipping the loop would mean a LATER entry that also matched some unrelated
    # floor silently crossed that floor too, which is not what the hold-out claims
    # to do. Resolving the parent also fails closed: an entry whose parent is not
    # itself a floor directory skips nothing and is refused as before.
    held_out_floor = None
    if value in EXCLUSION_FLOOR_HELD_OUT_FILES and "/" in value:
        held_out_floor = value.rsplit("/", 1)[0] + "/"
    for floor in EXCLUSION_FLOOR_DIRS:
        if floor != held_out_floor:
            bare = floor.rstrip("/")
            # `.a-cx/x`, `a/.a-cx/x`, and the directory name itself (which git can
            # only report as a path when it is a symlink or a submodule mount of
            # that name) are all refused. `is_guarded` matches that last form too,
            # so the floor and the matcher cover the same four shapes.
            if value.startswith(floor) or ("/" + floor) in value \
                    or value == bare or value.endswith("/" + bare):
                _refuse_exclusion(entry, f"'{floor}' is A-CX-internal by "
                                         "construction and can never be "
                                         "excluded.")
                return None
    # The FIFTH shape, floored on the same footing. `guarded_ancestors`
    # made a bare ancestor of floored content -- `.github`, `.github/workflows`,
    # `docs` -- guarded; leaving it EXCLUDABLE would let one unrefused config line
    # undo that guard and carry the whole mount upstream, which is the exact leak
    # the ancestor arm exists to close. Before that arm this was inert (the path
    # was not guarded, so excluding it was a no-op), which is why the floor did
    # not cover it and why covering it belongs with the arm rather than after it.
    #
    # Derived through `guarded_ancestors` from the floor's own two halves, so the
    # refusal set moves with the floor and there is no second list to drift.
    #
    # BUILT-IN ancestors only, which is NARROWER than what the matcher guards:
    # `guard_verdict` derives its arm from the full config-augmented
    # `internal_paths`, so a repo declaring `scm.internal_paths: ['vendor/x/']`
    # has a bare `vendor` guarded and may still exclude `./vendor` here. That is
    # the same scope choice `EXCLUSION_FLOOR_DIRS` already makes and states above
    # -- config paths never reach the floor, because the floor's criterion is
    # "A-CX-internal by construction" and a repo that guards its own directory
    # then excludes one path under it is doing exactly what the hatch is for. It
    # grants nothing either: both keys live in the same `.a-cx/github.yaml`, so
    # an author who can exclude the ancestor can equally delete the
    # `internal_paths` entry. Written out because the loop below otherwise reads
    # as a complete mirror of the matcher's arm, and it is deliberately not one.
    # Resolved per call rather than as a module constant because
    # `guarded_ancestors` is defined below; it is memoized, so this costs nothing.
    # Equality and the nested form only, never a prefix, mirroring the matcher --
    # so an exact file UNDER an ancestor stays excludable, including the
    # deliberately held-out `.github/ISSUE_TEMPLATE/<one-file>.yml` escape hatch
    # and `.github/copilot-instructions.md`.
    for ancestor in guarded_ancestors(
            list(EXCLUSION_FLOOR_DIRS) + list(DEFAULT_INTERNAL_EXACT)):
        if value == ancestor or value.endswith("/" + ancestor):
            _refuse_exclusion(
                entry, f"'{ancestor}' is a bare ancestor of A-CX-internal "
                       "content -- a symlink or submodule mount of it would "
                       "carry that content -- and can never be excluded. Name "
                       "the exact file under it instead.")
            return None
    if value in DEFAULT_INTERNAL_EXACT:
        _refuse_exclusion(entry, "it is an A-CX-authored file in the guard's own "
                                 "defaults and can never be excluded.")
        return None
    return value


def load_guard_config(repo_root: Path) -> Tuple[List[str], List[str], str]:
    """Read scm.internal_paths + scm.external_paths + release_scope from
    .a-cx/github.yaml.

    Returns (internal_paths, external_paths, release_scope). internal_paths is
    the union of the built-in defaults and any configured entries.
    external_paths is the validated, normalized exclusion set that
    `is_guarded` SUBTRACTS -- exclusions are returned beside the list they
    subtract from, so no consumer can read one without the other.
    release_scope defaults to 'public' (no tag restriction). Absent/malformed
    config, or an entry that fails validation -> no exclusions from it, which is
    always the more-guarded direction.
    """
    internal = list(DEFAULT_INTERNAL_PATHS)
    external: List[str] = []
    release_scope = "public"
    cfg_path = repo_root / ".a-cx" / "github.yaml"
    if not cfg_path.is_file():
        return internal, external, release_scope
    try:
        import yaml
    except ImportError:
        # Never fail-open on defaults, but WARN: any custom internal_paths /
        # external_paths / release_scope: internal cannot be read without PyYAML.
        print(f"WARNING: PyYAML unavailable; {cfg_path} not read -- enforcing "
              "built-in internal-path defaults only (custom internal_paths, "
              "external_paths exclusions and release_scope: internal are NOT "
              "applied).", file=sys.stderr)
        return internal, external, release_scope
    try:
        data = yaml.safe_load(cfg_path.read_text(encoding="utf-8")) or {}
    except Exception as exc:
        # Keep the defaults (never fail-open), but make the gap observable.
        print(f"WARNING: could not parse {cfg_path} ({exc}); enforcing built-in "
              "internal-path defaults only.", file=sys.stderr)
        return internal, external, release_scope
    if isinstance(data, dict):
        scm = data.get("scm") or {}
        if isinstance(scm, dict):
            for p in (scm.get("internal_paths") or []):
                if isinstance(p, str) and p.strip() and p.strip() not in internal:
                    internal.append(p.strip())
            external = _load_exclusions(scm.get("external_paths"))
        rs = data.get("release_scope")
        if rs in ("internal", "public"):
            release_scope = rs
    return internal, external, release_scope


def _load_exclusions(raw) -> List[str]:
    """Validate a raw `scm.external_paths` value into the exclusion set.

    A non-list value refuses the whole key (one message, no exclusions); each
    entry is validated independently, so one bad entry never discards the good
    ones and never becomes a wider exclusion than it asked for."""
    if raw is None:
        return []
    if not isinstance(raw, list):
        print("WARNING: refusing scm.external_paths: it must be a list of exact "
              f"path strings, got {type(raw).__name__}. No exclusions applied.",
              file=sys.stderr)
        return []
    exclusions: List[str] = []
    for entry in raw:
        normalized = validate_exclusion(entry)
        if normalized and normalized not in exclusions:
            exclusions.append(normalized)
    return exclusions


def guarded_ancestors(internal_paths: Sequence[str]) -> FrozenSet[str]:
    """Every PROPER directory prefix of every guarded entry.

    `.github` is not itself an entry, but four guarded things live under it -- the
    `.github/instructions/` and `.github/ISSUE_TEMPLATE/` directories, the
    `.github/copilot-instructions.md` basename entry, and all five
    `DEFAULT_INTERNAL_EXACT` workflow files. When `.github` (or
    `.github/workflows`) is a SYMLINK or a SUBMODULE mount, git reports it as the
    single bare path `.github`, which mounts all four and which none of
    `guard_verdict`'s three branches can match: the directory branch has no
    `.github/` entry to compare against, the EXACT branch compares a bare ancestor
    against a full nested file path, and the basename branch compares an
    ancestor's basename against a descendant's.

    Same EVASION CLASS as the bare guarded ROOT the directory branch already
    handles, different SHAPE: that one is a guarded root in its own bare form,
    this one is an UNGUARDED ancestor of guarded content.

    The set is PROPER prefixes only, so an entry never derives itself and the
    three existing branches stay the only thing that matches a full entry. It is
    matched ONLY against a bare path (equality, and the nested `endswith` form) --
    NEVER as a prefix. That asymmetry is the whole ruling: a bare
    `.github/workflows` is guarded because mounting it carries the five A-CX
    workflows, while `.github/workflows/other.yml` keeps its current unguarded
    verdict, because the EXACT list exists precisely so a workflow file A-CX did
    not author (its own comment names `examples/ci/claude.yml`) is not blocked.
    A prefix match would silently take that property away.

    Derived from whatever list it is handed, so a repo's own `scm.internal_paths`
    contribute ancestors on the same terms as the defaults -- there is no second
    hand-maintained list to drift.
    """
    return _guarded_ancestors_cached(tuple(internal_paths))


@lru_cache(maxsize=None)
def _guarded_ancestors_cached(entries: Tuple[str, ...]) -> FrozenSet[str]:
    """`guarded_ancestors` memoized on its (hashable) entry tuple. The matcher is
    called once per changed path and the entry list is fixed for a process, so
    re-deriving the set per path would be the one avoidable cost in a hot loop."""
    out = set()
    for entry in entries:
        segments = entry.rstrip("/").split("/")
        for i in range(1, len(segments)):
            prefix = "/".join(segments[:i])
            if prefix:  # an entry written '/foo' would otherwise derive ''
                out.add(prefix)
    return frozenset(out)


def guard_verdict(path: str, internal_paths: List[str],
                  external_paths: Optional[List[str]] = None) -> Tuple[bool, bool]:
    """(matched_internal, excluded) for one path -- the single evaluation behind
    both `is_guarded` and the applied-exclusion notice, so the verdict and what
    is reported can never disagree.

    `excluded` is exact-path equality against the validated exclusion set only:
    with `CLAUDE.md` excluded, `sub/dir/CLAUDE.md` stays guarded. That asymmetry
    is deliberate -- internal entries match by basename at any depth precisely
    because A-CX writes them anywhere, while an exclusion is one repo's statement
    about one file it owns."""
    matched_internal = False
    basename = path.rsplit("/", 1)[-1]
    for entry in internal_paths:
        if entry.endswith("/"):
            # The last two arms are the guarded root ITSELF, with nothing after
            # it -- the shape git reports when that root is a symlink or a
            # submodule mount rather than an ordinary directory (`M  .a-cx`,
            # `?? .a-cx`), which neither prefix arm can ever match. Equality, not
            # a prefix test, so `.a-cxtra` stays ordinary content; and matched at
            # ANY depth, on the same reasoning as the nested prefix arm beside
            # it. `validate_exclusion`'s floor check already refuses both bare
            # forms, and says so expecting this.
            bare = entry.rstrip("/")
            if path.startswith(entry) or ("/" + entry) in path \
                    or path == bare or path.endswith("/" + bare):
                matched_internal = True
                break
        elif entry in DEFAULT_INTERNAL_EXACT:
            if path == entry:  # workflow files: literal path only
                matched_internal = True
                break
        elif path == entry or basename == entry.rsplit("/", 1)[-1]:
            matched_internal = True
            break
    if not matched_internal:
        # The FOURTH arm: a bare path that is an ancestor of guarded
        # content. Runs only after the three above have all declined, so no
        # currently-guarded path changes verdict by reaching it. Equality and the
        # nested form, never a prefix -- `guarded_ancestors` owns the reasoning.
        for ancestor in guarded_ancestors(internal_paths):
            if path == ancestor or path.endswith("/" + ancestor):
                matched_internal = True
                break
    # The membership test is deliberately typed: `path in <str>` is a SUBSTRING
    # test, so a caller that handed a bare string here (a hand-rolled or
    # mis-evolved guard reached through an import) would silently unguard every
    # path that happens to be a substring of it -- floor entries included. A
    # non-sequence exclusion set means no exclusions, the guarded direction.
    excluded = isinstance(external_paths, (list, tuple, set)) \
        and path in external_paths
    return matched_internal, excluded


def is_guarded(path: str, internal_paths: List[str],
               external_paths: Optional[List[str]] = None) -> bool:
    """True when `path` matches an internal entry and is not excluded.

    - Directory entries (end in '/') match at ANY depth: at the repo root
      (`.a-cx/x`) or nested (`packages/app/.a-cx/x`). A root-only prefix match
      would let a nested internal dir leak upstream. The BARE root -- `.a-cx`,
      `packages/app/.a-cx`, with nothing after it -- matches too, by equality
      rather than prefix: git reports that shape when the root is a symlink or a
      submodule mount, and a prefix-only test would report it as ordinary
      content.
    - EXACT entries (the A-CX workflow files in DEFAULT_INTERNAL_EXACT) match only
      their literal path, so an unrelated same-named file elsewhere is not blocked.
    - Every other entry (instruction files like `CLAUDE.md` or
      `.github/copilot-instructions.md`, and config entries) matches the full path
      or its basename at any depth.
    - A BARE ANCESTOR of guarded content -- `.github`, `.github/workflows`,
      `docs`, at any depth -- matches too, because mounting it as a
      symlink or a submodule carries that content. Whole-path equality only, so
      nothing UNDER an ancestor changes verdict: `.github/workflows/other.yml`
      stays unguarded. `guarded_ancestors` owns the derivation and the reasoning.
    - `external_paths` SUBTRACTS: an exclusion match wins over an
      internal match. It defaults to None so the safe answer -- guarded -- is what
      a caller that knows nothing about exclusions gets."""
    matched_internal, excluded = guard_verdict(path, internal_paths,
                                               external_paths)
    return matched_internal and not excluded


def is_transient_scratch(path: str,
                         repo_root: Optional[Path] = None) -> bool:
    """True for a transient scratch FILE written directly into a `.a-cx/` root:
    a basename starting with `tmp-`, at any depth of `.a-cx` (`.a-cx/tmp-x`,
    `packages/app/.a-cx/tmp-x`), and nothing nested below it.

    "File" is checked, not assumed, and that check FAILS CLOSED. `git status
    -uall` expands an ordinary untracked directory into its files, so a real
    directory of scratch reports as its contents and never reaches here -- but a
    NESTED REPOSITORY, a submodule mount, or a symlink reports as one bare entry
    (`worktree_changed_paths`), and one of those named `.a-cx/tmp-x` could hold
    real internal work product. Reading the path shape alone would drop it out of
    the guarded bucket, so the capture would never commit it and `--check` would
    never hold it: silent loss, which is the direction this whole module refuses.
    So a symlink or a directory is NOT scratch, and anything the check could not
    resolve is not scratch either.

    `repo_root` is where that check is rooted. It defaults to None because the
    capture command imports this module and calls `classify_worktree_paths`
    positionally -- adding a required parameter would break it, and defaulting to
    "assume an ordinary file" would leave the gap exactly where the main consumer
    is. So None means RESOLVE it (`git_toplevel`), and a resolution that fails
    means not scratch.

    Consumed ONLY by `classify_worktree_paths` -- never by `is_guarded`,
    `classify_commit` or `evaluate_ref`. That narrowness is the whole design:

    - **On the UNCOMMITTED path it must not be guarded.** The guarded bucket is
      what `capture_internal_artifacts.py` commits as internal work product and
      what its `--check` gate holds open, so a throwaway probe left in `.a-cx/`
      fails the completion gate of every later unit run from that clone, and the
      gate's own printed remedy commits the throwaway to `main`.
    - **On the COMMITTED path it must stay guarded.** A `.a-cx/tmp-*` that did
      reach a commit is still A-CX-internal content, and the pre-push guard's
      job is to keep it off the upstream remote. Skipping it there would open a
      leak to fix a tidiness problem.

    It is deliberately NOT a `.gitignore` rule, which was the fix the issue
    proposed and the operator ruled against. An untracked `.a-cx/tmp-*` is a
    load-bearing concurrency signal, not noise: a planning session reads
    `git status` for exactly these files to avoid two concurrent sessions
    claiming the same plan key
    (`.a-cx/learnings/plan-key-registry-check-includes-untracked-temp-bodies.md`).
    `git status` is the only place that signal appears, so an ignore rule would
    have quietly broken a documented check. Bucketing here keeps the file
    visible and keeps one definition of what internal work product is.

    `scratch` is its own bucket rather than `contribution` for the same reason:
    a scratch file is not code to commit either, and telling a reader it is
    invites the junk commit from the other direction.
    """
    basename = path.rsplit("/", 1)[-1]
    if not basename.startswith(SCRATCH_BASENAME_PREFIX):
        return False
    parent = path[:-len(basename)].rstrip("/")
    if not (parent == SCRATCH_ROOT or parent.endswith("/" + SCRATCH_ROOT)):
        return False
    # Only now is the filesystem consulted -- the string test rules out every
    # ordinary path first, so a clean tree and a tree full of contribution work
    # both cost nothing here.
    try:
        root = repo_root if repo_root is not None else git_toplevel()
        target = root / path
        if target.is_symlink() or target.is_dir():
            return False
        # A path git reported must exist; one that does not is a state this
        # cannot reason about, so it keeps the guarded answer. (A DELETION is the
        # one benign case, and treating a deleted scratch file as guarded costs
        # nothing: there is no content left to capture.)
        return target.exists()
    except (RuntimeError, OSError):
        return False


def note_applied_exclusions(paths: List[str], internal_paths: List[str],
                            external_paths: Optional[List[str]] = None) -> None:
    """Print one line per exclusion this invocation actually APPLIED -- a path
    that matched an internal entry and was unguarded by `scm.external_paths`.

    Called by the matcher-level entry points (`evaluate_ref`, `classify_commit`,
    `classify_worktree_paths`), never by the subcommand wrappers: the capture
    command imports `classify_worktree_paths` directly and never runs a wrapper,
    so a notice in the wrapper would be invisible to it and each consumer would
    need its own -- exactly the per-consumer logic the shared classifier exists
    to prevent.

    stderr, not stdout: `classify`'s stdout is one machine-read token and
    `classify-worktree`'s is a parsed line list. Deduped per process."""
    if not external_paths:
        return
    for path in sorted(set(paths)):
        matched_internal, excluded = guard_verdict(path, internal_paths,
                                                   external_paths)
        if not (matched_internal and excluded):
            continue
        if path in _NOTED_EXCLUSIONS:
            continue
        _NOTED_EXCLUSIONS.add(path)
        print(f"NOTE: scm.external_paths exclusion applied to {path!r} -- it "
              "matches an A-CX internal entry but this repo declares it its "
              "own, so it is NOT treated as internal.", file=sys.stderr)


def _run_git_paths_z(cmd: List[str], failure: str) -> List[str]:
    """Run a git command whose output is a NUL-separated path list and return the
    paths, verbatim.

    `-z` is what makes the read correct rather than approximate, and every caller
    must pass it. Without it git honors `core.quotePath` -- ON by default -- and
    emits any path carrying a non-ASCII byte, a backslash, or a double quote as a
    QUOTED, octal-escaped C string (`".a-cx/\\303\\266tes.md"`). That literal
    neither starts with `.a-cx/` nor contains `/.a-cx/`, so `is_guarded` matches
    nothing and an internal file reads as ordinary contribution. `-z`
    disables the quoting outright, so there is nothing to unquote.

    Paths are returned UNSTRIPPED: a leading or trailing space is a legal part of
    a filename, and NUL already delimits the fields, so stripping would corrupt
    such a path instead of tidying it. Empty fields are dropped -- with
    `--pretty=format:` git emits the empty header as its own NUL-terminated field,
    so a multi-commit range carries one extra NUL per commit boundary.

    Decoding mirrors `worktree_changed_paths`: bytes in, `utf-8`/`surrogateescape`
    out, so a filename git handles fine never crashes the guard and never depends
    on the ambient locale encoding (`text=True` would decode as cp1252 on a
    default Windows console and mangle exactly the paths this fix is about).

    Fails CLOSED: a git failure raises RuntimeError, so no caller ever reads an
    error as 'no paths changed'.
    """
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        raise RuntimeError(
            f"{failure} ({' '.join(cmd)}): "
            f"{result.stderr.decode('utf-8', 'replace').strip()}"
        )
    payload = result.stdout.decode("utf-8", "surrogateescape")
    return [field for field in payload.split("\0") if field]


def changed_paths(local_sha: str, remote_sha: str) -> set:
    """Union of paths touched anywhere in the pushed range. Walks every commit
    (not just endpoints) so a path added and removed within the range is caught.
    Fails CLOSED: an uncomputable diff raises RuntimeError (caller blocks)."""
    if remote_sha == ZERO_SHA:
        rev = local_sha
    else:
        rev = f"{remote_sha}..{local_sha}"
    cmd = ["git", "log", "--name-only", "-z", "--pretty=format:", rev]
    return set(_run_git_paths_z(cmd, "could not compute the diff being pushed"))


def commit_paths(commit: str) -> List[str]:
    """Paths touched by a SINGLE commit (not a range). Fails CLOSED: an
    uncomputable diff raises RuntimeError so callers never treat an error as
    'no guarded paths'."""
    # Reject an argument that could be parsed as a git option rather than a
    # revision (e.g. `--output=...`), mirroring the SHA validation the pre-push
    # path already does before splicing a value into a git command. The shipped
    # caller passes the literal "HEAD", but `classify` is a shared primitive for
    # future callers (the upstream-PR replay filter feeds it range SHAs).
    if commit.startswith("-"):
        raise RuntimeError(f"refusing commit-ish that looks like an option: {commit!r}")
    # --root so an initial (parentless) commit reports its created paths instead
    # of an empty diff; harmless for ordinary commits (still diffed vs parent).
    # --end-of-options is a second belt: everything after it is a revision/path,
    # never an option (git >= 2.24).
    cmd = ["git", "diff-tree", "--root", "--no-commit-id", "--name-only", "-z",
           "-r", "--end-of-options", commit]
    return _run_git_paths_z(
        cmd, f"could not compute the diff for commit {commit}")


def classify_commit(commit: str, internal_paths: List[str],
                    external_paths: List[str]) -> str:
    """Classify one commit by whether its paths are guarded internal paths.

    The single shared primitive behind the mixed-commit reminder and the
    upstream-PR replay filter -- both reuse `is_guarded`/`load_guard_config`
    so 'internal' means one thing everywhere. Returns:
      - 'empty'         -- no file changes (e.g. a merge or empty commit)
      - 'config-only'   -- every changed path is a guarded internal path
      - 'contribution'  -- no changed path is guarded
      - 'mixed'         -- some guarded, some not (must never be split by tooling)

    `external_paths` is required, not defaulted: an omitted exclusion set here
    would silently misclassify a real repo's commit, and a required parameter
    makes every call site visible in review.
    """
    paths = commit_paths(commit)
    if not paths:
        return "empty"
    note_applied_exclusions(paths, internal_paths, external_paths)
    guarded = [p for p in paths if is_guarded(p, internal_paths, external_paths)]
    if not guarded:
        return "contribution"
    if len(guarded) == len(paths):
        return "config-only"
    return "mixed"


def classify_main(argv: List[str]) -> int:
    """`check_upstream_leak.py classify <commit>` -- print the classification of
    one commit ('empty'|'config-only'|'contribution'|'mixed') to stdout and exit
    0. Reuses the same guarded-path config as the pre-push guard. Exit non-zero
    only on a usage or git error (callers fail closed).

    The config is resolved from the REPO ROOT, not the cwd: `classify` is a
    standalone query run from wherever the caller happens to be, so reading
    `.a-cx/github.yaml` relative to the cwd would silently drop the repo's own
    `scm.internal_paths` on any subdirectory invocation and make the answer
    depend on the caller's directory."""
    if not argv:
        print("usage: check_upstream_leak.py classify <commit>", file=sys.stderr)
        return 2
    commit = argv[0]
    try:
        internal_paths, external_paths, _release_scope = load_guard_config(
            git_toplevel())
        print(classify_commit(commit, internal_paths, external_paths))
    except RuntimeError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    return 0


def _parse_status_z(payload: str) -> List[str]:
    """Parse `git status --porcelain -z` output into changed paths.

    Each entry is `XY <path>` -- a 2-char status field, ONE space, then the path
    (`M  a`, ` M b`, `?? c`, `UU d`). The apparent "two spaces" in `M  a` is a
    literal space in the Y column, so the prefix is always exactly 3 characters
    and the path is `entry[3:]`; splitting on whitespace breaks on ` M b` and on
    any path containing a space.

    For a rename/copy entry (`R` or `C` in either column) `-z` omits the ` -> `
    the human format uses and REVERSES the pair: this field carries the
    DESTINATION and the origin follows as the NEXT NUL field, which must be
    consumed; consuming it for a non-rename entry would swallow the following
    entry. A RENAME origin is reported as a changed path alongside the
    destination, because its removal is half of the change: a caller that
    commits only the destination records an add and leaves the deletion
    staged. A COPY origin is discarded, since a copy leaves it unchanged.
    Unmerged codes (`UU`, `AA`, `DD`, `AU`, `UD`, `DU`, `UA`) never carry a
    second field and never contain `R`/`C`, so the condition cannot collide with
    them.

    Using `-z` is what makes this safe rather than heuristic: it disables
    `core.quotePath` (no `"\\303\\266.md"` wrapping to unquote), and it removes the
    ` -> ` separator, which is unsplittable in the human format because a file may
    legitimately be named `a -> b`.

    Fails CLOSED: an unparseable entry raises RuntimeError rather than being
    skipped, so a caller never reads a short list as a complete one.
    """
    if payload.endswith("\0"):
        payload = payload[:-1]
    if not payload:
        return []
    fields = payload.split("\0")
    paths: List[str] = []
    i = 0
    while i < len(fields):
        entry = fields[i]
        i += 1
        if len(entry) < 4 or entry[2] != " ":
            raise RuntimeError(f"unparseable git status entry: {entry!r}")
        xy, path = entry[:2], entry[3:]
        paths.append(path)
        if "R" in xy or "C" in xy:
            if i >= len(fields):
                raise RuntimeError(
                    f"rename/copy entry missing its origin path: {entry!r}")
            if "R" in xy:
                paths.append(fields[i])
            i += 1
    return paths


def git_toplevel() -> Path:
    """Absolute repo root. Fails CLOSED (RuntimeError) outside a work tree, so a
    caller never mistakes 'not a git repo' for 'nothing changed'. Resolving the
    root explicitly also means `load_guard_config` finds `.a-cx/github.yaml` when
    the command runs from a subdirectory."""
    result = subprocess.run(["git", "rev-parse", "--show-toplevel"],
                            capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(
            f"not inside a git work tree: {result.stderr.strip()}")
    top = result.stdout.strip()
    if not top:
        raise RuntimeError("git rev-parse --show-toplevel returned no path")
    return Path(top)


def worktree_changed_paths(repo_root: Path) -> List[str]:
    """Every path with a staged, unstaged, or untracked change (deduped, sorted).

    `--untracked-files=all` expands untracked directories into individual files.
    Ignored paths are absent because `--ignored` is NOT passed -- that omission is
    the whole guarantee that `.a-cx/runs/` and `.a-cx/worktrees/` never appear.
    (`-uall` changes untracked GRANULARITY only; it never adds ignored paths. A
    TRACKED file inside an ignored directory still reports, which is correct:
    `.gitignore` does not apply to tracked paths.)

    Paths are repo-root-relative: `--porcelain` reports from the repository root
    and ignores `status.relativePaths`, which is what `is_guarded` expects.

    Coverage is bounded by what `git status` itself reports: a submodule is one
    path entry and is never descended into, and a symlink is one blob entry rather
    than a directory to walk. Files changed inside either are invisible here, not
    misclassified. A consumer that stages this output is reading a point-in-time
    snapshot, so re-run it immediately before staging rather than reusing an
    earlier report.

    Fails CLOSED: any git failure raises RuntimeError, matching `commit_paths`.
    """
    cmd = ["git", "status", "--porcelain", "-z", "--untracked-files=all"]
    result = subprocess.run(cmd, cwd=str(repo_root), capture_output=True)
    if result.returncode != 0:
        raise RuntimeError(
            f"could not read the working tree ({' '.join(cmd)}): "
            f"{result.stderr.decode('utf-8', 'replace').strip()}"
        )
    # surrogateescape both ways: a non-UTF-8 filename git handles fine must round
    # trip byte-exactly to the caller that will `git add` it, not crash here.
    return sorted(set(_parse_status_z(
        result.stdout.decode("utf-8", "surrogateescape"))))


def classify_worktree_paths(paths: List[str], internal_paths: List[str],
                            external_paths: List[str],
                            repo_root: Optional[Path] = None) -> List[str]:
    """Bucket working-tree paths through the SAME matcher as the pre-push guard:
    `guarded:<path>` for an A-CX-internal path, `contribution:<path>` otherwise.
    The internal-artifact capture consumes the guarded bucket, so 'internal' means
    one thing across the guard, the mixed-commit reminder, the replay filter, and the
    capture -- there is no second path list anywhere.

    One class is bucketed `scratch:<path>` instead, and ONLY here, on the
    uncommitted path: a transient `.a-cx/tmp-*` file, per `is_transient_scratch`,
    which owns that rule and the reasoning behind it. The test runs FIRST, so a
    scratch path is never also reported guarded; the committed-path entry points
    are untouched and still guard it.

    `repo_root` is optional and only reaches that predicate's fail-closed file
    check -- it is a TRAILING keyword precisely so the capture command's existing
    three-argument call keeps working, and passing None resolves the root rather
    than skipping the check.

    `external_paths` is required for the same reason as in `classify_commit`, and
    the applied-exclusion notice is emitted HERE rather than in the subcommand
    wrapper so the capture command, which imports this function, inherits it."""
    note_applied_exclusions(paths, internal_paths, external_paths)
    lines: List[str] = []
    for p in paths:
        if is_transient_scratch(p, repo_root):
            bucket = "scratch"
        elif is_guarded(p, internal_paths, external_paths):
            bucket = "guarded"
        else:
            bucket = "contribution"
        lines.append(f"{bucket}:{p}")
    return lines


def classify_worktree_main(argv: List[str]) -> int:
    """`check_upstream_leak.py classify-worktree` -- print one line per changed
    path as `guarded:<path>`, `contribution:<path>` or `scratch:<path>`, covering
    staged, unstaged and untracked changes. A clean tree prints nothing and exits
    0. Read-only: it never stages and never commits.

    Exit 2 on a usage or git error, with NOTHING on stdout -- every line is
    computed before the first byte is written, so a caller can never read a
    partial list as a complete one."""
    if argv:
        print("usage: check_upstream_leak.py classify-worktree", file=sys.stderr)
        return 2
    try:
        root = git_toplevel()
        internal_paths, external_paths, _release_scope = load_guard_config(root)
        lines = classify_worktree_paths(
            worktree_changed_paths(root), internal_paths, external_paths,
            root)
    except RuntimeError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    for line in lines:
        sys.stdout.buffer.write(line.encode("utf-8", "surrogateescape") + b"\n")
    sys.stdout.buffer.flush()
    return 0


def evaluate_ref(local_ref: str, local_sha: str, remote_sha: str,
                 internal_paths: List[str], external_paths: List[str],
                 release_scope: str) -> List[str]:
    """Return violation messages for one pushed ref (empty = clean).

    Two rules: (1) any internal path in the pushed range; (2) an internal-scope
    tag heading upstream. `changed_paths` may raise RuntimeError, which the
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
    pushed = sorted(changed_paths(local_sha, remote_sha))
    note_applied_exclusions(pushed, internal_paths, external_paths)
    for p in pushed:
        if is_guarded(p, internal_paths, external_paths):
            violations.append(f"internal path in pushed range: {p}")
    return violations


def main() -> int:
    if len(sys.argv) < 2:
        return 0
    # `classify` subcommand: per-commit classification for the mixed-commit reminder
    # and upstream-PR replay filter. Handled before the remote check because it
    # is a standalone query, not a pre-push invocation (which passes a remote).
    if sys.argv[1] == "classify":
        return classify_main(sys.argv[2:])
    # `classify-worktree`: the same bucketing over UNCOMMITTED changes, for a
    # caller deciding what to stage. Before the remote check for the same reason
    # as `classify` -- a standalone query, not a pre-push invocation.
    if sys.argv[1] == "classify-worktree":
        return classify_worktree_main(sys.argv[2:])
    remote_name = sys.argv[1]
    if remote_name != UPSTREAM_REMOTE:
        return 0  # only the upstream remote is guarded

    internal_paths, external_paths, release_scope = load_guard_config(Path.cwd())
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
                                       internal_paths, external_paths,
                                       release_scope)
        except RuntimeError as exc:
            print(f"BLOCKED: {exc}", file=sys.stderr)
            print(f"Fetch '{remote_name}' and retry, or investigate before "
                  "pushing -- failing closed rather than allowing an unverified "
                  "push.", file=sys.stderr)
            return 1

    if violations:
        print(f"BLOCKED: push to '{remote_name}' would leak internal content:",
              file=sys.stderr)
        sys.stderr.flush()
        for v in sorted(set(violations)):
            # Written as bytes, not via print(). A violation string reaches here
            # through `_run_git_paths_z`'s utf-8/surrogateescape decode, so a
            # filename that is not valid UTF-8 (git never requires one to be)
            # carries lone surrogates. print() would raise UnicodeEncodeError on
            # any stream that cannot represent them -- notably a real Windows
            # console, which writes through the UTF-16 console API. The push
            # would still be blocked (an uncaught exception exits non-zero), but
            # the operator would get a traceback instead of the report naming
            # WHICH path leaked, on exactly the paths this guard exists to
            # catch. `classify_worktree_main` already writes its output this way
            # for the same reason.
            sys.stderr.buffer.write(
                b"  " + v.encode("utf-8", "surrogateescape") + b"\n")
        sys.stderr.buffer.flush()
        print("Push to the internal remote instead, or cut upstream PRs from a "
              "branch that carries none of these paths.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
