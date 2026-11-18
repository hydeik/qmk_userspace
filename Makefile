# Local builds against the pinned firmware sources.
#
#   make firmware                         Fetch the firmware source of every keymap family
#                                         into .firmware/ at the commit pinned in
#                                         firmware_sources.json.
#   make <keyboard>:<keymap>              Build one build target.
#   make <keyboard>:<keymap>:flash        Build and flash one build target.
#
# The keymap name is the keymap family, which selects the firmware source. The
# build stops if that firmware checkout is missing or not at its pinned commit.

.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
FIRMWARE_SOURCES := python3 .github/scripts/firmware_sources.py

USAGE := Usage: make firmware | make <keyboard>:<keymap>[:flash]

# The keymap name of the goal being built, which is its keymap family.
keymap_family = $(word 2,$(subst :, ,$@))

.PHONY: help firmware FORCE

help:
	echo "$(USAGE)"

firmware:
	cd "$(QMK_USERSPACE)" && $(FIRMWARE_SOURCES) fetch

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
