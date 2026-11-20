# Local builds against the pinned firmware sources.
#
#   make firmware                         Fetch the firmware source of every keymap family
#                                         into .firmware/ at the commit pinned in
#                                         firmware_sources.json.
#   make <keyboard>:<keymap>              Build one build target.
#   make <keyboard>:<keymap>:flash        Build and flash one build target.
#   make test                             Build and run the unit tests of the hydeik_vial
#                                         keymap family.
#
# The keymap name is the keymap family, which selects the firmware source. The
# build stops if that firmware checkout is missing or not at its pinned commit.
# The unit tests check the firmware checkout of their keymap family the same way.

.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
FIRMWARE_SOURCES := python3 .github/scripts/firmware_sources.py

USAGE := Usage: make firmware | make test | make <keyboard>:<keymap>[:flash]

# The keymap name of the goal being built, which is its keymap family.
keymap_family = $(word 2,$(subst :, ,$@))

.PHONY: help firmware test FORCE

help:
	echo "$(USAGE)"

firmware:
	cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) fetch

# Unit tests live in users/<keymap family>/tests/<test>/, one directory with a
# test.mk per test. They run natively on this machine, on the unit-test
# framework of the firmware source of that keymap family.
TEST_FAMILY := hydeik_vial
TEST_DIRS := $(sort $(patsubst %/test.mk,%,$(wildcard $(QMK_USERSPACE)/users/$(TEST_FAMILY)/tests/*/test.mk)))

# Apple clang, which is what `gcc` is on macOS, warns where gcc does not and
# rejects narrowing conversions in the test sources of the firmware source.
# These flags are added by the `test` recipe, only when `gcc` is clang.
CLANG_TEST_FLAGS := ALLOW_WARNINGS=yes EXTRAFLAGS=-Wno-c++11-narrowing

# Each test is built by the test makefile of the firmware source, called
# directly with the test directory of this repository, so nothing is placed in
# the firmware checkout but its own build output. A test binary that fails, or
# that contains no test at all, fails the goal.
#
# That test makefile puts the test directory on VPATH and in `include`, so the
# path of this repository must not contain spaces.
test:
	checkout=$$(cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) locate "$(TEST_FAMILY)") || exit 1; \
	[ -n "$(TEST_DIRS)" ] || { echo "No tests found in users/$(TEST_FAMILY)/tests/." >&2; exit 1; }; \
	flags=; \
	if gcc --version 2>/dev/null | grep -q clang; then flags="$(CLANG_TEST_FLAGS)"; fi; \
	failed=; \
	for test_dir in $(TEST_DIRS); do \
		name=$$(basename "$$test_dir"); \
		output="$(TEST_FAMILY)_$$name"; \
		$(MAKE) -r -R -C "$$checkout" -f builddefs/build_test.mk \
			TEST="$$name" TEST_OUTPUT="$$output" TEST_PATH="$$test_dir" FULL_TESTS="$$name" \
			SILENT=false $$flags || exit 1; \
		binary="$$checkout/.build/test/$$output.elf"; \
		if ! "$$binary" --gtest_list_tests | grep -q '^  '; then \
			echo "$$name: the test binary contains no tests." >&2; failed=1; \
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
