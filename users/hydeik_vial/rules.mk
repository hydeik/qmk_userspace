include $(QMK_USERSPACE)/users/hydeik_common/common.mk

SRC += hydeik_vial.c
# SRC += features/custom_oneshot.c
# SRC += features/swapper.c

COMBO_ENABLE ?= yes
ENCODER_ENABLE ?= no
ENCODER_MAP_ENABLE ?= no
TAP_DANCE_ENABLE ?= yes
