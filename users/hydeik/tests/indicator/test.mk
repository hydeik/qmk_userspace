# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the logic of the indicators, on the unit-test frameworks of
# both firmware sources. Run them with `make test` from the repository root.
#
# The test compiles the real indicator.c with the real smart_mod.c and
# target_layer.c, whose state it shows, the real key_typed.c that those two
# use, and a test-only glue with a test-only table of entries. hydeik.c and
# layout.h are not part of the test build, and neither are Vial and RGB
# Matrix: the logic knows nothing of either.

# The firmware sources this test runs on, by their names in
# firmware_sources.json. `make test` reads this line itself, so keep it a
# single assignment of literal names.
TEST_FIRMWARE_SOURCES = qmk vial-qmk

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# The sources of users/hydeik/ are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += indicator.c smart_mod.c target_layer.c key_typed.c test_glue.c

# Features of users/hydeik/rules.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
