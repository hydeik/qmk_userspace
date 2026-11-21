# Throwaway probe for issue #37.
VPATH += /
VPATH += $(abspath $(TEST_PATH)/../src)
SRC += split_probe.c probe_glue.c
LAYER_LOCK_ENABLE = yes
INTROSPECTION_KEYMAP_C = probe_keymap_tables.c
