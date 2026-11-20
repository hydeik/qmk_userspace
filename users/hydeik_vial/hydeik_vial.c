/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Glue between QMK and smart_mod, the state machine of the one-shot mods and
 * the smart mod layer (features/smart_mod.h describes the behaviour). This
 * file forwards the three QMK hooks to smart_mod and tells it which keycodes
 * are one-shot mod keys, which keys are smart mod layer keys, which of them
 * has the Shift tap rule, and which layer is the smart mod layer. It holds no
 * behaviour of its own.
 *
 * tests/smart_mod/test_glue.c is the copy of this file that the unit tests
 * build. The two must configure smart_mod the same way. They differ only in
 * this:
 *
 * - The unit tests are built without layout.h, so test_glue.h has its own
 *   copy of the keycodes and layer numbers used here, under `TEST_LAYER_*`
 *   names for the layers. The assertions below pin the numbers that the copy
 *   repeats.
 * - The unit tests are built without Vial. Here, Vial's tap-hold settings
 *   decide whether a smart mod layer key is tapped or held, and the tapping
 *   term of the double tap of `SMT_MOD` is the one set in Vial, not the
 *   `TAPPING_TERM` of config.h.
 */

#include "hydeik_vial.h"

#include "features/smart_mod.h"

/*
 * Keymaps stored by Vial hold these numbers, and vial.json names the custom
 * keycodes by their position. tests/smart_mod/test_glue.c pins them too.
 */
_Static_assert((int)COS_MOD == (int)QK_KB_0, "COS_MOD must stay the reserved slot at QK_KB_0");
_Static_assert((int)COS_LCTL == (int)QK_KB_0 + 1, "the one-shot mod keys must start at QK_KB_0 + 1");
_Static_assert((int)COS_RGUI == (int)QK_KB_0 + 8, "the one-shot mod keys must end at QK_KB_0 + 8");
_Static_assert(_MOD == 5, "the smart mod layer must stay layer 5");

/*****************************************************************************
 * Told to smart_mod
 *****************************************************************************/

/* The one-shot mod keys and their modifiers. */
uint8_t smart_mod_one_shot_mod_bits(uint16_t keycode) {
    switch (keycode) {
        case COS_LCTL:
            return MOD_BIT_LCTRL;
        case COS_LSFT:
            return MOD_BIT_LSHIFT;
        case COS_LALT:
            return MOD_BIT_LALT;
        case COS_LGUI:
            return MOD_BIT_LGUI;
        case COS_RCTL:
            return MOD_BIT_RCTRL;
        case COS_RSFT:
            return MOD_BIT_RSHIFT;
        case COS_RALT:
            return MOD_BIT_RALT;
        case COS_RGUI:
            return MOD_BIT_RGUI;
        default:
            return 0;
    }
}

/* The smart mod layer. Nothing but smart_mod switches it on or off. */
uint8_t smart_mod_layer(void) {
    return _MOD;
}

/* The smart mod layer keys. */
bool smart_mod_is_layer_key(uint16_t keycode) {
    switch (keycode) {
        case SMT_MOD:
        case SPC_MOD:
        case ENT_MOD:
            return true;
        default:
            return false;
    }
}

/*
 * A tap of `SMT_MOD` makes Shift a pending one-shot mod, or cancels all
 * pending one-shot mods, and a double tap turns Caps Word on. A tap of the
 * other smart mod layer keys is their ordinary key.
 */
bool smart_mod_layer_key_taps_shift(uint16_t keycode) {
    return keycode == SMT_MOD;
}

/*****************************************************************************
 * QMK hooks
 *****************************************************************************/

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return smart_mod_process_record(keycode, record);
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    smart_mod_post_process_record(keycode, record);
}

void housekeeping_task_user(void) {
    smart_mod_housekeeping_task();
}
