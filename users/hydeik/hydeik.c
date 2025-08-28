#include QMK_KEYBOARD_H

#include "hydeik.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
layer_state_t layer_state_set_keymap(layer_state_t state) {
    return state;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

/*****************************************************************************
 * Tap-hold configuration (https://docs.qmk.fm/tap_hold)
 *****************************************************************************/

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case HM_D:
        case HM_K:
        case HM_F:
        case HM_J:
            return TAPPING_TERM - 50;
        default:
            return TAPPING_TERM;
    }
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        /* Do not select the hold action when another key is pressed. */
        case SYM_SPC:
            return true;
        case NAV_TAB:
            return true;
        case SFT_ENT:
            return true;
        case NUM_BSPC:
            return true;
        default:
            /* Do not select the hold action when another key is pressed. */
            return false;
    }
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t* record) {
    /*
     * If you quickly hold a tap-hold key after tapping it, the tap action is
     * repeated. Key repeating is useful e.g. for Vim navigation keys, but can
     * lead to missed triggers in fast typing. Here, returning 0 means we
     * instead want to "force hold" and disable key repeating.
     */
    switch (keycode) {
        /* Repeating is useful for Vim/Emacs navigation keys. */
        case HM_D:
        case HM_F:
        case HM_J:
        case HM_K:
        case HM_L:
            return QUICK_TAP_TERM;  /* Enable key repeating. */
        default:
            return 0;  /* Otherwise, force hold and disable key repeating. */
    }
}

uint16_t get_flow_tap_term(uint16_t keycode, keyrecord_t* record,
                           uint16_t prev_keycode) {
    /* Only apply Flow Tap when following a letter key, not a hotkeys. */
    if (get_tap_keycode(prev_keycode) <= KC_Z &&
        (get_mods() & (MOD_MASK_CG | MOD_BIT_LALT)) == 0) {
        switch (keycode) {
            case HM_S:     /* LALT */
            case HM_L:     /* RALT */
                return FLOW_TAP_TERM;
            /* Shorter FLOW_TAP_TERM for strong fingers. */
            case HM_D:     /* LCTL */
            case HM_K:     /* RCTL */
            case HM_V:     /* LGUI */
            case HM_M:     /* RGUI */
                return FLOW_TAP_TERM - 25;
            /* Disable Flow Tap for shift keys. */
        }
    }

    return 0; /* Disable Flow Tap otherwise. */
}

#ifdef CHORDAL_HOLD
bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record,
                      uint16_t other_keycode, keyrecord_t* other_record) {
    /*
     * Exceptionally consider the following chords as holds, even though they
     * are on the same hand in Magic Sturdy.
     */
    switch (tap_hold_keycode) {
        case HM_D:
            if (other_keycode == KC_A ||
                other_keycode == HM_F ||
                other_keycode == HM_V ||
                other_keycode == KC_B) {
                return true;
            }
            break;
        case HM_K:
            if (other_keycode == HM_J ||
                other_keycode == HM_L ||
                other_keycode == KC_Y ||
                other_keycode == KC_U ||
                other_keycode == KC_N ||
                other_keycode == KC_M) {
                return true;
            }
            break;
        case HM_M:
            if (other_keycode == HM_K ||
                other_keycode == HM_L ||
                other_keycode == KC_P) {
                return true;
            }
            break;
        case HM_V:
            if (other_keycode == KC_A ||
                other_keycode == HM_S ||
                other_keycode == KC_Z ||
                other_keycode == KC_X ||
                other_keycode == KC_C) {
                return true;
            }
            break;
    }
    return get_chordal_hold_default(tap_hold_record, other_record);
}
#endif  /* CHORDAL_HOLD */

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
        case LCTL(KC_H):
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

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    if (mods & MOD_MASK_CTRL) {
        switch (keycode) {
            case KC_A:                    /* Ctrl+A -> Ctrl+C */
                return C(KC_C);
            case KC_C:                    /* Ctrl+C -> Ctrl+V */
                return C(KC_V);
        }
    } else if (mods & MOD_MASK_GUI) {
        switch (keycode) {
            case KC_A:                    /* Cmd+A -> Cmd+C */
                return G(KC_C);
            case KC_C:                    /* Cmd+C -> Cmd+V */
                return G(KC_V);
        }
    } else if ((mods & ~MOD_MASK_SHIFT) == 0) {
        switch (keycode) {
            /*
             * For navigating next/previous search results in Vim:
             * N -> Shift + N, Shift + N -> N.
             */
            case KC_N:
                if ((mods & MOD_MASK_SHIFT) == 0) {
                    return S(KC_N);
                }
                return KC_N;
            case KC_DOT:                    /* . -> ./ */
                if ((mods & MOD_MASK_SHIFT) == 0) {
                    return M_UPDIR;
                }
                return M_NOOP;
            case KC_EQL:                    /* = -> == */
                return M_EQEQ;
            case KC_HASH:                   /* # -> include */
                return M_INCLUDE;
            case KC_QUOT:
                if ((mods & MOD_MASK_SHIFT) != 0) {
                    return M_DOCSTR;        /* " -> ""<cursor>""" */
                }
                return M_NOOP;
            case KC_GRV:                    /* ` -> ``<cursor>``` (for Markdown code) */
                return M_MKGRVS;
            case KC_LABK:                   /* < -> - (for Haskell) */
                return KC_MINS;

            /* arithmetic operators, such as, +=, -=, *=, %= ... */
            case KC_PLUS:
            case KC_MINS:
            case KC_ASTR:
            case KC_PERC:
            case KC_PIPE:
            case KC_AMPR:
            case KC_CIRC:
            case KC_TILD:
            case KC_EXLM:
            case KC_RABK:
                return KC_EQL;
        }
    }
    return KC_TRNS;
}

/*
 * An enhanced version of SEND_STRING: if Caps Word is active, the Shift key is
 * held while sending the string. Additionally, the last key is set such that if
 * the Repeat Key is pressed next, it produces `repeat_keycode`. This helper is
 * used for several macros below in my process_record_user() function.
 */
#define MAGIC_STRING(str, repeat_keycode) \
    magic_send_string_P(PSTR(str), (repeat_keycode))

static void magic_send_string_P(const char* str, uint16_t repeat_keycode) {
    uint8_t saved_mods = 0;
    /* If Caps Word is on, save the mods and hold Shift. */
    if (is_caps_word_on()) {
        saved_mods = get_mods();
        register_mods(MOD_BIT(KC_LSFT));
    }

    send_string_P(str);  /* Send the string. */
    set_last_keycode(repeat_keycode);

    /* If Caps Word is on, restore the mods. */
    if (is_caps_word_on()) {
        set_mods(saved_mods);
    }
}

#endif  /* REPEAT_KEY_ENABLE */

/*
 * Custom bahavior of keycodes
 */
#define CASE_MT_NON_BASIC_KEYCODE(macro_key, tap_key) \
    case macro_key:                                   \
        if (record->tap.count) {                      \
            if (record->event.pressed) {              \
                tap_code16(tap_key);                  \
            }                                         \
            return false;                             \
        }                                             \
        break

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch(keycode) {
        CASE_MT_NON_BASIC_KEYCODE(HM_LPRN, KC_LPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_RPRN, KC_RPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_COLN, KC_COLN);
        CASE_MT_NON_BASIC_KEYCODE(HM_AMPR, KC_AMPR);
        CASE_MT_NON_BASIC_KEYCODE(HM_DQUO, KC_DQUO);
        CASE_MT_NON_BASIC_KEYCODE(HM_UNDS, KC_UNDS);
        CASE_MT_NON_BASIC_KEYCODE(HM_HASH, KC_HASH);
        CASE_MT_NON_BASIC_KEYCODE(HM_PLUS, KC_PLUS);
    }

    if (record->event.pressed) {
        switch (keycode) {
            case UPDIR:
                SEND_STRING_DELAY("../", TAP_CODE_DELAY);
                return false;
#ifdef REPEAT_KEY_ENABLE
            // Macros invoked through the MAGIC key.
            case M_UPDIR:
                MAGIC_STRING(/*.*/"./", UPDIR);
                break;
            case M_INCLUDE:
                SEND_STRING_DELAY(/*#*/"include ", TAP_CODE_DELAY);
                break;
            case M_EQEQ:
                SEND_STRING_DELAY(/*=*/"==", TAP_CODE_DELAY);
                break;
            case M_DOCSTR:
                SEND_STRING_DELAY(/*"*/"\"\"\"\"\""
                        SS_TAP(X_LEFT) SS_TAP(X_LEFT) SS_TAP(X_LEFT), TAP_CODE_DELAY);
                break;
            case M_MKGRVS:
                SEND_STRING_DELAY(/*`*/"``\n\n```" SS_TAP(X_UP), TAP_CODE_DELAY);
                break;
#endif  /* REPEAT_KEY_ENABLE */
        }
    }

    return process_record_keymap(keycode, record);
}

#undef CASE_MT_NON_BASIC_KEYCODE

/*
 * Custom behavior for layers
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

