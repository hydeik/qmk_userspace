# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit tests of firmware_default, the firmware defaults of the Vial settings,
# on the unit-test framework of the firmware source. Run them with `make test`
# from the repository root.
#
# The test compiles the real firmware_default.c and the real qmk_settings.c of
# vial-qmk, the module that holds the Vial settings. The rest of Vial is not
# part of the test build: fakes.c stands in for the stored settings and for
# the EEPROM check of VIA.

# The firmware sources this test runs on, by their names in
# firmware_sources.json. `make test` reads this line itself, so keep it a
# single assignment of literal names. Only vial-qmk has qmk_settings.c.
TEST_FIRMWARE_SOURCES = vial-qmk

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# The sources of users/hydeik/ are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../../features)

SRC += firmware_default.c fakes.c

# What builddefs/build_vial.mk of vial-qmk adds for QMK_SETTINGS = yes, as far
# as the test build can take it: the Vial settings and the per-key tap-hold
# hooks that read them.
AUTO_SHIFT_ENABLE = yes
SRC += $(QUANTUM_DIR)/qmk_settings.c
OPT_DEFS += -DQMK_SETTINGS \
    -DAUTO_SHIFT_NO_SETUP -DAUTO_SHIFT_REPEAT_PER_KEY -DAUTO_SHIFT_NO_AUTO_REPEAT_PER_KEY \
    -DPERMISSIVE_HOLD_PER_KEY -DHOLD_ON_OTHER_KEY_PRESS_PER_KEY -DQUICK_TAP_TERM_PER_KEY -DRETRO_TAPPING_PER_KEY \
    -DCHORDAL_HOLD -DFLOW_TAP_TERM=321
