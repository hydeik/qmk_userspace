#!/usr/bin/env python3
"""Read firmware_sources.json for CI and for local builds.

firmware_sources.json declares the firmware sources by name, `qmk` (upstream
QMK) and `vial-qmk`. Each firmware source is pinned to a commit and lists, in
`keymaps`, the names of the keymaps that are built against it. The keymap
name of a build target, matched exactly, selects its firmware source: it must
be in the `keymaps` list of exactly one firmware source. Nothing is inferred
from the form of a keymap name, and there is no default firmware source.

Commands (run from the repository root):

    matrix           Print the CI build matrix as a `matrix=<json>` line, with
                     one entry per firmware source that has build targets.
    select SOURCE    Rewrite qmk.json so that it holds only the build targets
                     whose keymap is built against the firmware source
                     SOURCE, and print them.
    fetch            Clone or update every firmware source into
                     .firmware/SOURCE/ and check out its pinned commit,
                     submodules included.
    locate SOURCE    Print the path of the firmware checkout of the firmware
                     source SOURCE. Fail if it, or one of its submodules, is
                     missing or not at the pinned commit.
    source-of KEYMAP Print the name of the firmware source that the keymap
                     KEYMAP is built against. Fail if no firmware source
                     lists the keymap, or more than one does.
    tests [SOURCE]   Print one `<firmware source> <test directory>` line per
                     unit test and firmware source it runs on. With SOURCE,
                     print only the runs on that firmware source. Fail if
                     there is none.
    test-matrix      Print the CI test matrix as a `test_matrix=<json>` line,
                     with one entry per firmware source that unit tests run
                     on.

`matrix`, `select` and `test-matrix` are used by CI, `fetch`, `locate`,
`source-of` and `tests` by the Makefile for local builds and unit tests.

A unit test is a directory users/<owner>/tests/<test>/ with a test.mk. Its
test.mk must name the firmware sources the test runs on:

    TEST_FIRMWARE_SOURCES = qmk vial-qmk

The line is read as text, not evaluated by make. There is no default: a
test.mk without the line is an error.

Every command fails if firmware_sources.json is malformed. `matrix` and
`select` also fail if the keymap of any build target is listed by no firmware
source, or by more than one.
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
KEYMAPS_KEY = "keymaps"
COMMIT_SHA = re.compile(r"[0-9a-f]{40}")


def load_json(path):
    with open(path) as f:
        return json.load(f)


def check_keymaps(name, keymaps):
    """Return the problems found in the `keymaps` list of a firmware source."""
    if not isinstance(keymaps, list):
        return [
            f"{SOURCES_FILE}: {name}: `{KEYMAPS_KEY}` must be a list of keymap names"
        ]

    errors = []
    reported = set()
    for keymap in keymaps:
        if not isinstance(keymap, str) or not keymap:
            errors.append(
                f"{SOURCES_FILE}: {name}: `{KEYMAPS_KEY}` must hold keymap names, "
                f"got {json.dumps(keymap)}"
            )
        elif keymaps.count(keymap) > 1 and keymap not in reported:
            reported.add(keymap)
            errors.append(
                f"{SOURCES_FILE}: {name}: `{KEYMAPS_KEY}` lists keymap `{keymap}` "
                "more than once"
            )
    return errors


def check_sources(sources):
    """Return the problems found in the firmware source declarations."""
    if not isinstance(sources, dict):
        return [f"{SOURCES_FILE}: expected an object keyed by firmware source"]

    errors = []
    for name, source in sources.items():
        if not isinstance(source, dict):
            errors.append(f"{SOURCES_FILE}: {name}: expected an object")
            continue
        for key in REQUIRED_KEYS:
            if not isinstance(source.get(key), str) or not source[key]:
                errors.append(f"{SOURCES_FILE}: {name}: missing `{key}`")
        ref = source.get("ref")
        if isinstance(ref, str) and ref and not COMMIT_SHA.fullmatch(ref):
            errors.append(
                f"{SOURCES_FILE}: {name}: `ref` must be a full commit SHA, got `{ref}`"
            )
        if KEYMAPS_KEY not in source:
            errors.append(f"{SOURCES_FILE}: {name}: missing `{KEYMAPS_KEY}`")
        else:
            errors.extend(check_keymaps(name, source[KEYMAPS_KEY]))
    return errors


def firmware_source_of(keymap, sources):
    """Return (firmware source, None) for a keymap, or (None, the problem).

    A keymap is built against the one firmware source that lists it. Nothing
    is inferred from its name, and there is no default.
    """
    listed_by = [
        name for name, source in sources.items() if keymap in source[KEYMAPS_KEY]
    ]
    if not listed_by:
        return None, (
            f"keymap `{keymap}` has no firmware source: it is in no "
            f"`{KEYMAPS_KEY}` list of {SOURCES_FILE}"
        )
    if len(listed_by) > 1:
        return None, (
            f"keymap `{keymap}` has more than one firmware source: it is in the "
            f"`{KEYMAPS_KEY}` lists of {', '.join(listed_by)} in {SOURCES_FILE}"
        )
    return listed_by[0], None


def check_targets(targets, sources):
    """Return the problems found in assigning build targets to firmware sources."""
    errors = []
    for keyboard, keymap in targets:
        _, problem = firmware_source_of(keymap, sources)
        if problem:
            errors.append(
                f"{TARGETS_FILE}: build target {keyboard}:{keymap}: {problem}"
            )
    return errors


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


def matrix_entry(name, source):
    return {"firmware_source": name, "repo": source["repo"], "ref": source["ref"]}


def build_matrix(sources, targets):
    """Return one matrix entry per firmware source that has build targets."""
    used = {keymap for _, keymap in targets}
    return [
        matrix_entry(name, source)
        for name, source in sources.items()
        if used.intersection(source[KEYMAPS_KEY])
    ]


def cmd_matrix():
    sources, qmk_json = load()
    include = build_matrix(sources, qmk_json["build_targets"])
    if not include:
        raise SystemExit(f"{TARGETS_FILE}: no build targets")
    print("matrix=" + json.dumps({"include": include}, separators=(",", ":")))


def declared_source(name, sources):
    """Return the declaration of a firmware source, which must be declared."""
    if name not in sources:
        raise SystemExit(
            f"`{name}` is not a firmware source declared in {SOURCES_FILE} "
            f"(declared: {', '.join(sources)})"
        )
    return sources[name]


def cmd_select(name):
    sources, qmk_json = load()
    keymaps = declared_source(name, sources)[KEYMAPS_KEY]

    # Targets are [keyboard, keymap] pairs.
    qmk_json["build_targets"] = [
        target for target in qmk_json["build_targets"] if target[1] in keymaps
    ]
    if not qmk_json["build_targets"]:
        raise SystemExit(
            f"{TARGETS_FILE}: no build targets for firmware source `{name}`"
        )

    with open(TARGETS_FILE, "w") as f:
        json.dump(qmk_json, f, indent=4)

    for keyboard, keymap in qmk_json["build_targets"]:
        print(f"{keyboard}:{keymap}")


def cmd_source_of(keymap):
    name, problem = firmware_source_of(keymap, load_sources())
    if problem:
        raise SystemExit(problem[0].upper() + problem[1:] + ".")
    print(name)


def firmware_checkout(name):
    """Return the absolute path of the firmware checkout of a firmware source."""
    return os.path.realpath(os.path.join(FIRMWARE_DIR, name))


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


def fetch_source(name, source):
    """Put the firmware checkout of a firmware source at its pinned commit.

    Like the CI checkout, this fetches only the pinned commit, without history.
    """
    checkout = firmware_checkout(name)
    url = f"https://github.com/{source['repo']}.git"
    ref = source["ref"]
    print(f"==> {name}: {source['repo']} at {ref}", flush=True)

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
    for name, source in load_sources().items():
        try:
            fetch_source(name, source)
        except subprocess.CalledProcessError as error:
            raise SystemExit(
                f"{firmware_checkout(name)}: `{' '.join(error.cmd)}` failed"
            )


def cmd_locate(name):
    checkout = firmware_checkout(name)
    ref = declared_source(name, load_sources())["ref"]
    commit = checked_out_commit(checkout)
    if commit is None:
        raise SystemExit(
            f"{checkout}: no firmware checkout for firmware source `{name}`. "
            "Run `make firmware`."
        )
    if commit != ref:
        raise SystemExit(
            f"{checkout}: at {commit}, but firmware source `{name}` is pinned to "
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
    """Return the names of the firmware sources a unit test runs on."""
    with open(test_makefile) as f:
        lines = TEST_SOURCES_LINE.findall(f.read())

    if len(lines) > 1:
        raise SystemExit(
            f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is set more than once"
        )
    if not lines:
        raise SystemExit(
            f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is not set. Every unit "
            "test must name the firmware sources it runs on "
            f"(declared: {', '.join(sources)})"
        )

    names = lines[0].split()
    if not names:
        raise SystemExit(f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` is empty")
    for name in names:
        if name not in sources:
            raise SystemExit(
                f"{test_makefile}: `{TEST_SOURCES_VARIABLE}` names `{name}`, "
                f"which is not a firmware source declared in {SOURCES_FILE} "
                f"(declared: {', '.join(sources)})"
            )
    return names


def test_runs(sources):
    """Return (firmware source, test directory) for every run of a unit test.

    The runs are ordered by firmware source, as declared, then by directory.
    """
    tests = [
        (os.path.dirname(test_makefile), test_sources(test_makefile, sources))
        for test_makefile in sorted(glob.glob(TEST_MAKEFILES))
    ]
    return [
        (name, test_dir)
        for name in sources
        for test_dir, names in tests
        if name in names
    ]


def cmd_tests(name=None):
    sources = load_sources()
    runs = test_runs(sources)
    if name is not None:
        declared_source(name, sources)
        runs = [run for run in runs if run[0] == name]
    if not runs:
        on = f" that run on firmware source `{name}`" if name else ""
        raise SystemExit(f"No unit tests{on} found in {TEST_MAKEFILES}.")
    for run_source, test_dir in runs:
        print(run_source, os.path.realpath(test_dir))


def cmd_test_matrix():
    sources = load_sources()
    used = {name for name, _ in test_runs(sources)}
    include = [
        matrix_entry(name, source) for name, source in sources.items() if name in used
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
    elif len(argv) == 2 and argv[0] == "source-of":
        cmd_source_of(argv[1])
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
