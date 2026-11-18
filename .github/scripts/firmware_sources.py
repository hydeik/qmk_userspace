#!/usr/bin/env python3
"""Assign the build targets in qmk.json to the firmware sources they are built against.

Each keymap family declares its firmware source in firmware_sources.json. A
build target's keymap name is its keymap family, matched exactly.

Commands (run from the repository root):

    matrix           Print the CI build matrix as a `matrix=<json>` line, with
                     one entry per keymap family that has build targets.
    select FAMILY    Rewrite qmk.json so that it holds only the build targets
                     of FAMILY, and print them.

Both commands fail if firmware_sources.json is malformed or if any build
target's keymap has no entry in it.
"""

import json
import re
import sys

SOURCES_FILE = "firmware_sources.json"
TARGETS_FILE = "qmk.json"

REQUIRED_KEYS = ("repo", "branch", "ref")
COMMIT_SHA = re.compile(r"[0-9a-f]{40}")


def load_json(path):
    with open(path) as f:
        return json.load(f)


def check_sources(sources):
    """Return the problems found in the firmware source declarations."""
    if not isinstance(sources, dict):
        return [f"{SOURCES_FILE}: expected an object keyed by keymap family"]

    errors = []
    for family, source in sources.items():
        if not isinstance(source, dict):
            errors.append(f"{SOURCES_FILE}: {family}: expected an object")
            continue
        for key in REQUIRED_KEYS:
            if not isinstance(source.get(key), str) or not source[key]:
                errors.append(f"{SOURCES_FILE}: {family}: missing `{key}`")
        ref = source.get("ref")
        if isinstance(ref, str) and ref and not COMMIT_SHA.fullmatch(ref):
            errors.append(
                f"{SOURCES_FILE}: {family}: `ref` must be a full commit SHA, got `{ref}`"
            )
    return errors


def check_targets(targets, sources):
    """Return the problems found in assigning build targets to firmware sources."""
    return [
        f"{TARGETS_FILE}: build target {keyboard}:{keymap} has no firmware source: "
        f"keymap family `{keymap}` is not declared in {SOURCES_FILE}"
        for keyboard, keymap in targets
        if keymap not in sources
    ]


def load():
    """Load and validate both files, returning (sources, qmk_json)."""
    sources = load_json(SOURCES_FILE)
    qmk_json = load_json(TARGETS_FILE)

    errors = check_sources(sources)
    if not errors:
        errors = check_targets(qmk_json["build_targets"], sources)
    if errors:
        raise SystemExit("\n".join(errors))

    return sources, qmk_json


def build_matrix(sources, targets):
    """Return one matrix entry per keymap family that has build targets."""
    used = {keymap for _, keymap in targets}
    return [
        {"family": family, "repo": source["repo"], "ref": source["ref"]}
        for family, source in sources.items()
        if family in used
    ]


def cmd_matrix():
    sources, qmk_json = load()
    include = build_matrix(sources, qmk_json["build_targets"])
    if not include:
        raise SystemExit(f"{TARGETS_FILE}: no build targets")
    print("matrix=" + json.dumps({"include": include}, separators=(",", ":")))


def cmd_select(family):
    sources, qmk_json = load()
    if family not in sources:
        raise SystemExit(f"{SOURCES_FILE}: no keymap family `{family}`")

    # Targets are [keyboard, keymap] pairs.
    qmk_json["build_targets"] = [
        target for target in qmk_json["build_targets"] if target[1] == family
    ]
    if not qmk_json["build_targets"]:
        raise SystemExit(
            f"{TARGETS_FILE}: no build targets for keymap family `{family}`"
        )

    with open(TARGETS_FILE, "w") as f:
        json.dump(qmk_json, f, indent=4)

    for keyboard, keymap in qmk_json["build_targets"]:
        print(f"{keyboard}:{keymap}")


def main(argv):
    if argv == ["matrix"]:
        cmd_matrix()
    elif len(argv) == 2 and argv[0] == "select":
        cmd_select(argv[1])
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main(sys.argv[1:])
