# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the target_layer state machine, on the unit-test frameworks of
# both firmware sources. Run them with `make test` from the repository root.
#
# The test compiles the real target_layer.c, the real common.c for the Repeat
# Key rule, and a test-only glue with a test-only table of entries. No keymap
# family is part of the test build.

# The firmware sources this test runs on, each named by the keymap family
# that declares it in firmware_sources.json. `make test` reads this line
# itself, so keep it a single assignment of literal names.
TEST_FIRMWARE_SOURCES = hydeik hydeik_vial

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# Common-core sources are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../..)
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += common.c target_layer.c test_glue.c

# Features of users/hydeik_common/common.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
REPEAT_KEY_ENABLE = yes
