#include QMK_KEYBOARD_H

#ifdef ACHORDION_ENABLE
#include "features/achordion.h"
#endif  /* ACHORDION_ENABLE */
#ifdef LAYER_LOCK_ENABLE
#include "features/layer_lock.h"
#endif  /* LAYER_LOCK_ENABLE */

#include "hydeik.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

/*****************************************************************************
 * Achordion (https://getreuer.info/posts/keyboards/achordion)
 *****************************************************************************/
#ifdef ACHORDION_ENABLE

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
            return QUICK_TAP_TERM;  /* Enable key repeating. */
        default:
            return 0;  /* Otherwise, force hold and disable key repeating. */
    }
}

bool achordion_chord(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record,
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
    /*
     * Also allow same-hand holds when the other key is in the rows below the
     * alphas. I need the `% (MATRIX_ROWS / 2)` because my keyboard is split.
     */
    if (other_record->event.key.row % (MATRIX_ROWS / 2) >= 4) {
        return true;
    }

    /* Otherwise, follow the opposite hands rule. */
    return achordion_opposite_hands(tap_hold_record, other_record);
}

uint16_t achordion_timeout(uint16_t tap_hold_keycode) {
    return 800;  /* Use a timeout of 800 ms. */
}

uint16_t achordion_streak_timeout(uint16_t tap_hold_keycode) {
    if (IS_QK_LAYER_TAP(tap_hold_keycode)) {
        return 0;  /* Disable streak detection on layer-tap keys. */
    }
    /* Otherwise, tap_hold_keycode is a mod-tap key. */
    uint8_t mod = mod_config(QK_MOD_TAP_GET_MODS(tap_hold_keycode));
    if ((mod & MOD_LSFT) != 0) {
        return 0;  /* Disable for Shift mod-tap keys. */
    } else {
        return 100;
    }
}

#endif  /* ACHORDION_ENABLE */

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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef ACHORDION_ENABLE
    if (!process_achordion(keycode, record)) { return false; }
#endif  /* ACHORDION_ENABLE */
#ifdef LAYER_LOCK_ENABLE
    if (!process_layer_lock(keycode, record, LLOCK)) { return false; }
#endif  /* LAYER_LOCK_ENABLE */

    switch(keycode) {
        CASE_MT_NON_BASIC_KEYCODE(HM_UNDS, KC_UNDS);
        CASE_MT_NON_BASIC_KEYCODE(HM_COLN, KC_COLN);
        CASE_MT_NON_BASIC_KEYCODE(HM_ASTR, KC_ASTR);
        CASE_MT_NON_BASIC_KEYCODE(HM_LPRN, KC_LPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_RPRN, KC_RPRN);
        CASE_MT_NON_BASIC_KEYCODE(HM_DQUO, KC_DQUO);
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
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
#ifdef ACHORDION_ENABLE
    achordion_task();
#endif  /* ACHORDION_ENABLE */
#if defined(LAYER_LOCK_ENABLE) && defined(LAYER_LOCK_IDLE_TIMEOUT)
    layer_lock_task();
#endif  /* LAYER_LOCK_ENABLE */
    matrix_scan_keymap();
}

