include $(QMK_USERSPACE)/users/hydeik_common/common.mk

SRC += hydeik_vial.c
# Probe: firmware_default exists only with Vial.
ifeq ($(strip $(VIAL_ENABLE)),yes)
    SRC += features/firmware_default.c
endif
SRC += features/smart_mod.c

COMBO_ENABLE ?= yes
ENCODER_ENABLE ?= no
ENCODER_MAP_ENABLE ?= no
TAP_DANCE_ENABLE ?= yes
