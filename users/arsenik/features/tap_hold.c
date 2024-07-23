#include QMK_KEYBOARD_H
#include "tap_hold.h"
#include "../definitions/keycodes.h"

#ifdef ACHORDION_ENABLE

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t* record) {
    // If you quickly hold a tap-hold key after tapping it, the tap action is
    // repeated. Key repeating is useful e.g. for Vim navigation keys, but can
    // lead to missed triggers in fast typing. Here, returning 0 means we
    // instead want to "force hold" and disable key repeating.
    switch (keycode) {
        // Repeating is useful for Vim navigation keys.
        case HOME_J:
        case HOME_K:
        case HOME_L:
            return QUICK_TAP_TERM;  // Enable key repeating.
        default:
            return 0;  // Otherwise, force hold and disable key repeating.
    }
}

bool achordion_chord(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record,
        uint16_t other_keycode, keyrecord_t* other_record) {
    // Exceptionally consider the following chords as holds, even though they
    // are on the same hand in Magic Sturdy.
    switch (tap_hold_keycode) {
        case HOME_A:  // A + F and A + G.
            if (other_keycode == HOME_F || other_keycode == KC_G || other_keycode == KC_B) {
                return true;
            }
            break;
    }

    // // Also allow same-hand holds when the other key is in the rows below the
    // // alphas. I need the `% (MATRIX_ROWS / 2)` because my keyboard is split.
    // if (other_record->event.key.row % (MATRIX_ROWS / 2) >= 4) {
    //     return true;
    // }

    // Otherwise, follow the opposite hands rule.
    return achordion_opposite_hands(tap_hold_record, other_record);
}

uint16_t achordion_timeout(uint16_t tap_hold_keycode) {
    return 800;  // Use a timeout of 800 ms.
}

uint16_t achordion_streak_timeout(uint16_t tap_hold_keycode) {
    if (IS_QK_LAYER_TAP(tap_hold_keycode)) {
        return 0;  // Disable streak detection on layer-tap keys.
    }

    // Otherwise, tap_hold_keycode is a mod-tap key.
    uint8_t mod = mod_config(QK_MOD_TAP_GET_MODS(tap_hold_keycode));
    if ((mod & MOD_LSFT) != 0) {
        return 0;  // Disable for Shift mod-tap keys.
    } else {
        return 100;
    }
}

#endif /* ACHORDION_ENABLE */


