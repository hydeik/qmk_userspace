# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the smart_mod state machine, on the unit-test frameworks of
# both firmware sources. Run them with `make test` from the repository root.
#
# The test compiles the real smart_mod.c, the real key_typed.c that it uses,
# and a test-only glue. hydeik.c, target_layer.c and layout.h are not part of
# the test build, and neither is Vial.

# The firmware sources this test runs on, by their names in
# firmware_sources.json. `make test` reads this line itself, so keep it a
# single assignment of literal names.
TEST_FIRMWARE_SOURCES = qmk vial-qmk

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# The sources of users/hydeik/ are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += smart_mod.c key_typed.c test_glue.c

# Features of users/hydeik/rules.mk that the scenarios depend on. Combos and
# tap dances are on only in a keymap with Vial.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
INTROSPECTION_KEYMAP_C = test_keymap_tables.c
