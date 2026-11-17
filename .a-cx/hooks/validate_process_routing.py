#!/usr/bin/env python3
"""
validate_process_routing.py -- validate a consuming repo's declared model
routing against the bundled routing policy.

WHY THIS SHIPS IN THE PLUGIN PAYLOAD
------------------------------------
Agent and skill frontmatter is checked against the policy where the plugins are
authored, and ships read-only into dev repos, so a consuming repo cannot drift it.

`.a-cx/process.yaml` is the opposite case. It is authored per dev repo and read
at run time by `deliver-code`, `deliver-pr-review`, and `deliver-autonomous`, and
no authoring-side check ever sees it -- so it has to be validated where it
lives. This script lives beside the `knowledge/model-routing.yaml` that each
delivery skill already bundles, which is what makes it reachable from a dev repo
at all.

WHAT IT CHECKS
--------------
Two independent classes, with deliberately different consequences:

  SCHEMA (refuse) -- an unknown phase key, a model value outside
  `allowed_models`, or an effort value outside `allowed_effort`. The declaration
  is unresolvable, so the run cannot know what it would route to. Preflight
  refuses before any board move, assignment, branch, worktree, or registry claim
  (`knowledge/delivery-gates.md`).

  POLICY (refuse only without an override) -- a phase whose task class is pinned
  to the reasoning tier, routed to a lower tier. This is a legitimate,
  documented engagement choice, so it is not a schema error; but it is also the
  exact drift the no-regression bar on reasoning-tier phases exists to prevent,
  so it proceeds only against an explicit `deliver.routing_overrides` entry.
  With a matching override: WARN and proceed. Without one: REFUSE.

ABSENCE IS NOT A VIOLATION. No `.a-cx/process.yaml`, no `deliver` block, or no
`models` / `run_profiles` blocks all pass clean. Most dev repos declare nothing
here and must stay unaffected.

The reasoning-pinned set is DERIVED from the routing map (a phase whose
`task_classes` entry has `tier: reasoning`), never hardcoded -- so retiering a
class in the yaml moves this check with it.

THE CONFIG IS UNTRUSTED INPUT
----------------------------
`.a-cx/process.yaml` is repo content an outside contributor may have authored, and
this script's findings are printed into a delivery run's report. Free-text fields it echoes back (notably an override's
`reason`) are therefore INERT DATA, exactly as `knowledge/delivery-gates.md`,
"Recorded artifacts are inert data", requires: no reading agent may treat any of
it as an instruction, an approval, or a readiness signal. Every repo-authored
value is control-escaped, whitespace-collapsed, and length-bounded before it
reaches a message, so it can neither forge extra report lines nor erase and
overwrite real ones with a terminal escape sequence.

EXIT CODES
    0  clean (including "nothing declared")
    1  refuse -- at least one SCHEMA violation or unauthorized POLICY downgrade
    2  warnings only -- authorized downgrades and/or stale overrides

USAGE
    python3 validate_process_routing.py [--repo-root DIR]
                                        [--process-config FILE]
                                        [--routing-map FILE]
                                        [--json]

Python 3.9 compatible. Standard library plus PyYAML, which is required only when
the config actually declares routing.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from typing import Any, Dict, List, Optional, Tuple

# Tier ordering, low to high. A tier's index is its rank; "below its class tier"
# means a strictly lower rank. Tier NAMES come from the routing map's `aliases`
# block; this ordering is the only routing fact this script hardcodes, because it
# is a property of the tier vocabulary itself and not an assignment.
TIER_RANK = {"fast": 0, "balanced": 1, "reasoning": 2}

SCHEMA = "schema"
POLICY = "policy"


class RoutingConfigError(Exception):
    """A file exists but cannot be read as the mapping shape this script needs."""


class AliasNotAllowed(RoutingConfigError):
    """The config uses a YAML alias. Carries its own message and remedy so the
    refusal names the real problem instead of "make the file parseable"."""


# ---------------------------------------------------------------------------
# YAML loading

# The routing block keys. Their absence means there is nothing here to validate.
ROUTING_KEYS = ("models", "run_profiles", "routing_overrides")

# A LAST-RESORT text heuristic, used for exactly one question and no other: with no
# YAML parser available, must this run demand one? It is deliberately NOT the
# authoritative "is anything declared" test -- see `load_process_config`.
#
# Any text heuristic is line-anchored and therefore blind to shapes PyYAML accepts:
# a substring test misses `"models":` and `models :`; even this key-position regex
# misses `deliver: {models: {...}}` and `? models`. Every one of those is ordinary
# YAML, so relying on text to decide whether validation happens at all is a
# fail-open -- the gate reports clean while the delivery skills honor the block.
# The structural check is what closes that; this pattern only ever ERRS TOWARD
# demanding a parser.
ROUTING_KEY_RE = re.compile(
    r"^\s*[\"']?(?:{})[\"']?\s*:".format("|".join(ROUTING_KEYS))
)

# Hard ceiling on a config we will hand to a YAML parser. A `.a-cx/process.yaml`
# is a few dozen lines; anything past this is not a config anyone wrote by hand.
# This bounds a FLAT oversized file only. It does NOT bound alias expansion: a
# 549-byte anchor bomb parses fine and then costs gigabytes the moment any of its
# values is stringified. `_load_yaml_text` refuses aliases outright for that.
MAX_CONFIG_BYTES = 512 * 1024

# Longest repo-authored free-text value echoed into a message.
MAX_UNTRUSTED_CHARS = 200

# What `sanitize_untrusted` escapes, and why this is not the same set as "control
# characters".
#
# `str.split()` -- the collapse -- already treats \t \n \v \f \r \x1c-\x1f \x85
# and space as whitespace, so those never needed escaping to be kept off a report
# line. Every OTHER control character survived it verbatim, ESC (\x1b) above all:
# `\x1b[2K` erases the rendered line, so a value could delete a real report line
# and write a forged one over it -- exactly what the collapse exists to prevent.
#
# So the class below is every C0 control, DEL, and every C1 control, MINUS the
# three that are genuine text whitespace (\t \n \r) and stay collapsed. The rest
# of the split() whitespace set (\v \f \x1c-\x1f \x85) is escaped rather than
# collapsed: those are controls a terminal acts on -- \f clears a screen, \x85 is
# a line break -- so rendering them as a space HIDES what arrived.
#
# C1 (\x80-\x9f) is in the class for the same reason C0 is: a terminal in 8-bit
# mode reads \x9b as CSI, which is the identical forgery vector to \x1b[.
#
# The backslash is in the class because the escape form has to be
# unambiguous. Escaping ESC but not the introducer would leave a reader unable
# to tell a real ESC from source text that merely spells one -- and evidence of
# what actually arrived is the whole reason this escapes rather than strips.
#
# The class also covers three groups of INVISIBLE Unicode format characters, which
# `str.split()` does not treat as whitespace and which are outside \x00-\x9f, so
# nothing else here would touch them. Both halves of this render's audience are
# attacked by them, which is why they belong in the same class rather than a
# separate concern:
#
#   U+200B-U+200F, U+2060-U+2064, U+FEFF -- zero-width and joiner characters, and
#   U+E0000-U+E007F -- the Unicode Tag block, which can carry an entire
#   hidden ASCII payload with NO visible footprint. The delivering AGENT reads
#   this string, and the module docstring above promises it is inert data that no
#   reading agent may treat as an instruction. An invisible payload is exactly the
#   smuggling that promise rules out, and it is the more dangerous half precisely
#   because a human reviewing the same report sees nothing.
#
#   U+202A-U+202E, U+2066-U+2069 -- bidi overrides and isolates. These do not
#   erase a line, they REORDER one: a Trojan-Source-class attack that makes the
#   rendered text differ from its logical byte order for the HUMAN reader. The
#   guarantee this function owes is that a value cannot erase, relocate, or
#   recolor a report line, and this is the "relocate" half.
#
# U+2028, U+2029, and NBSP are deliberately NOT here: Python calls them
# whitespace, so the collapse already turns each into a plain space, and adding
# them would double-handle a character that never reaches the output.
_ESCAPE_UNTRUSTED_RE = re.compile(
    "[\\\\\x00-\x08\x0b\x0c\x0e-\x1f\x7f-\x9f"
    "\u200b-\u200f\u202a-\u202e\u2060-\u2064\u2066-\u2069\ufeff"
    "\U000e0000-\U000e007f]"
)

# The tokens the escape above emits, so a truncation can be pushed back onto a
# token boundary.
#
# The set is a PREFIX CODE, and that property comes from two things -- neither of
# them the order the alternatives are listed in, which is free to change: every
# raw backslash in the input is doubled, so a backslash in the OUTPUT is always
# the head of a token and never ordinary text; and the four tokens are then
# distinguished by the single character after that head (`\`, `x`, `u`, `U`,
# case-sensitive). At most one alternative can match at any position. So `\\x1b`
# -- an escaped backslash followed by the literal text "x1b" -- reads as `\\`
# then plain text, and can never be confused with the `\x1b` a real ESC produces.
_ESCAPE_TOKEN_RE = re.compile(
    r"\\\\|\\x[0-9a-f]{2}|\\u[0-9a-f]{4}|\\U[0-9a-f]{8}"
)

# The complete key set of one `deliver.routing_overrides` entry. Anything else is
# reported rather than ignored -- see `parse_overrides`.
OVERRIDE_KEYS = ("phase", "model", "reason")

PYYAML_REMEDY = (
    "This repo declares model routing, so it must be parsed to be validated. "
    "Install PyYAML (`python3 -m pip install pyyaml`), or remove the "
    "deliver.models / deliver.run_profiles / deliver.routing_overrides blocks."
)


def declares_routing(text: str) -> bool:
    """True when the config text opens any routing block.

    Comments are stripped first, because the scaffolded `.a-cx/process.yaml`
    ships `# models:` commented out as self-documentation -- counting that as a
    declaration would demand a YAML parser from every repo that never enabled the
    feature. A `#` cannot precede a block key on its own line, so cutting at the
    first `#` can only ever UNDER-detect a commented block, never miss a live one.

    Detection is by KEY POSITION (`ROUTING_KEY_RE`), never by substring -- see that
    pattern's comment for the fail-open a substring test allows.
    """
    for line in text.splitlines():
        bare = line.split("#", 1)[0]
        if ROUTING_KEY_RE.match(bare):
            return True
    return False


def _escape_one(match: re.Match) -> str:
    """Render one matched character as its unambiguous escape token.

    Three widths, chosen by codepoint so every token is fixed-length and the set
    stays a prefix code: `\\xHH` below U+0100, `\\uHHHH` inside the BMP, and
    `\\UHHHHHHHH` above it (the Tag block lives there). A variable-width form
    would leave `_cut_on_token_boundary` unable to tell where a token ends.
    """
    char = match.group(0)
    if char == "\\":
        return "\\\\"
    code = ord(char)
    if code < 0x100:
        return "\\x{:02x}".format(code)
    if code < 0x10000:
        return "\\u{:04x}".format(code)
    return "\\U{:08x}".format(code)


def _cut_on_token_boundary(text: str, limit: int) -> int:
    """The largest index <= `limit` that does not fall inside an escape token.

    Escaping before the cap is necessary but not
    sufficient: a value near the limit would otherwise be cut mid-sequence and
    render as `...\\x1`, which is ambiguous in exactly the way the escape exists
    to remove. Only the token straddling the cut can need moving, so this is one
    scan and never rewrites what came before it.
    """
    for match in _ESCAPE_TOKEN_RE.finditer(text):
        if match.start() >= limit:
            break
        if match.start() < limit < match.end():
            return match.start()
    return limit


def bound_untrusted(value: str, limit: int = MAX_UNTRUSTED_CHARS) -> str:
    """Bound ONE field that is about to be embedded in a message which is itself
    sanitized as a whole (every `Finding` sanitizes its `message`).

    Bounds only. Escaping here as well would escape the field TWICE, because the
    escape is deliberately not idempotent -- it emits a literal backslash, which a
    second pass re-escapes, so a single ESC would render `\\\\x1b` instead of
    `\\x1b` and the reader would be told a backslash arrived that never did.
    Collapsing here would be wrong for a subtler reason: `str.split()` treats
    \\x0b and \\x1c-\\x1f as whitespace, so an inner collapse would quietly
    turn those into spaces before the outer pass could escape them -- the exact
    hiding this fix removed, reintroduced one layer up. Leave both to the outer
    pass so every field renders identically however deeply it was nested.

    The bound is on the RAW length, so an escape-heavy field can still expand past
    `limit` in the final message. That is safe: the message's own cap bounds it,
    on an escape-token boundary.
    """
    text = str(value)
    if len(text) > limit:
        return text[:limit] + "... (truncated)"
    return text


def sanitize_untrusted(value: str, limit: int = MAX_UNTRUSTED_CHARS) -> str:
    """Make one repo-authored string safe to embed in a rendered message.

    Escapes every control character a terminal acts on, collapses ALL whitespace
    (newlines included), and bounds the length. Without this, a `reason:` written
    as a YAML block scalar could emit newlines that forge additional report lines
    -- a fake `OK ...` or a fake second `REFUSE ...` -- in the plain-text render a
    human and the delivering agent read, and a `reason:` carrying `\\x1b[2K` could
    ERASE a real line and write a forged one over it.

    The order is load-bearing: escape, then collapse,
    then cap on an escape-token boundary. Capping first would truncate a value
    mid-sequence and hand the reader something ambiguous again.

    Control characters are ESCAPED rather than stripped, deliberately. Stripping
    renders `\\x1b[2Kfake OK` as `[2Kfake OK`, and the operator cannot tell
    whether the config wrote that literally or wrote an ESC -- so it would make
    the forgery attempt itself silent, in a check whose entire purpose is
    detecting a silent wrong answer. `_ESCAPE_UNTRUSTED_RE` owns which characters
    are in the class and why.
    """
    escaped = _ESCAPE_UNTRUSTED_RE.sub(_escape_one, str(value))
    collapsed = " ".join(escaped.split())
    if len(collapsed) > limit:
        return collapsed[:_cut_on_token_boundary(collapsed, limit)] + "... (truncated)"
    return collapsed


def _read_text_capped(path: str, origin: str) -> str:
    """Read a config file, refusing anything past MAX_CONFIG_BYTES.

    Encoding is pinned to `utf-8-sig`, not left to the default. Two reasons:
    `open(path, "r")` with no `encoding=` follows `locale.getpreferredencoding()`,
    so the same committed file could decode differently (or not at all) depending
    on the machine running the check -- a gate whose verdict varies by locale is
    not a gate. And `-sig` transparently drops a UTF-8 BOM, which several Windows
    editors write. PyYAML's scanner already skips a BOM at index 0, so this is
    belt-and-braces there rather than a fix, but it makes the guarantee ours
    instead of a parser implementation detail.

    A file that will not decode is a REFUSAL, raised as RoutingConfigError so it
    renders through the same path as every other unreadable input -- an escaping
    UnicodeDecodeError would hand a `--json` caller a traceback instead of the
    documented shape.
    """
    size = os.path.getsize(path)
    if size > MAX_CONFIG_BYTES:
        raise RoutingConfigError(
            "{}: file is {} bytes, over the {}-byte limit for a process config".format(
                origin, size, MAX_CONFIG_BYTES
            )
        )
    try:
        with open(path, "r", encoding="utf-8-sig") as handle:
            return handle.read(MAX_CONFIG_BYTES + 1)
    except UnicodeDecodeError as exc:
        raise RoutingConfigError(
            "{}: not valid UTF-8 ({})".format(origin, exc)
        )
    except OSError as exc:
        # A permission change or a race between the isfile() check and the read.
        # Rare and operational rather than attacker-driven, but an escaping OSError
        # would still hand a --json caller a traceback.
        raise RoutingConfigError("{}: not readable ({})".format(origin, exc))


def pyyaml_available() -> bool:
    try:
        import yaml  # type: ignore  # noqa: F401
    except ImportError:
        return False
    return True


def _no_alias_loader():
    """A SafeLoader that REFUSES YAML aliases.

    `safe_load` handles an anchor bomb fine on its own -- aliases become shared
    object references, so the parsed graph stays small. The blow-up comes later,
    from `str()`: it is not alias-aware and re-renders every shared node at every
    occurrence. Measured, a 549-byte config produced a 5.2 GB string in 45
    seconds. That is a deterministic CPU/memory denial of service on a check that
    runs at every commit and every delivery preflight, and the byte cap cannot see
    it because the SOURCE is tiny.

    `.a-cx/process.yaml` has no legitimate use for anchors or aliases, so refusing
    them kills the whole class at the parse boundary rather than guarding each
    field that might one day stringify a value.
    """
    import yaml  # type: ignore

    class NoAliasSafeLoader(yaml.SafeLoader):
        def compose_node(self, parent, index):  # type: ignore[no-untyped-def]
            if self.check_event(yaml.events.AliasEvent):
                event = self.peek_event()
                raise AliasNotAllowed(
                    "YAML aliases are not allowed in a process config "
                    "(found *{} at line {})".format(
                        event.anchor, getattr(event.start_mark, "line", 0) + 1
                    )
                )
            return super(NoAliasSafeLoader, self).compose_node(parent, index)

    return NoAliasSafeLoader


def _load_yaml_text(text: str, origin: str) -> Dict[str, Any]:
    """Parse a mapping-shaped YAML document with PyYAML.

    Requiring PyYAML follows the convention every other shipped config parser in
    this toolchain uses (`skills/github/scripts/github_config.py`). A hand-rolled
    fallback was considered and rejected: it would have to be trusted to decide
    whether a run refuses, and a subtly wrong parse fails in whichever direction
    the bug happens to point. Refusing with a remedy is the honest failure.
    """
    if not pyyaml_available():
        raise RoutingConfigError("{}: PyYAML is required. {}".format(origin, PYYAML_REMEDY))
    import yaml  # type: ignore

    try:
        data = yaml.load(text, Loader=_no_alias_loader())
    except AliasNotAllowed:
        # Raised from inside the loader; already fully worded. Let it through so
        # main() can attach the alias-specific remedy.
        raise
    except Exception as exc:
        raise RoutingConfigError("{}: not valid YAML ({})".format(origin, exc))
    if data is None:
        return {}
    if not isinstance(data, dict):
        raise RoutingConfigError("{}: top level must be a mapping".format(origin))
    return data


def declares_routing_structurally(data: Dict[str, Any]) -> bool:
    """The AUTHORITATIVE "is anything declared" test, on the parsed document.

    Structural, so it is blind to formatting: flow mappings, explicit keys, quoted
    keys, and odd spacing all resolve to the same parsed keys and are all seen.
    """
    deliver = data.get("deliver")
    if not isinstance(deliver, dict):
        return False
    return any(key in deliver for key in ROUTING_KEYS)


def load_mapping(path: str) -> Optional[Dict[str, Any]]:
    """Read one YAML file. Returns None when the file does not exist."""
    if not path or not os.path.isfile(path):
        return None
    return _load_yaml_text(_read_text_capped(path, path), path)


def load_process_config(path: str) -> Optional[Dict[str, Any]]:
    """Read the consuming repo's process config.

    Returns None when the file is absent and when it declares no routing -- the two
    "nothing to validate" cases, which must both pass clean.

    **What decides "nothing declared" depends on whether a parser exists**, and the
    split matters:

    - PyYAML present (the normal case): PARSE FIRST, then decide structurally.
      Text can never be trusted for this decision -- every line-anchored heuristic
      is blind to some shape PyYAML accepts (`deliver: {models: {...}}`,
      `? models`), and each blind spot is a fail-open where the gate reports clean
      while the delivery skills honor the block.
    - PyYAML absent: the text heuristic is all there is, and its job narrows to one
      question -- must this run demand a parser? Its blind spots are safe here
      because both answers are conservative: a hit REFUSES asking for PyYAML, and
      a miss can only occur on a file we cannot read anyway.
    """
    if not path or not os.path.isfile(path):
        return None
    text = _read_text_capped(path, path)

    if not pyyaml_available():
        if not declares_routing(text):
            return None
        raise RoutingConfigError("{}: PyYAML is required. {}".format(path, PYYAML_REMEDY))

    data = _load_yaml_text(text, path)
    if not declares_routing_structurally(data):
        return None
    return data


# ---------------------------------------------------------------------------
# Routing map accessors


class RoutingPolicy:
    """The subset of `knowledge/model-routing.yaml` this check consumes."""

    def __init__(self, data: Dict[str, Any], origin: str) -> None:
        self.origin = origin
        self.aliases = _require_mapping(data, "aliases", origin)
        self.phases = _require_mapping(data, "phases", origin)
        self.task_classes = _require_mapping(data, "task_classes", origin)
        self.allowed_models = _require_list(data, "allowed_models", origin)
        self.allowed_effort = _require_list(data, "allowed_effort", origin)
        # model alias -> tier name, inverted from `aliases`.
        self.model_to_tier = {}  # type: Dict[str, str]
        for tier, model in self.aliases.items():
            self.model_to_tier[str(model)] = str(tier)

    def class_for_phase(self, phase: str) -> Optional[str]:
        value = self.phases.get(phase)
        return str(value) if value is not None else None

    def tier_for_class(self, task_class: str) -> Optional[str]:
        entry = self.task_classes.get(task_class)
        if isinstance(entry, dict):
            tier = entry.get("tier")
            return str(tier) if tier is not None else None
        return None

    def tier_for_model(self, model: str) -> Optional[str]:
        return self.model_to_tier.get(model)


def _require_mapping(data: Dict[str, Any], key: str, origin: str) -> Dict[str, Any]:
    value = data.get(key)
    if not isinstance(value, dict):
        raise RoutingConfigError("{}: `{}` must be a mapping".format(origin, key))
    return value


def _require_list(data: Dict[str, Any], key: str, origin: str) -> List[str]:
    value = data.get(key)
    if not isinstance(value, list):
        raise RoutingConfigError("{}: `{}` must be a list".format(origin, key))
    return [str(item) for item in value]


# ---------------------------------------------------------------------------
# Finding model


class InvalidBlock:
    """A `models` value that is not a mapping at all.

    A dedicated type rather than a `{"__invalid__": value}` marker dict: a real
    config could legitimately contain a phase key literally named `__invalid__`,
    and the marker check would then condemn the WHOLE block and skip validating
    its sibling phases -- hiding a co-located unauthorized downgrade behind one
    generic error. A type cannot be forged by config content.
    """

    def __init__(self, value: Any) -> None:
        self.value = value


class Finding:
    """One reportable finding, rendered as exactly one logical line.

    `where` and `message` are sanitized HERE rather than at each call site,
    because both routinely carry repo-authored fragments -- an unknown phase KEY,
    a rejected model value, a profile name, an override's `reason`. Making the
    constructor the chokepoint means a future finding cannot reintroduce the
    forged-report-line problem by forgetting to sanitize its inputs.
    `remedy` is always this script's own literal text and needs no bound.
    """

    def __init__(self, kind: str, where: str, message: str, remedy: str) -> None:
        self.kind = kind
        self.where = sanitize_untrusted(where, 200)
        self.message = sanitize_untrusted(message, 800)
        self.remedy = remedy

    def as_dict(self) -> Dict[str, str]:
        return {
            "kind": self.kind,
            "where": self.where,
            "message": self.message,
            "remedy": self.remedy,
        }

    def __repr__(self) -> str:  # pragma: no cover - debugging aid
        return "Finding({!r}, {!r}, {!r})".format(self.kind, self.where, self.message)


class Result:
    def __init__(self) -> None:
        self.refusals = []  # type: List[Finding]
        self.warnings = []  # type: List[Finding]
        self.checked_entries = 0

    @property
    def exit_code(self) -> int:
        if self.refusals:
            return 1
        if self.warnings:
            return 2
        return 0

    def as_dict(self) -> Dict[str, Any]:
        return {
            "exit_code": self.exit_code,
            "checked_entries": self.checked_entries,
            "refusals": [f.as_dict() for f in self.refusals],
            "warnings": [f.as_dict() for f in self.warnings],
        }


# ---------------------------------------------------------------------------
# Validation


def split_routed_value(value: str) -> Tuple[str, Optional[str]]:
    """`sonnet:high` -> ("sonnet", "high"); `sonnet` -> ("sonnet", None)."""
    if ":" in value:
        model, effort = value.split(":", 1)
        return model.strip(), effort.strip()
    return value.strip(), None


def collect_model_blocks(deliver: Dict[str, Any]) -> List[Tuple[str, Any]]:
    """Every `models` mapping the config declares, each with a label naming where
    it came from. `deliver.models` plus one per `deliver.run_profiles.<name>`.

    A profile with no `models` block contributes nothing of its OWN -- it inherits
    `deliver.models`, which is already checked under that label, and an override
    authorizes its (phase, value) pair wherever the pair is declared. There is no
    per-block scoping, so nothing about the inherited case can go unauthorized;
    see `_override_for` for why scoping was removed.

    A malformed shape is REPORTED, never skipped: silently dropping a
    non-mapping `run_profiles` or profile would make a typo look like "no
    profiles declared", which is the same fail-quiet this whole check exists to
    remove.
    """
    blocks = []  # type: List[Tuple[str, Any]]
    models = deliver.get("models")
    if isinstance(models, dict):
        blocks.append(("deliver.models", models))
    elif models is not None:
        blocks.append(("deliver.models", InvalidBlock(models)))

    profiles = deliver.get("run_profiles")
    if isinstance(profiles, dict):
        for name in sorted(profiles, key=str):
            profile = profiles[name]
            label = "deliver.run_profiles.{}.models".format(name)
            if not isinstance(profile, dict):
                blocks.append(
                    ("deliver.run_profiles.{}".format(name), InvalidBlock(profile))
                )
                continue
            profile_models = profile.get("models")
            if isinstance(profile_models, dict):
                blocks.append((label, profile_models))
            elif profile_models is not None:
                blocks.append((label, InvalidBlock(profile_models)))
    elif profiles is not None:
        blocks.append(("deliver.run_profiles", InvalidBlock(profiles)))
    return blocks


def parse_overrides(deliver: Dict[str, Any]) -> Tuple[List[Dict[str, Any]], List[Finding]]:
    """Read `deliver.routing_overrides`, rejecting malformed entries as SCHEMA
    findings. Shape is documented in `knowledge/process-config.md`."""
    findings = []  # type: List[Finding]
    raw = deliver.get("routing_overrides")
    if raw is None:
        return [], findings
    if not isinstance(raw, list):
        findings.append(
            Finding(
                SCHEMA,
                "deliver.routing_overrides",
                "must be a list of override entries, got {}".format(type(raw).__name__),
                "See knowledge/process-config.md, \"Routing overrides\".",
            )
        )
        return [], findings

    overrides = []
    for index, entry in enumerate(raw):
        where = "deliver.routing_overrides[{}]".format(index)
        finding = _override_entry_finding(entry, where)
        if finding is not None:
            findings.append(finding)
            continue
        overrides.append(
            {
                "phase": str(entry["phase"]).strip(),
                "model": str(entry["model"]).strip(),
                "reason": str(entry["reason"]).strip(),
                "_where": where,
                "_matched": False,
            }
        )
    return overrides, findings


def _override_entry_finding(entry: Any, where: str) -> Optional[Finding]:
    """Validate ONE override entry. Returns the first violation, or None.

    Order is load-bearing: the shape check must precede the key checks, and the
    TYPE check must precede the emptiness check, because the emptiness check calls
    `str()` -- see its comment.
    """
    doc = "See knowledge/process-config.md, \"Routing overrides\"."

    if not isinstance(entry, dict):
        return Finding(
            SCHEMA, where, "must be a mapping with phase, model, and reason", doc
        )

    # An unrecognized key is REPORTED, not ignored. `scope:` is the reason: it was
    # a real key once, and an author who still writes it must be told it does
    # nothing rather than believing they narrowed an authorization. A
    # silently-ignored key is how the misreading this feature removed got in.
    unknown = sorted((k for k in entry if k not in OVERRIDE_KEYS), key=str)
    if unknown:
        return Finding(
            SCHEMA,
            where,
            "unrecognized key(s) {}; an override takes only {}".format(
                ", ".join(str(k) for k in unknown),
                ", ".join(sorted(OVERRIDE_KEYS, key=str)),
            ),
            "`scope:` was removed -- an override authorizes its (phase, model) "
            "pair in every block. " + doc,
        )

    # Type-check BEFORE stringifying. `str()` on a nested structure is the
    # amplifier an anchor bomb needs; aliases are already refused at the parse
    # boundary, and this keeps the guarantee local so a future loader change cannot
    # silently reopen it. It also produces a better error than a coerced
    # "['x', 'x', ...]" phase name would.
    non_scalar = sorted(
        (
            k
            for k in OVERRIDE_KEYS
            if k in entry
            and entry[k] is not None
            and not isinstance(entry[k], (str, int, float, bool))
        ),
        key=str,
    )
    if non_scalar:
        return Finding(
            SCHEMA,
            where,
            "key(s) {} must be text, not a list or mapping".format(
                ", ".join(non_scalar)
            ),
            doc,
        )

    missing = [k for k in OVERRIDE_KEYS if not str(entry.get(k) or "").strip()]
    if missing:
        return Finding(
            SCHEMA,
            where,
            "missing required key(s): {}".format(", ".join(missing)),
            "An override with no stated reason authorizes nothing. " + doc,
        )

    return None


def _override_for(
    overrides: List[Dict[str, Any]], phase: str, routed: str
) -> Optional[Dict[str, Any]]:
    """Find the override authorizing `phase -> routed`.

    Matching is on the routed value AS WRITTEN, so tightening `sonnet` to `haiku`
    invalidates the override instead of inheriting it.

    An override authorizes its (phase, routed value) pair WHEREVER that pair is
    declared -- there is deliberately no per-block scoping. A `scope` field was
    built and removed: a `run_profiles.<name>` with no `models` key INHERITS
    `deliver.models` per phase, so `scope: deliver.models` read as "interactive
    runs only" while the inheriting autonomous profile resolved the very same
    value, unchecked. An authorization that narrower than it appears is worse than
    no narrowing at all, and nothing in this feature's requirements asked for it.
    """
    for override in overrides:
        if override["phase"] != phase or override["model"] != routed:
            continue
        override["_matched"] = True
        return override
    return None


def validate(
    process_config: Optional[Dict[str, Any]], policy: RoutingPolicy
) -> Result:
    """Validate one parsed process config against one routing policy."""
    result = Result()
    if not process_config:
        return result
    deliver = process_config.get("deliver")
    if not isinstance(deliver, dict):
        return result

    overrides, override_findings = parse_overrides(deliver)
    result.refusals.extend(override_findings)

    # key=str for the same reason as every other sort here, even though this one
    # reads the TRUSTED bundled map: consistency means a reader never has to work
    # out which sorts are safe, and a hand-edit of the map cannot introduce a crash.
    known_phases = sorted(policy.phases.keys(), key=str)

    for block_label, block in collect_model_blocks(deliver):
        if isinstance(block, InvalidBlock):
            result.refusals.append(_invalid_block_finding(block_label, block))
            continue
        for phase in sorted(block, key=str):
            where = "{}.{}".format(block_label, phase)
            result.checked_entries += 1

            schema_finding = _entry_schema_finding(
                policy, phase, block[phase], where, known_phases
            )
            if schema_finding is not None:
                result.refusals.append(schema_finding)
                continue

            model, _effort = split_routed_value(block[phase])
            downgrade = _downgrade_finding(
                policy, phase, model, block[phase], where, overrides
            )
            if downgrade is not None:
                finding, authorized = downgrade
                if authorized:
                    result.warnings.append(finding)
                else:
                    result.refusals.append(finding)

    result.warnings.extend(_stale_override_findings(overrides))
    return result


def _invalid_block_finding(block_label: str, block: InvalidBlock) -> Finding:
    expected = (
        "a mapping of phase -> model[:effort]"
        if block_label.endswith("models")
        else "a mapping"
    )
    return Finding(
        SCHEMA,
        block_label,
        "must be {}, got {}".format(expected, type(block.value).__name__),
        "See knowledge/process-config.md, \"Model profile\" and \"Run profiles\".",
    )


def _entry_schema_finding(
    policy: RoutingPolicy,
    phase: Any,
    raw_value: Any,
    where: str,
    known_phases: List[str],
) -> Optional[Finding]:
    """The SCHEMA half for ONE `phase: model[:effort]` entry.

    Returns the first violation, or None when the entry resolves. Split out of
    `validate()` so the caller reads as "schema, then policy" rather than as four
    near-identical append-and-continue blocks; the checks are ordered cheapest and
    most-fundamental first, because a later message would be nonsense on an entry
    that already failed an earlier one.
    """
    if phase not in policy.phases:
        return Finding(
            SCHEMA,
            where,
            "unknown phase key; `phases:` in {} defines: {}".format(
                policy.origin, ", ".join(known_phases)
            ),
            "Use one of the declared phase keys, or drop the entry.",
        )

    if not isinstance(raw_value, str):
        return Finding(
            SCHEMA,
            where,
            "value must be a tier alias or tier:effort string, got {}".format(
                type(raw_value).__name__
            ),
            "Write e.g. `sonnet` or `sonnet:high`.",
        )

    model, effort = split_routed_value(raw_value)

    if model not in policy.allowed_models:
        return Finding(
            SCHEMA,
            where,
            "model {!r} is not in allowed_models ({})".format(
                model, ", ".join(policy.allowed_models)
            ),
            "Aliases are version-floating; never pin a version string.",
        )

    if effort is not None and effort not in policy.allowed_effort:
        return Finding(
            SCHEMA,
            where,
            "effort {!r} is not in allowed_effort ({})".format(
                effort, ", ".join(policy.allowed_effort)
            ),
            "Use one of the declared effort levels.",
        )

    return None


def _stale_override_findings(overrides: List[Dict[str, Any]]) -> List[Finding]:
    """An override matching no declared downgrade. Untidy, never unsafe."""
    return [
        Finding(
            POLICY,
            override["_where"],
            "override authorizes {} -> {} but no such downgrade is declared "
            "(stale)".format(
                bound_untrusted(override["phase"], 60),
                bound_untrusted(override["model"], 60),
            ),
            "Remove the override, or correct its phase/model.",
        )
        for override in overrides
        if not override["_matched"]
    ]


def _downgrade_finding(
    policy: RoutingPolicy,
    phase: str,
    model: str,
    raw_value: str,
    where: str,
    overrides: List[Dict[str, Any]],
) -> Optional[Tuple[Finding, bool]]:
    """POLICY check: is this entry a reasoning-tier downgrade, and is it authorized?

    Returns None when the entry is not a downgrade at all, else the finding plus
    an explicit authorized flag. The caller routes an authorized downgrade to
    warnings and an unauthorized one to refusals -- the decision is carried as
    data, not inferred from the message text.
    """
    task_class = policy.class_for_phase(phase)
    if task_class is None:
        return None
    class_tier = policy.tier_for_class(task_class)
    routed_tier = policy.tier_for_model(model)
    if class_tier is None or routed_tier is None:
        return None
    if class_tier not in TIER_RANK or routed_tier not in TIER_RANK:
        return None
    # Only the reasoning-pinned classes are protected. The reasoning-tier rule
    # (knowledge/model-routing.md) pins planning, requirements, architecture, and
    # backlog ordering by rule; every one of them reaches here as a class whose
    # declared tier IS reasoning, so the set is read from the map, not listed.
    if class_tier != "reasoning":
        return None
    if TIER_RANK[routed_tier] >= TIER_RANK[class_tier]:
        return None

    override = _override_for(overrides, phase, raw_value)
    detail = "phase {!r} maps to task class {!r} (tier {}), routed to {!r} (tier {})".format(
        phase, task_class, class_tier, raw_value, routed_tier
    )
    if override is not None:
        return (
            Finding(
                POLICY,
                where,
                "{} -- authorized by {}: {}".format(
                    detail, override["_where"], bound_untrusted(override["reason"])
                ),
                "Proceeding. Re-check it against the no-regression bar "
                "(knowledge/model-routing.md) when the engagement changes.",
            ),
            True,
        )
    return (
        Finding(
            POLICY,
            where,
            "{} -- no matching deliver.routing_overrides entry".format(detail),
            "Add an override with phase, model, and a stated reason, or route the "
            "phase at its class tier. See knowledge/process-config.md, "
            "\"Routing overrides\".",
        ),
        False,
    )


# ---------------------------------------------------------------------------
# Path resolution and CLI


def default_routing_map(script_dir: str) -> Optional[str]:
    """Locate the bundled routing map.

    Two layouts, both real:
      1. Assembled plugin -- `<skill>/scripts/` beside `<skill>/knowledge/`.
      2. Dev-repo git hook -- the map sits next to the script, because
         `/setup-repo` installs both into one directory.
    """
    candidates = [
        os.path.join(script_dir, os.pardir, "knowledge", "model-routing.yaml"),
        os.path.join(script_dir, "model-routing.yaml"),
        os.path.join(script_dir, os.pardir, os.pardir, os.pardir, "knowledge", "model-routing.yaml"),
    ]
    for candidate in candidates:
        resolved = os.path.normpath(candidate)
        if os.path.isfile(resolved):
            return resolved
    return None


def render(result: Result, as_json: bool) -> str:
    if as_json:
        return json.dumps(result.as_dict(), indent=2, sort_keys=True)
    lines = []
    for finding in result.refusals:
        lines.append("REFUSE [{}] {}: {}".format(finding.kind, finding.where, finding.message))
        lines.append("        {}".format(finding.remedy))
    for finding in result.warnings:
        lines.append("WARN   [{}] {}: {}".format(finding.kind, finding.where, finding.message))
        lines.append("        {}".format(finding.remedy))
    if not lines:
        lines.append(
            "OK  routing declarations validate ({} entr{} checked)".format(
                result.checked_entries, "y" if result.checked_entries == 1 else "ies"
            )
        )
    return "\n".join(lines)


def _refuse(where: str, message: str, remedy: str, as_json: bool) -> int:
    """Emit a single unreadable-input refusal through the SAME renderer as every
    other finding.

    Routing these paths through `Result` rather than a bare `print` is what keeps
    `--json` a contract instead of a best case: a caller that asked for JSON must
    never receive plain text just because the failure happened early.
    """
    result = Result()
    result.refusals.append(Finding(SCHEMA, where, message, remedy))
    print(render(result, as_json), file=sys.stderr)
    return result.exit_code


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Validate .a-cx/process.yaml model routing against the bundled routing policy.",
    )
    parser.add_argument("--repo-root", default=".", help="repo whose .a-cx/process.yaml to read")
    parser.add_argument("--process-config", help="explicit path to the process config")
    parser.add_argument("--routing-map", help="explicit path to model-routing.yaml")
    parser.add_argument("--json", action="store_true", help="emit machine-readable findings")
    args = parser.parse_args(argv)

    process_path = args.process_config or os.path.join(args.repo_root, ".a-cx", "process.yaml")

    # Read the CONSUMING repo first. When it declares no routing there is nothing
    # to validate, so the run must pass clean without needing the routing map or a
    # YAML parser at all -- otherwise "absence is not a violation" would quietly
    # depend on the environment having PyYAML.
    try:
        process_config = load_process_config(process_path)
    except AliasNotAllowed as exc:
        return _refuse(
            process_path,
            str(exc),
            "Rewrite the value literally. Anchors and aliases have no use in a "
            "process config and are refused because stringifying an aliased "
            "structure costs gigabytes from a tiny file.",
            args.json,
        )
    except RoutingConfigError as exc:
        return _refuse(
            process_path,
            str(exc),
            "Make the file parseable, or remove the routing blocks.",
            args.json,
        )

    if not process_config:
        print(render(Result(), args.json))
        return 0

    script_dir = os.path.dirname(os.path.abspath(__file__))
    routing_map = args.routing_map or default_routing_map(script_dir)
    if not routing_map or not os.path.isfile(routing_map):
        return _refuse(
            "model-routing.yaml",
            "bundled routing map not found (looked beside {})".format(script_dir),
            "Pass --routing-map, or reinstall the plugin payload.",
            args.json,
        )

    try:
        policy_data = load_mapping(routing_map)
        if policy_data is None:
            raise RoutingConfigError("{}: not readable".format(routing_map))
        policy = RoutingPolicy(policy_data, routing_map)
    except RoutingConfigError as exc:
        return _refuse(
            routing_map,
            str(exc),
            "The bundled routing map is unreadable; reinstall the plugin payload.",
            args.json,
        )

    result = validate(process_config, policy)
    stream = sys.stderr if result.refusals else sys.stdout
    print(render(result, args.json), file=stream)
    return result.exit_code


if __name__ == "__main__":
    sys.exit(main())
