# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of target_layer together with smart_mod, as the hydeik keymap
# family wires them, on the unit-test frameworks of both firmware sources. Run
# them with `make test` from the repository root.
#
# The test compiles the real hydeik.c with the real layout.h, so the two state
# machines are wired as in the firmware and the layers of the tests are the
# layers of the family. Vial is not part of the test build, and neither are
# the sources that exist only with Vial, vial_hooks.c and firmware_default.c.

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

# The sources of users/hydeik/rules.mk for a keymap without Vial.
SRC += hydeik.c typing_rules.c key_typed.c smart_mod.c target_layer.c test_glue.c

# Features of users/hydeik/rules.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
MOUSEKEY_ENABLE = yes
REPEAT_KEY_ENABLE = yes
