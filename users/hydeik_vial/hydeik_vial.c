#include QMK_KEYBOARD_H

#include "hydeik_vial.h"
// #include "features/custom_oneshot.h"
// #include "features/swapper.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

__attribute__ ((weak))
layer_state_t layer_state_set_keymap(layer_state_t state) {
    return state;
}

/*****************************************************************************
 * Caps word (https://docs.qmk.fm/features/caps_word)
 *****************************************************************************/
#ifdef CAPS_WORD_ENABLE
/* Callback for the Caps Word features */
bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        /* Keycodes that continue Caps Word, with shift applied. */
        case KC_A ... KC_Z:
            /* Apply shift to the next key. */
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;

        /* Keycodes that continue Caps Word, without shifting. */
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_MINS:
        case KC_UNDS:
            return true;

        default:
            /* Deactivate Caps Word. */
            return false;
    }
}

#endif  /* CAPS_WORD_ENABLE */

/****************************************************************************
 * Repeat key (https://docs.qmk.fm/features/repeat_key)
 ****************************************************************************/
#ifdef REPEAT_KEY_ENABLE

bool remember_last_key_user(uint16_t keycode, keyrecord_t* record,
                            uint8_t* remembered_mods) {
    /* Unpack tapping keycode for tap-hold keys. */
    switch (keycode) {
#ifndef NO_ACTION_TAPPING
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
            break;
#ifndef NO_ACTION_LAYER
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
            keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
            break;
#endif  /* NO_ACTION_LAYER */
#endif  /* NO_ACTION_TAPPING */
    }
    /*
     * Forget Shift on letters when Shift or AltGr are the only mods.
     * Exceptionally, I want Shift remembered on N and Z for "NN" and "ZZ" in Vim.
     */
    switch (keycode) {
        case KC_A ... KC_M:
        case KC_O ... KC_Y:
            if ((*remembered_mods & ~(MOD_MASK_SHIFT | MOD_BIT(KC_RALT))) == 0) {
                *remembered_mods &= ~MOD_MASK_SHIFT;
            }
            break;
    }

    return true;
}

#endif  /* REPEAT_KEY_ENABLE */

/*****************************************************************************
 * Exidex's oneshot mods & oneshot mod layer
 * (https://github.com/Exidex/qmk_firmware)
 *****************************************************************************/
bool is_custom_oneshot_mod_key(uint16_t keycode) {
    switch (keycode) {
        case COS_LCTL:
        case COS_LSFT:
        case COS_LALT:
        case COS_LGUI:
        case COS_RCTL:
        case COS_RSFT:
        case COS_RALT:
        case COS_RGUI:
            return true;
        default:
            return false;
    }
}

bool is_custom_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
        case MO(_NAV):
        case MO(_FUN):
        case TO(_NAV):
        case TO(_FUN):
            return true;
        default:
            return false;
    }
}

bool is_custom_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
        case COS_MOD:
        case COS_LCTL:
        case COS_LSFT:
        case COS_LALT:
        case COS_LGUI:
        case COS_RCTL:
        case COS_RSFT:
        case COS_RALT:
        case COS_RGUI:
            return true;
        default:
            return false;
    }
}

uint8_t custom_oneshot_mod_get_mods(uint16_t keycode) {
    switch (keycode) {
        case COS_LCTL:
            return MOD_BIT_LCTRL;
            break;
        case COS_LSFT:
            return MOD_BIT_LSHIFT;
            break;
        case COS_LALT:
            return MOD_BIT_LALT;
            break;
        case COS_LGUI:
            return MOD_BIT_LGUI;
            break;
        case COS_RCTL:
            return MOD_BIT_RCTRL;
            break;
        case COS_RSFT:
            return MOD_BIT_RSHIFT;
            break;
        case COS_RALT:
            return MOD_BIT_RALT;
            break;
        case COS_RGUI:
            return MOD_BIT_RGUI;
            break;
        default:
            return 0;
            break;
    }
}

static uint8_t pressed_oneshot_mods = 0;
static uint8_t queued_oneshot_mods = 0;
static uint8_t used_oneshot_mods = 0;
static uint8_t mod_layer_hold_count = 0;

static void custom_oneshot_clear_mods(void) {
    pressed_oneshot_mods = 0;
    queued_oneshot_mods = 0;
    used_oneshot_mods = 0;
    clear_mods();
}

bool process_custom_oneshot_mods(uint16_t keycode, keyrecord_t *record) {
    if (is_custom_oneshot_mod_key(keycode)) {
        /* Handle custom oneshot mod key */
        uint8_t mod = custom_oneshot_mod_get_mods(keycode);
        if (record->event.pressed) {
            pressed_oneshot_mods |= mod;
            register_mods(mod);
        } else {
            pressed_oneshot_mods &= ~mod;

            if (used_oneshot_mods & mod) {
                used_oneshot_mods &= ~mod;
                unregister_mods(mod);
            } else {
                queued_oneshot_mods |= mod;
            }

            if (!pressed_oneshot_mods) {
                if (IS_LAYER_ON(SMT_MOD_LAYER)) {
                    layer_off(SMT_MOD_LAYER);
                }
            }
        }
    } else {
        /* Handle other keys */
        if (record->event.pressed) {
            if (is_custom_oneshot_cancel_key(keycode)) {
                /* Cancel all oneshot mods on designated cancel keydown. */
                custom_oneshot_clear_mods();
                mod_layer_hold_count = 0;
            }
        } else {
            if (!is_custom_oneshot_ignored_key(keycode)) {
                /* On non-ignored keydown, mark the oneshot mods as used. */
                if (pressed_oneshot_mods) {
                    used_oneshot_mods |= pressed_oneshot_mods;
                }
                if (queued_oneshot_mods) {
                    if (mod_layer_hold_count > 0) {
                        used_oneshot_mods |= queued_oneshot_mods;
                    } else {
                        unregister_mods(queued_oneshot_mods);
                        used_oneshot_mods &= ~queued_oneshot_mods;
                        queued_oneshot_mods = 0;
                    }
                }
            }
        }
    }

    return true;
}

void handel_mod_layer_hold_event(uint8_t layer, keyrecord_t *record) {
    if (record->event.pressed) {
        if (mod_layer_hold_count == 0) {
            custom_oneshot_clear_mods();
            layer_on(layer);
        }
        ++mod_layer_hold_count;
    } else {
        --mod_layer_hold_count;
        if (mod_layer_hold_count == 0) {
            if (!pressed_oneshot_mods) {
                layer_off(layer);
                if (queued_oneshot_mods & used_oneshot_mods) {
                    used_oneshot_mods &= ~queued_oneshot_mods;
                    unregister_mods(queued_oneshot_mods);
                    queued_oneshot_mods = 0;
                }
            }
        }
    }
}

/*
 * Custom bahavior of keycodes
 */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_custom_oneshot_mods(keycode, record);

    switch (keycode) {
        case SMT_MOD:
            if (record->tap.count == 0) {
                handel_mod_layer_hold_event(SMT_MOD_LAYER, record);
            } else {
                if (record->event.pressed) {
                    register_mods(MOD_BIT_LSHIFT);
                } else {
                    add_oneshot_mods(MOD_BIT_LSHIFT);
                    unregister_mods(MOD_BIT_LSHIFT);
                }
            }
            return false;
            break;
        case SPC_MOD:
        case ENT_MOD:
            if (record->tap.count == 0) {
                handel_mod_layer_hold_event(SMT_MOD_LAYER, record);
                return false;
            }
            break;
    }

    return process_record_keymap(keycode, record);
}

/*
 * Layers manipulation
 */
layer_state_t layer_state_set_user(layer_state_t state) {
    // state = update_tri_layer_state(state, _SYM, _NUM, _FUN);
    return layer_state_set_keymap(state);
}

/*
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
    matrix_scan_keymap();
}

