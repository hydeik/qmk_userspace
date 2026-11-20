#!/usr/bin/env python3
"""Read firmware_sources.json for CI and for local builds.

Each keymap family declares its firmware source in firmware_sources.json. A
build target's keymap name is its keymap family, matched exactly.

Commands (run from the repository root):

    matrix           Print the CI build matrix as a `matrix=<json>` line, with
                     one entry per keymap family that has build targets.
    select FAMILY    Rewrite qmk.json so that it holds only the build targets
                     of FAMILY, and print them.
    fetch            Clone or update the firmware source of every keymap
                     family into .firmware/FAMILY/ and check out its pinned
                     commit, submodules included.
    locate FAMILY    Print the path of the firmware checkout of FAMILY. Fail
                     if it, or one of its submodules, is missing or not at
                     the pinned commit.
    source FAMILY    Print the firmware source of FAMILY as `repo=<repo>` and
                     `ref=<ref>` lines.

`matrix`, `select` and `source` are used by CI, `fetch` and `locate` by the
Makefile for local builds and unit tests.

Every command fails if firmware_sources.json is malformed. `matrix` and
`select` also fail if any build target's keymap has no entry in it.
"""

import json
import os
import re
import subprocess
import sys

SOURCES_FILE = "firmware_sources.json"
TARGETS_FILE = "qmk.json"
FIRMWARE_DIR = ".firmware"

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


def load_sources():
    """Load and validate the firmware source declarations."""
    sources = load_json(SOURCES_FILE)
    errors = check_sources(sources)
    if errors:
        raise SystemExit("\n".join(errors))
    return sources


def load():
    """Load and validate both files, returning (sources, qmk_json)."""
    sources = load_sources()
    qmk_json = load_json(TARGETS_FILE)

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


def firmware_checkout(family):
    """Return the absolute path of the firmware checkout of a keymap family."""
    return os.path.realpath(os.path.join(FIRMWARE_DIR, family))


def git(checkout, *args, capture=False):
    """Run git in a firmware checkout and return its output if captured.

    Git is kept from looking for a repository above the checkout, so a missing
    or broken checkout can never make a command act on this repository.
    """
    result = subprocess.run(
        ["git", "-C", checkout, *args],
        check=True,
        env=dict(os.environ, GIT_CEILING_DIRECTORIES=os.path.dirname(checkout)),
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.DEVNULL if capture else None,
        text=True,
    )
    return result.stdout if capture else None


def is_repository(checkout):
    try:
        git(checkout, "rev-parse", "--git-dir", capture=True)
    except (subprocess.CalledProcessError, NotADirectoryError):
        return False
    return True


def checked_out_commit(checkout):
    """Return the commit a firmware checkout is at, or None if there is none."""
    if not os.path.isdir(checkout):
        return None
    try:
        return git(checkout, "rev-parse", "--verify", "HEAD", capture=True).strip()
    except subprocess.CalledProcessError:
        return None


def stale_submodules(checkout):
    """Return the submodules that are missing or not at the commit the checkout pins."""
    status = git(checkout, "submodule", "status", "--recursive", capture=True)
    return [line.split()[1] for line in status.splitlines() if line[0] in "-+U"]


def fetch_source(family, source):
    """Put the firmware checkout of a keymap family at its pinned commit.

    Like the CI checkout, this fetches only the pinned commit, without history.
    """
    checkout = firmware_checkout(family)
    url = f"https://github.com/{source['repo']}.git"
    ref = source["ref"]
    print(f"==> {family}: {source['repo']} at {ref}", flush=True)

    os.makedirs(checkout, exist_ok=True)
    if not is_repository(checkout):
        git(checkout, "-c", "init.defaultBranch=main", "init", "--quiet")
    has_origin = "origin" in git(checkout, "remote", capture=True).split()
    git(checkout, "remote", "set-url" if has_origin else "add", "origin", url)

    if checked_out_commit(checkout) != ref:
        git(checkout, "fetch", "--depth", "1", "origin", ref)
        git(checkout, "checkout", "--quiet", "--detach", ref)

    git(checkout, "submodule", "sync", "--quiet", "--recursive")
    git(checkout, "submodule", "update", "--init", "--recursive", "--depth", "1")


def cmd_fetch():
    for family, source in load_sources().items():
        try:
            fetch_source(family, source)
        except subprocess.CalledProcessError as error:
            raise SystemExit(
                f"{firmware_checkout(family)}: `{' '.join(error.cmd)}` failed"
            )


def declared_source(family):
    """Return the validated firmware source declaration of a keymap family."""
    sources = load_sources()
    if family not in sources:
        raise SystemExit(
            f"`{family}` is not a keymap family declared in {SOURCES_FILE} "
            f"(declared: {', '.join(sources)})"
        )
    return sources[family]


def cmd_source(family):
    source = declared_source(family)
    print(f"repo={source['repo']}")
    print(f"ref={source['ref']}")


def cmd_locate(family):
    checkout = firmware_checkout(family)
    ref = declared_source(family)["ref"]
    commit = checked_out_commit(checkout)
    if commit is None:
        raise SystemExit(
            f"{checkout}: no firmware checkout for keymap family `{family}`. "
            "Run `make firmware`."
        )
    if commit != ref:
        raise SystemExit(
            f"{checkout}: at {commit}, but keymap family `{family}` is pinned to "
            f"{ref}. Run `make firmware`."
        )
    stale = stale_submodules(checkout)
    if stale:
        raise SystemExit(
            f"{checkout}: submodules missing or not at their pinned commit "
            f"({', '.join(stale)}). Run `make firmware`."
        )
    print(checkout)


def main(argv):
    if argv == ["matrix"]:
        cmd_matrix()
    elif len(argv) == 2 and argv[0] == "select":
        cmd_select(argv[1])
    elif argv == ["fetch"]:
        cmd_fetch()
    elif len(argv) == 2 and argv[0] == "locate":
        cmd_locate(argv[1])
    elif len(argv) == 2 and argv[0] == "source":
        cmd_source(argv[1])
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except FileNotFoundError as error:
        raise SystemExit(f"{error.filename}: not found")
