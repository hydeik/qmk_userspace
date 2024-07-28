SRC += hydeik.c

LTO_ENABLE = yes
EXTRAKEY_ENABLE = yes  # Audio control and System control
TAP_DANCE_ENABLE = no
COMBO_ENABLE = no
NKRO_ENABLE = yes
OS_DETECTION_ENABLE = yes
DEFERRED_EXEC_ENABLE = yes
MOUSEKEY_ENABLE = no
BOOTMAGIC_ENABLE = no
BACKLIGHT_ENABLE = no
SWAP_HANDS_ENABLE = no
KEY_LOCK_ENABLE = no
CONSOLE_ENABLED = no
COMMAND_ENABLE = no
SPACE_CADET_ENABLE = no
GRAVE_EMC_ENABLE = no
MAGIC_ENABLE = no
MUSIC_ENABLE = no

CAPS_WORD_ENABLE ?= yes
REPEAT_KEY_ENABLE ?= yes

ACHORDION_ENABLE ?= yes
ifeq ($(strip $(ACHORDION_ENABLE)), yes)
	SRC += features/achordion.c
endif

LAYER_LOCK_ENABLE ?= yes
ifeq ($(strip $(LAYER_LOCK_ENABLE)), yes)
	SRC += features/layer_lock.c
endif
