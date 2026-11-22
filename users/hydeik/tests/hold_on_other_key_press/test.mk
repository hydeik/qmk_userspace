# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the hydeik keymap family with Hold On Other Key Press, the one
# tap-hold setting of the family, on the unit-test frameworks of both firmware
# sources. Run them with `make test` from the repository root.
#
# Like tests/target_layer_smart_mod/, the test compiles the real hydeik.c with
# the real layout.h. It differs in its config.h, which defines
# HOLD_ON_OTHER_KEY_PRESS as users/hydeik/config.h does, and so covers what
# the setting is there for. Vial is not part of the test build.

# The firmware sources this test runs on, by their names in
# firmware_sources.json. `make test` reads this line itself, so keep it a
# single assignment of literal names.
TEST_FIRMWARE_SOURCES = qmk vial-qmk

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# The sources of users/hydeik/ are named by basename and found through VPATH.
# hydeik.c includes its features as "features/<name>.h".
VPATH += $(abspath $(TEST_PATH)/../..)
VPATH += $(abspath $(TEST_PATH)/../../features)

# The test-only glue of tests/target_layer_smart_mod/, test_glue.c and
# test_glue.h, is used as it is: the layers of layout.h as a table.
VPATH += $(abspath $(TEST_PATH)/../target_layer_smart_mod)

# The sources of users/hydeik/rules.mk for a keymap without Vial.
SRC += hydeik.c typing_rules.c key_typed.c smart_mod.c target_layer.c test_glue.c

# Features of users/hydeik/rules.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
REPEAT_KEY_ENABLE = yes
