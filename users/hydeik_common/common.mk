# Common core: rules.mk settings that do not depend on the keymap family.
#
# Each keymap family includes this file at the top of its rules.mk, so the
# keymap family can still override a setting afterwards. This file must not
# branch on the keymap family or the firmware source.

# Common-core sources, named by their full path from the userspace root.
SRC += users/hydeik_common/common.c
SRC += users/hydeik_common/features/key_typed.c
SRC += users/hydeik_common/features/target_layer.c

EXTRAKEY_ENABLE = yes
LTO_ENABLE = yes
REPEAT_KEY_ENABLE = yes
UNICODE_ENABLE = no
UCIS_ENABLE = no
UNICODEMAP_ENABLE = no
UNICODE_COMMON = no

# Defaults that a keymap's own rules.mk may override.
AUTOCORRECT_ENABLE ?= no
CAPS_WORD_ENABLE ?= yes
CONSOLE_ENABLE ?= no
GRAVE_ESC_ENABLE ?= no
LAYER_LOCK_ENABLE ?= yes
MAGIC_ENABLE ?= no
MUSIC_ENABLE ?= no
NKRO_ENABLE ?= no
SPACE_CADET_ENABLE ?= no
