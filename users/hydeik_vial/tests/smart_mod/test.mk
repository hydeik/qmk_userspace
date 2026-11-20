# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the smart_mod state machine, on the unit-test framework of the
# firmware source. Run them with `make test` from the repository root.
#
# The test compiles the real smart_mod.c and a test-only glue. hydeik_vial.c
# and layout.h are not part of the test build, and neither is Vial.

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# User sources are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += smart_mod.c test_glue.c

# Features of users/hydeik_common/common.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes

# Features of users/hydeik_vial/rules.mk that the scenarios depend on.
TAP_DANCE_ENABLE = yes
INTROSPECTION_KEYMAP_C = tap_dance_actions.c
