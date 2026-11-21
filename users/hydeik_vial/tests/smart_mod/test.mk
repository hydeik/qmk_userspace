# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of the smart_mod state machine, on the unit-test framework of the
# firmware source. Run them with `make test` from the repository root.
#
# The test compiles the real smart_mod.c, the common-core sources it uses and
# a test-only glue. hydeik_vial.c and layout.h are not part of the test build,
# and neither is Vial.

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# User sources are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += smart_mod.c test_glue.c

# The common core is named by its path from the userspace root, in `#include`
# and in SRC alike, as in the firmware build.
VPATH += $(abspath $(TEST_PATH)/../../../..)

# Common-core sources of users/hydeik_common/common.mk that smart_mod.c uses.
SRC += users/hydeik_common/features/key_typed.c

# Features of users/hydeik_common/common.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes

# Features of users/hydeik_vial/rules.mk that the scenarios depend on.
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
INTROSPECTION_KEYMAP_C = test_keymap_tables.c
