# Local builds against the pinned firmware sources.
#
#   make firmware                         Fetch every firmware source into
#                                         .firmware/<firmware source>/ at the commit
#                                         pinned in firmware_sources.json.
#   make <keyboard>:<keymap>              Build one build target.
#   make <keyboard>:<keymap>:flash        Build and flash one build target.
#   make test                             Build and run every unit test, each on the
#                                         firmware sources it names.
#   make test FIRMWARE_SOURCE=<source>    Build and run only the unit tests that run on
#                                         one firmware source, `qmk` or `vial-qmk`.
#
# The keymap name of a build target selects its firmware source: the one that
# lists the keymap under `keymaps` in firmware_sources.json. The build stops if
# no firmware source lists the keymap, or more than one does, and if the
# firmware checkout is missing or not at its pinned commit. The unit tests
# check every firmware checkout they run on the same way.

.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
FIRMWARE_SOURCES := python3 .github/scripts/firmware_sources.py

USAGE := Usage: make firmware | make test [FIRMWARE_SOURCE=qmk|vial-qmk] | make <keyboard>:<keymap>[:flash]

# The keymap name of the goal being built.
keymap = $(word 2,$(subst :, ,$@))

.PHONY: help firmware test FORCE

help:
	echo "$(USAGE)"

firmware:
	cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) fetch

# Unit tests live in users/<owner>/tests/<test>/, one directory with a test.mk
# per test. They run natively on this machine, on the unit-test framework of a
# firmware source.
#
# Every test names the firmware sources it runs on in its test.mk, by their
# names in firmware_sources.json:
#
#     TEST_FIRMWARE_SOURCES = qmk vial-qmk
#
# There is no default: a test.mk without the line fails the run.
#
# FIRMWARE_SOURCE, the name of a firmware source, limits the run to the tests
# that run on it, and then that firmware checkout is the only one needed. CI
# uses it to run the tests once per firmware source.
FIRMWARE_SOURCE :=

# Apple clang, which is what `gcc` is on macOS, warns where gcc does not and
# rejects narrowing conversions in the test sources of the firmware source.
# These flags are added by the `test` recipe, only when `gcc` is clang.
CLANG_TEST_FLAGS := ALLOW_WARNINGS=yes EXTRAFLAGS=-Wno-c++11-narrowing

# Each test is built by the test makefile of the firmware source, called
# directly with the test directory of this repository, so nothing is placed in
# the firmware checkout but its own build output. Every firmware checkout the
# run needs is checked before anything is built. A test binary that fails, or
# that contains no test at all, fails the goal.
#
# That test makefile puts the test directory on VPATH and in `include`, so the
# path of this repository must not contain spaces. `runs` is a flat list of
# <firmware source> <test directory> pairs for the same reason.
test:
	cd "$(QMK_USERSPACE)" || exit 1; \
	runs=$$($(FIRMWARE_SOURCES) tests $(FIRMWARE_SOURCE)) || exit 1; \
	set -- $$runs; \
	while [ $$# -gt 0 ]; do \
		$(FIRMWARE_SOURCES) locate "$$1" >/dev/null || exit 1; \
		shift 2; \
	done; \
	flags=; \
	if gcc --version 2>/dev/null | grep -q clang; then flags="$(CLANG_TEST_FLAGS)"; fi; \
	failed=; \
	set -- $$runs; \
	while [ $$# -gt 0 ]; do \
		source=$$1; test_dir=$$2; shift 2; \
		checkout=$$($(FIRMWARE_SOURCES) locate "$$source") || exit 1; \
		name=$$(basename "$$test_dir"); \
		owner=$$(basename "$$(dirname "$$(dirname "$$test_dir")")"); \
		output="$${owner}_$$name"; \
		echo "==> $$owner/$$name on $$source"; \
		$(MAKE) -r -R -C "$$checkout" -f builddefs/build_test.mk \
			TEST="$$name" TEST_OUTPUT="$$output" TEST_PATH="$$test_dir" FULL_TESTS="$$name" \
			SILENT=false $$flags || exit 1; \
		binary="$$checkout/.build/test/$$output.elf"; \
		if ! "$$binary" --gtest_list_tests | grep -q '^  '; then \
			echo "$$owner/$$name on $$source: the test binary contains no tests." >&2; failed=1; \
		elif ! "$$binary"; then \
			failed=1; \
		fi; \
	done; \
	[ -z "$$failed" ]

FORCE:

# Never try to remake this file with the catch-all rule below.
Makefile: ;

# Any other goal is <keyboard>:<keymap>[:<action>] and is handed to the make of
# the firmware source that lists its keymap. FORCE runs the rule even when a
# file or directory with the name of the goal exists.
%: FORCE
	$(if $(keymap),,$(error Unknown target `$@`. $(USAGE)))
	cd "$(QMK_USERSPACE)" || exit 1; \
	source=$$($(FIRMWARE_SOURCES) source-of "$(keymap)") || exit 1; \
	checkout=$$($(FIRMWARE_SOURCES) locate "$$source") || exit 1; \
	$(MAKE) -C "$$checkout" "$@" QMK_USERSPACE="$(QMK_USERSPACE)"
