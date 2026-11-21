# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of target_layer together with smart_mod, as the hydeik_vial
# keymap family wires them, on the unit-test framework of the firmware source
# of that family. Run them with `make test` from the repository root.
#
# The test compiles the real hydeik_vial.c with the real layout.h, so the two
# state machines are wired as in the firmware and the keymap of the tests is
# the keymap of the family. Vial is not part of the test build: test_glue.c
# stands in for firmware_default, the one module here that needs it.

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# User sources are named by basename and found through VPATH. hydeik_vial.c
# includes its features as "features/<name>.h".
VPATH += $(abspath $(TEST_PATH)/../..)
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += hydeik_vial.c smart_mod.c test_glue.c

# The common core is named by its path from the userspace root, in `#include`
# and in SRC alike, as in the firmware build.
VPATH += $(abspath $(TEST_PATH)/../../../..)

# The common-core sources of users/hydeik_common/common.mk.
SRC += users/hydeik_common/common.c
SRC += users/hydeik_common/features/key_typed.c
SRC += users/hydeik_common/features/target_layer.c

# Features of users/hydeik_common/common.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
LAYER_LOCK_ENABLE = yes
REPEAT_KEY_ENABLE = yes
