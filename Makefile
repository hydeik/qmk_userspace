# Local builds against the pinned firmware sources.
#
#   make firmware                         Fetch the firmware source of every keymap family
#                                         into .firmware/ at the commit pinned in
#                                         firmware_sources.json.
#   make <keyboard>:<keymap>              Build one build target.
#   make <keyboard>:<keymap>:flash        Build and flash one build target.
#   make test                             Build and run every unit test, each on the
#                                         firmware sources it names.
#   make test FIRMWARE_SOURCE=<family>    Build and run only the unit tests that run on
#                                         the firmware source of one keymap family.
#
# The keymap name is the keymap family, which selects the firmware source. The
# build stops if that firmware checkout is missing or not at its pinned commit.
# The unit tests check every firmware checkout they run on the same way.

.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
FIRMWARE_SOURCES := python3 .github/scripts/firmware_sources.py

USAGE := Usage: make firmware | make test [FIRMWARE_SOURCE=<family>] | make <keyboard>:<keymap>[:flash]

# The keymap name of the goal being built, which is its keymap family.
keymap_family = $(word 2,$(subst :, ,$@))

.PHONY: help firmware test FORCE

help:
	echo "$(USAGE)"

firmware:
	cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) fetch

# Unit tests live in users/<owner>/tests/<test>/, one directory with a test.mk
# per test, where <owner> is a keymap family or the common core
# (hydeik_common). They run natively on this machine, on the unit-test
# framework of a firmware source.
#
# A test names the firmware sources it runs on in its test.mk, each by the
# keymap family that declares it in firmware_sources.json:
#
#     TEST_FIRMWARE_SOURCES = hydeik hydeik_vial
#
# A test of a keymap family may leave the line out, and then runs on the
# firmware source of that family. A test of the common core must have it.
#
# FIRMWARE_SOURCE, a keymap family, limits the run to the tests that run on
# its firmware source, and then that firmware checkout is the only one needed.
# CI uses it to run the tests once per firmware source.
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
# <keymap family> <test directory> pairs for the same reason.
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
		family=$$1; test_dir=$$2; shift 2; \
		checkout=$$($(FIRMWARE_SOURCES) locate "$$family") || exit 1; \
		name=$$(basename "$$test_dir"); \
		owner=$$(basename "$$(dirname "$$(dirname "$$test_dir")")"); \
		output="$${owner}_$$name"; \
		echo "==> $$owner/$$name on the firmware source of $$family"; \
		$(MAKE) -r -R -C "$$checkout" -f builddefs/build_test.mk \
			TEST="$$name" TEST_OUTPUT="$$output" TEST_PATH="$$test_dir" FULL_TESTS="$$name" \
			SILENT=false $$flags || exit 1; \
		binary="$$checkout/.build/test/$$output.elf"; \
		if ! "$$binary" --gtest_list_tests | grep -q '^  '; then \
			echo "$$owner/$$name on $$family: the test binary contains no tests." >&2; failed=1; \
		elif ! "$$binary"; then \
			failed=1; \
		fi; \
	done; \
	[ -z "$$failed" ]

FORCE:

# Never try to remake this file with the catch-all rule below.
Makefile: ;

# Any other goal is <keyboard>:<keymap>[:<action>] and is handed to the make of
# the firmware source of its keymap family. FORCE runs the rule even when a
# file or directory with the name of the goal exists.
%: FORCE
	$(if $(keymap_family),,$(error Unknown target `$@`. $(USAGE)))
	checkout=$$(cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) locate "$(keymap_family)") || exit 1; \
	$(MAKE) -C "$$checkout" "$@" QMK_USERSPACE="$(QMK_USERSPACE)"
