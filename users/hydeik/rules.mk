# Build settings of the hydeik keymap family, for every keymap of the family,
# with and without Vial.
#
# The rules.mk of a keymap is read before this file, so this file sees what
# the keymap set, VIAL_ENABLE among it, and a `?=` default here gives way to
# the keymap.

SRC += hydeik.c
SRC += typing_rules.c
SRC += features/key_typed.c
SRC += features/smart_mod.c
SRC += features/target_layer.c

EXTRAKEY_ENABLE = yes
LTO_ENABLE = yes
REPEAT_KEY_ENABLE = yes
UNICODE_ENABLE = no
UCIS_ENABLE = no
UNICODEMAP_ENABLE = no
UNICODE_COMMON = no

# Defaults that the rules.mk of a keymap may override.
AUTOCORRECT_ENABLE ?= no
CAPS_WORD_ENABLE ?= yes
CONSOLE_ENABLE ?= no
ENCODER_ENABLE ?= no
ENCODER_MAP_ENABLE ?= no
GRAVE_ESC_ENABLE ?= no
LAYER_LOCK_ENABLE ?= yes
MAGIC_ENABLE ?= no
MUSIC_ENABLE ?= no
NKRO_ENABLE ?= no
SPACE_CADET_ENABLE ?= no

# What exists only with Vial is chosen here, and nowhere else: no C source
# branches on Vial. A keymap with Vial gets the firmware defaults of the Vial
# settings with the QMK hooks that forward to them, and combos and tap dances
# for use from the Vial GUI. A keymap without Vial gets neither.
ifeq ($(strip $(VIAL_ENABLE)), yes)
    SRC += vial_hooks.c
    SRC += features/firmware_default.c
    COMBO_ENABLE = yes
    TAP_DANCE_ENABLE = yes
else
    COMBO_ENABLE = no
    TAP_DANCE_ENABLE = no
endif
