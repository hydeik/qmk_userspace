/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "hydeik_vial.h"

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
        case COS_CLR:
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
        case COS_CLR:
        case SMT_MOD:
        case SPC_MOD:
        case ENT_MOD:
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

/* Stores the state of whether the oneshot-mod keys are pressed (keys down). */
static uint8_t pressed_mods = 0;
/* A variable used to keep track of when a key other than a modifier key is pressed. */
static uint8_t used_mods = 0;
/* Count the number of smart mod layer is held */
static uint8_t smart_mod_layer_hold_count = 0;

static void clear_custom_oneshot_mods(void) {
    clear_mods();
    pressed_mods = 0;
    used_mods = 0;
}

static void custom_oneshot_layer_hold_event(keyrecord_t *record) {
    if (record->event.pressed) { /* key down */
        used_mods = get_mods();
    } else { /* key up */
        if (used_mods) {
            unregister_mods(used_mods);
            used_mods = 0;
        }
    }
}

static void custom_oneshot_on_other_key_release_event(void) {
    used_mods |= pressed_mods;
    /* Oneshot mods are the mods other than used_mods. */
    const uint8_t queued_mods = get_mods() & ~used_mods;
    if (queued_mods) {
        if (smart_mod_layer_hold_count) {
            used_mods |= queued_mods;
        } else {
            unregister_mods(queued_mods);
        }
    }
}

static void smart_mod_layer_hold_event_handler(
    keyrecord_t *record
) {
    if (record->event.pressed) { /* key down */
        if (smart_mod_layer_hold_count == 0) {
            layer_on(SMART_MOD_TARGET_LAYER);
            used_mods = get_mods(); /* mark oneshot mods as used */
        }
        ++smart_mod_layer_hold_count;
    } else {
        --smart_mod_layer_hold_count;
        if (smart_mod_layer_hold_count == 0) {
            if (!pressed_mods) {
                if (IS_LAYER_ON(SMART_MOD_TARGET_LAYER)) {
                    layer_off(SMART_MOD_TARGET_LAYER);
                }
                unregister_mods(used_mods);
                used_mods = 0;
            }
        }
    }
}

static bool process_custom_oneshot_mods(uint16_t keycode, keyrecord_t *record) {
    if (is_custom_oneshot_mod_key(keycode)) {
        /* Handle custom oneshot mod key */
        const uint8_t mod = custom_oneshot_mod_get_mods(keycode);
        if (record->event.pressed) {
            pressed_mods |= mod;
            register_mods(mod);
        } else {
            pressed_mods &= ~mod;

            if (used_mods & mod) {
                used_mods &= ~mod;
                unregister_mods(mod);
            }

            if (!pressed_mods) {
                if (IS_LAYER_ON(SMART_MOD_TARGET_LAYER)) {
                    layer_off(SMART_MOD_TARGET_LAYER);
                }
            }
        }
    } else if (IS_QK_MOMENTARY(keycode)) {
        const uint8_t layer = QK_MOMENTARY_GET_LAYER(keycode);
        if (layer == SMART_MOD_TARGET_LAYER) {
            smart_mod_layer_hold_event_handler(record);
            return false;
        } else {
            custom_oneshot_layer_hold_event(record);
        }
    } else if (IS_QK_LAYER_MOD(keycode)) {
        const uint8_t layer = QK_LAYER_MOD_GET_LAYER(keycode);
        if (layer == SMART_MOD_TARGET_LAYER) {
            smart_mod_layer_hold_event_handler(record);
            return false;
        } else {
            custom_oneshot_layer_hold_event(record);
        }
    } else if (IS_QK_LAYER_TAP(keycode)) {
        const uint8_t layer = QK_LAYER_TAP_GET_LAYER(keycode);
        if (record->tap.count == 0) { /* key is held: */
            if (layer == SMART_MOD_TARGET_LAYER) {
                smart_mod_layer_hold_event_handler(record);
                return false;
            } else {
                custom_oneshot_layer_hold_event(record);
            }
        } else { /* key is tapped */
            if (!record->event.pressed) {
                custom_oneshot_on_other_key_release_event();
            }
        }
    } else {
        /* Handle other keys */
        if (record->event.pressed) {
            if (is_custom_oneshot_cancel_key(keycode)) {
                /* Cancel all oneshot mods on designated cancel keydown. */
                clear_custom_oneshot_mods();
            }
        } else {
            if (!is_custom_oneshot_ignored_key(keycode)) {
                custom_oneshot_on_other_key_release_event();
            }
        }

    }
    return true;
}

static bool process_smart_mod_layer_key(
    uint16_t target_keycode,
    uint16_t keycode,
    keyrecord_t *record
) {
#ifdef CAPS_WORD_ENABLE
    static bool tapped = false;
    static uint16_t tap_timer = 0;
#endif /* CAPS_WORD_ENABLE */
    if (keycode == target_keycode) {
        if (record->tap.count > 0) {
            if (record->event.pressed) {
                clear_custom_oneshot_mods();
                register_mods(MOD_BIT_LSHIFT);
#ifdef CAPS_WORD_ENABLE
                if (tapped && !timer_expired(record->event.time, tap_timer)) {
                    clear_custom_oneshot_mods();
                    caps_word_on();
                }
                tapped = true;
                tap_timer = record->event.time + GET_TAPPING_TERM(keycode, record);
#endif /* CAPS_WORD_ENABLE */
            } else {
                if (used_mods & MOD_BIT_LSHIFT) {
                    used_mods &= ~MOD_BIT_LSHIFT;
                    unregister_mods(MOD_BIT_LSHIFT);
                }
            }
        }
        return false;
#ifdef CAPS_WORD_ENABLE
    } else {
        if (record->event.pressed) {
            tapped = false;
        }
#endif /* CAPS_WORD_ENABLE */
    }

    return true;
}

/*
 * Custom bahavior of keycodes
 */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!(
        process_custom_oneshot_mods(keycode, record) &&
        process_smart_mod_layer_key(SMT_MOD, keycode, record)
    )) {
        return false;
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

