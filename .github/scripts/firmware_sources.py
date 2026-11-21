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
    tests [FAMILY]   Print one `<keymap family> <test directory>` line per
                     unit test and firmware source it runs on, the firmware
                     source being named by its keymap family. With FAMILY,
                     print only the runs on the firmware source of FAMILY.
                     Fail if there is none.
    test-matrix      Print the CI test matrix as a `test_matrix=<json>` line,
                     with one entry per firmware source that unit tests run
                     on.

`matrix`, `select` and `test-matrix` are used by CI, `fetch`, `locate` and
`tests` by the Makefile for local builds and unit tests.

A unit test is a directory users/<owner>/tests/<test>/ with a test.mk, where
<owner> is a keymap family or the common core. Its test.mk names the firmware
sources it runs on, each by the keymap family that declares it:

    TEST_FIRMWARE_SOURCES = hydeik hydeik_vial

The line is read as text, not evaluated by make. A test of a keymap family
may leave it out and then runs on the firmware source of that family; any
other test must have it.

Every command fails if firmware_sources.json is malformed. `matrix` and
`select` also fail if any build target's keymap has no entry in it.
"""

import glob
import json
import os
import re
import subprocess
import sys

SOURCES_FILE = "firmware_sources.json"
TARGETS_FILE = "qmk.json"
FIRMWARE_DIR = ".firmware"

TEST_MAKEFILES = "users/*/tests/*/test.mk"
TEST_SOURCES_VARIABLE = "TEST_FIRMWARE_SOURCES"
TEST_SOURCES_LINE = re.compile(
    rf"^[ \t]*{TEST_SOURCES_VARIABLE}[ \t]*(?:::?|\?|\+)?=([^#\n]*)", re.MULTILINE
)

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


def test_sources(test_makefile, sources):
    """Return the keymap families whose firmware sources a unit test runs on."""
    owner = test_makefile.split("/")[1]
    with open(test_makefile) as f:
        lines = TEST_SOURCES_LINE.findall(f.read())

    if len(lines) > 1:
        raise SystemExit(
            f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is set more than once"
        )
    if not lines:
        if owner not in sources:
            raise SystemExit(
                f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is not set, and "
                f"`{owner}` is not a keymap family whose firmware source it "
                f"could default to (declared: {', '.join(sources)})"
            )
        return [owner]

    families = lines[0].split()
    if not families:
        raise SystemExit(f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is empty")
    for family in families:
        if family not in sources:
            raise SystemExit(
                f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` names `{family}`, "
                f"which is not a keymap family declared in {SOURCES_FILE} "
                f"(declared: {', '.join(sources)})"
            )
    return families


def test_runs(sources):
    """Return (keymap family, test directory) for every run of a unit test.

    The runs are ordered by firmware source, as declared, then by directory.
    """
    tests = [
        (os.path.dirname(test_makefile), test_sources(test_makefile, sources))
        for test_makefile in sorted(glob.glob(TEST_MAKEFILES))
    ]
    return [
        (family, test_dir)
        for family in sources
        for test_dir, families in tests
        if family in families
    ]


def cmd_tests(family=None):
    sources = load_sources()
    runs = test_runs(sources)
    if family is not None:
        declared_source(family)
        runs = [run for run in runs if run[0] == family]
    if not runs:
        on = f" that run on the firmware source of `{family}`" if family else ""
        raise SystemExit(f"No unit tests{on} found in {TEST_MAKEFILES}.")
    for run_family, test_dir in runs:
        print(run_family, os.path.realpath(test_dir))


def cmd_test_matrix():
    sources = load_sources()
    used = {family for family, _ in test_runs(sources)}
    include = [
        {"family": family, "repo": source["repo"], "ref": source["ref"]}
        for family, source in sources.items()
        if family in used
    ]
    if not include:
        raise SystemExit(f"No unit tests found in {TEST_MAKEFILES}.")
    print("test_matrix=" + json.dumps({"include": include}, separators=(",", ":")))


def main(argv):
    if argv == ["matrix"]:
        cmd_matrix()
    elif len(argv) == 2 and argv[0] == "select":
        cmd_select(argv[1])
    elif argv == ["fetch"]:
        cmd_fetch()
    elif len(argv) == 2 and argv[0] == "locate":
        cmd_locate(argv[1])
    elif len(argv) in (1, 2) and argv[0] == "tests":
        cmd_tests(*argv[1:])
    elif argv == ["test-matrix"]:
        cmd_test_matrix()
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except FileNotFoundError as error:
        raise SystemExit(f"{error.filename}: not found")
