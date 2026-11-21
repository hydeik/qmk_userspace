/* Throwaway probe for issue #37: what a keymap family would pass in. */

#pragma once

#include "quantum_keycodes.h"

enum probe_layer_t {
    PROBE_LAYER_BASE,
    PROBE_LAYER_SYM,
    PROBE_LAYER_NUM,
    PROBE_LAYER_NAV,
    PROBE_LAYER_FUN,
    PROBE_LAYER_TRI,
};

/* PROBE_TRIGGER_BASE is QK_KB_0 or QK_USER, set by the config.h of the test. */
enum probe_keycode_t {
    TRG_NAV = PROBE_TRIGGER_BASE,
    TRG_FUN,
};

#define TAB_SYM LT(PROBE_LAYER_SYM, KC_TAB)
#define ENT_NUM LT(PROBE_LAYER_NUM, KC_ENT)
