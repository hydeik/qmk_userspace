/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Glue between QMK and the two state machines of the hydeik_vial keymap
 * family:
 *
 * - smart_mod, the one-shot mods and the smart mod layer
 *   (features/smart_mod.h describes the behaviour). This file tells it which
 *   keycodes are one-shot mod keys, which keys are smart mod layer keys,
 *   which of them has the Shift tap rule, and which layer is the smart mod
 *   layer.
 * - target_layer of the common core, the layer switching
 *   (users/hydeik_common/features/target_layer.h describes the behaviour).
 *   This file tells it the table of entries, which key presses are those of
 *   one-shot mod keys, and when the timeout of a pending target layer is
 *   suspended.
 *
 * This file forwards the three QMK hooks to both, in the order that their
 * interaction needs, and holds no behaviour of its own.
 *
 * The unit tests build this file in two ways:
 *
 * - tests/target_layer_smart_mod/ builds this very file, with layout.h, and
 *   so tests both state machines as they are wired here.
 * - tests/smart_mod/test_glue.c is a copy of the smart_mod part of this file,
 *   without target_layer and without layout.h. The two must configure
 *   smart_mod the same way. test_glue.h has its own copy of the keycodes and
 *   layer numbers used here, under `TEST_LAYER_*` names for the layers. The
 *   assertions below pin the numbers that the copy repeats.
 *
 * Both are built without Vial. Here, Vial's tap-hold settings decide whether
 * a smart mod layer key or a holding key is tapped or held, and the tapping
 * term of the double tap of `SMT_MOD` is the one set in Vial, not the
 * `TAPPING_TERM` of config.h.
 */

#include "hydeik_vial.h"

#include "features/firmware_default.h"
#include "features/smart_mod.h"
#include "users/hydeik_common/features/target_layer.h"

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
 * Told to target_layer
 *****************************************************************************/

/*
 * The table of entries: `TAB_SYM` and the trigger key `MO(_NAV)` switch to
 * NAV, and a quick release of `TAB_SYM` locks it. `ENT_NUM` and the trigger
 * key `MO(_FUN)` switch to FUN, and a quick release of `ENT_NUM` leaves it
 * pending.
 */
const target_layer_entry_t target_layer_entries[] = {
    {TAB_SYM, _NAV, TARGET_LAYER_QUICK_RELEASE_LOCKED},
    {ENT_NUM, _FUN, TARGET_LAYER_QUICK_RELEASE_PENDING},
};

const uint8_t target_layer_entry_count = sizeof(target_layer_entries) / sizeof(target_layer_entries[0]);

/* The Layer Lock key locks the highest layer that is on. */
_Static_assert(_NAV > _SYM && _FUN > _NUM, "a target layer must lie above its holding layer");

/*
 * A one-shot mod key press is that of a one-shot mod key, or a tap of
 * `SMT_MOD`, which makes Shift pending, cancels the pending one-shot mods or
 * turns Caps Word on, and sends nothing.
 */
bool target_layer_is_one_shot_mod_key(uint16_t keycode, const keyrecord_t *record) {
    if (smart_mod_one_shot_mod_bits(keycode) != 0) {
        return true;
    }
    return smart_mod_is_layer_key(keycode) && smart_mod_layer_key_taps_shift(keycode) && record->tap.count != 0;
}

/*
 * Latched one-shot mods have no timeout, so a pending target layer has none
 * either while a smart mod layer key is held.
 */
bool target_layer_is_pending_timeout_suspended(void) {
    return smart_mod_is_layer_key_held();
}

/*****************************************************************************
 * QMK hooks
 *****************************************************************************/

/*
 * Both state machines see every key event, also those that the other one
 * handles: target_layer needs the one-shot mod keys and the smart mod layer
 * keys, which smart_mod keeps from QMK, and smart_mod needs the press of a
 * trigger key, which target_layer keeps from QMK. target_layer comes first.
 */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const bool target_layer_passes = target_layer_process_record(keycode, record);
    const bool smart_mod_passes    = smart_mod_process_record(keycode, record);
    return target_layer_passes && smart_mod_passes;
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    smart_mod_post_process_record(keycode, record);
    target_layer_post_process_record(keycode, record);
}

/*
 * smart_mod comes first, so that the pending one-shot mods end before a
 * pending target layer when both timeouts end in the same scan loop.
 */
void housekeeping_task_user(void) {
    smart_mod_housekeeping_task();
    target_layer_housekeeping_task();
}

/*
 * The hooks of firmware_default, the firmware defaults of the Vial settings
 * (features/firmware_default.h). They are not part of test_glue.c: the unit
 * tests of firmware_default call it directly.
 *
 * `via_init_kb()` is a keyboard-level hook that no keyboard of this keymap
 * family defines, and vial-qmk has no user-level hook at that point.
 */

void via_init_kb(void) {
    firmware_default_via_init();
}

void eeconfig_init_user(void) {
    /* What the default `eeconfig_init_user()` of QMK does. */
    eeconfig_update_user(0);
    firmware_default_eeconfig_init();
}

void keyboard_post_init_user(void) {
    firmware_default_keyboard_post_init();
}
