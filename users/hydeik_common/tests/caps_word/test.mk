# Copyright Hidekazu Ikeno <@hydeik>
# SPDX-License-Identifier: GPL-2.0+

# Unit test of the common core's Caps Word rule, on the unit-test frameworks
# of both firmware sources. Run it with `make test` from the repository root.
#
# It is the minimal test that proves a common-core test builds and passes on
# both firmware sources. The test compiles the real common.c.

# The firmware sources this test runs on, each named by the keymap family
# that declares it in firmware_sources.json. `make test` reads this line
# itself, so keep it a single assignment of literal names.
TEST_FIRMWARE_SOURCES = hydeik hydeik_vial

# This directory is outside the firmware checkout, so the sources found here
# have absolute paths. Let make resolve them.
VPATH += /

# Common-core sources are named by basename and found through VPATH.
VPATH += $(abspath $(TEST_PATH)/../..)

SRC += common.c

# Features of users/hydeik_common/common.mk that the scenarios depend on.
CAPS_WORD_ENABLE = yes
REPEAT_KEY_ENABLE = yes
