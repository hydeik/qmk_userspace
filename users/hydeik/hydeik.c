#include QMK_KEYBOARD_H

#include "hydeik.h"

#ifdef SMTD_ENABLE
#include "feature/sm_td.h"
#endif  /* SMTD_ENABLE */

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

/*****************************************************************************
 * Tap-hold configuration (https://docs.qmk.fm/tap_hold)
 *****************************************************************************/

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case HM_S:
        case HM_K:
            return TAPPING_TERM - 40;
        case HM_F:
        case HM_J:
            return TAPPING_TERM - 25;
        default:
            return TAPPING_TERM;
    }
}

#ifndef SMTD_ENABLE
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
        /* Also repeating arrow keys. */
        case HM_DOWN:
        case HM_UP:
        case HM_RGHT:
        case HM_BSPC:
            return QUICK_TAP_TERM;  /* Enable key repeating. */
        default:
            return 0;  /* Otherwise, force hold and disable key repeating. */
    }
}

uint16_t get_flow_tap_term(uint16_t keycode, keyrecord_t* record,
                           uint16_t prev_keycode) {
    if (is_flow_tap_key(keycode) && is_flow_tap_key(prev_keycode)) {
        switch (keycode) {
            case HM_F:
            case HM_J:
              return FLOW_TAP_TERM - 25;

            default:
              return FLOW_TAP_TERM;
        }
    }
    return 0;
}

#ifdef CHORDAL_HOLD
bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record,
                      uint16_t other_keycode, keyrecord_t* other_record) {
    /*
     * Exceptionally consider the following chords as holds, even though they
     * are on the same hand in Magic Sturdy.
     */
    switch (tap_hold_keycode) {
        case HM_A:
            if (other_keycode == HM_F || other_keycode == KC_G ||
                other_keycode == KC_B) {
                return true;
            }
            break;
    }
    return get_chordal_hold_default(tap_hold_record, other_record);
}
#endif  /* CHORDAL_HOLD */

#endif /* SMTD_ENABLE */

/*****************************************************************************
 * sm_td (https://github.com/stasmarkin/sm_td)
 *****************************************************************************/
#ifdef SMTD_ENABLE

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch(keycode) {
        SMTD_LT(EXT_TAB, KC_TAB, _EXT, 2)
        SMTD_LT(SYM_SPC, KC_SPC, _SYM, 2)
        SMTD_LT(NUM_ENT, KC_ENT, _NUM, 2)
        SMTD_MT(SFT_BSPC, KC_BSPC, KC_LSFT, 2, false)

        SMTD_MTE(HM_A, KC_A, KC_LCTL, 2)
        SMTD_MTE(HM_S, KC_S, KC_LALT, 2)
        SMTD_MTE(HM_D, KC_D, KC_LGUI, 2)
        SMTD_MTE(HM_F, KC_F, KC_LSFT, 2)
        SMTD_MTE(HM_J, KC_J, KC_RSFT, 2)
        SMTD_MTE(HM_K, KC_K, KC_RGUI, 2)
        SMTD_MTE(HM_L, KC_L, KC_RALT, 2)
        SMTD_MTE(HM_SCLN, KC_SCLN, KC_RCTL, 2, false)

        SMTD_MT(HM_DOWN, KC_DOWN, KC_RSFT, 2, false)
        SMTD_MT(HM_UP, KC_UP, KC_RGUI, 2, false)
        SMTD_MT(HM_RGHT, KC_RGHT, KC_RALT, 2, false)
        SMTD_MT(HM_BSPC, KC_BSPC, KC_RCTL, 2, false)

        SMTD_MT(HM_ASTR, KC_ASTR, KC_LCTL, 2, false)
        SMTD_MT(HM_LPRN, KC_LPRN, KC_LALT, 2, false)
        SMTD_MT(HM_RPRN, KC_RPRN, KC_LGUI, 2, false)
        SMTD_MT(HM_COLN1,KC_COLN, KC_LSFT, 2, false)
        SMTD_MT(HM_DQUO, KC_DQUO, KC_RSFT, 2, false)
        SMTD_MT(HM_LBRC, KC_LBRC, KC_RGUI, 2, false)
        SMTD_MT(HM_RBRC, KC_RBRC, KC_RALT, 2, false)

        SMTD_MT(HM_DOT, KC_DOT, KC_LCTL, 2, false)
        SMTD_MT(HM_1, KC_1, KC_LALT, 2, false)
        SMTD_MT(HM_2, KC_2, KC_LGUI, 2, false)
        SMTD_MT(HM_3, KC_3, KC_LSFT, 2, false)
        SMTD_MT(HM_QUOT, KC_QUOT, KC_RSFT, 2, false)
        SMTD_MT(HM_UNDS, KC_UNDS, KC_RGUI, 2, false)
        SMTD_MT(HM_EQL, KC_EQL, KC_RALT, 2, false)
        SMTD_MT(HM_COLN2, KC_COLN, KC_RCTL, 2, false)
    }
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    switch (keycode) {
        case HM_A:
        case HM_S:
        case HM_D:
        case HM_F:
        case HM_J:
        case HM_K:
        case HM_L:
        case HM_SCLN:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            break;
    }

    return get_smtd_timeout_default(timeout);
}
#endif /* SMTD_ENABLE */

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

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    if (mods & MOD_MASK_CTRL) {
        switch (keycode) {
            case HM_A:                    /* Ctrl+A -> Ctrl+C */
                return C(KC_C);
            case KC_C:                    /* Ctrl+C -> Ctrl+V */
                return C(KC_V);
        }
    } else if (mods & MOD_MASK_GUI) {
        switch (keycode) {
            case HM_A:                    /* Cmd+A -> Cmd+C */
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

layer_state_t layer_state_set_user(layer_state_t state) {
    state = update_tri_layer_state(state, _SYM, _NUM, _FUN);
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef SMTD_ENABLE
    if (!process_smtd(keycode, record)) { return false; }
#endif  /* SMTD_ENABLE */

#ifndef SMTD_ENABLE
    switch(keycode) {
        CASE_MT_NON_BASIC_KEYCODE(HM_ASTR, KC_ASTR);
        CASE_MT_NON_BASIC_KEYCODE(HM_LPRN, KC_LPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_RPRN, KC_RPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_COLN1, KC_COLN);
        CASE_MT_NON_BASIC_KEYCODE(HM_DQUO, KC_DQUO);
        CASE_MT_NON_BASIC_KEYCODE(HM_UNDS, KC_UNDS);
        CASE_MT_NON_BASIC_KEYCODE(HM_COLN2, KC_COLN);
    }
#endif  /* not def: SMTD_ENABLE */

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
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
    matrix_scan_keymap();
}

