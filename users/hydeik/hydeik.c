#include QMK_KEYBOARD_H

#include "hydeik.h"

#include "features/sm_td.h"

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
 * sm_td (https://github.com/stasmarkin/sm_td)
 *****************************************************************************/
void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch(keycode) {
        case CKC_SPC: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    break;
                case SMTD_ACTION_TAP:
                    if (is_caps_word_on()) {
                        switch (tap_count) {
                            case 0:
                                tap_code16(KC_UNDS);
                                return;
                            default:
                                caps_word_off();
                                tap_code16(KC_BSPC);
                                tap_code16(KC_SPC);
                                return;
                        }
                    }
                    tap_code16(KC_SPC);
                    break;
                case SMTD_ACTION_HOLD:
                    if (tap_count < 2) {
                        LAYER_PUSH(_NAV);
                    } else {
                        SMTD_REGISTER_16(true, KC_SPC);
                    }
                    break;
                case SMTD_ACTION_RELEASE:
                    if (tap_count < 2) {
                        LAYER_RESTORE();
                    } else {
                        SMTD_REGISTER_16(true, KC_SPC);
                    }
                    break;
            }
            break;
        }

        SMTD_LT(CKC_TAB, KC_TAB, _NAV, 2)
        SMTD_MT(CKC_ENT, KC_ENT, KC_LSFT, 2, false)
        SMTD_LT(CKC_ESC, KC_ESC, _NUM, 2)

        SMTD_MTE(CKC_A, KC_A, KC_LCTL, 2)
        SMTD_MTE(CKC_S, KC_S, KC_LALT, 2)
        SMTD_MTE(CKC_D, KC_D, KC_LGUI, 2)
        SMTD_MTE(CKC_F, KC_F, KC_LSFT, 2)
        SMTD_MTE(CKC_J, KC_J, KC_RSFT, 2)
        SMTD_MTE(CKC_K, KC_K, KC_RGUI, 2)
        SMTD_MTE(CKC_L, KC_L, KC_RALT, 2)
        SMTD_MTE(CKC_SCLN, KC_SCLN, KC_RCTL, 2, false)

        SMTD_MT(CKC_ASTR, KC_ASTR, KC_LCTL, 2, false)
        SMTD_MT(CKC_LPRN, KC_LPRN, KC_LALT, 2, false)
        SMTD_MT(CKC_RPRN, KC_RPRN, KC_LGUI, 2, false)
        SMTD_MT(CKC_COLN, KC_COLN, KC_LSFT, 2, false)
        SMTD_MT(CKC_DQUO, KC_DQUO, KC_RSFT, 2, false)
        SMTD_MT(CKC_LBRC, KC_LBRC, KC_RGUI, 2, false)
        SMTD_MT(CKC_RBRC, KC_RBRC, KC_RALT, 2, false)

        SMTD_MT(CKC_DOT, KC_DOT, KC_LCTL, 2, false)
        SMTD_MT(CKC_1, KC_1, KC_LALT, 2, false)
        SMTD_MT(CKC_2, KC_2, KC_LGUI, 2, false)
        SMTD_MT(CKC_3, KC_3, KC_LSFT, 2, false)
        SMTD_MT(CKC_QUOT, KC_QUOT, KC_RSFT, 2, false)
        SMTD_MT(CKC_UNDS, KC_UNDS, KC_RGUI, 2, false)
        SMTD_MT(CKC_EQL, KC_EQL, KC_RALT, 2, false)
    }
}

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    switch (keycode) {
        /* Index fingers */
        case CKC_F:
        case CKC_J:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_RELEASE) return 30;
            break;

        /* Middle fingers */
        case CKC_D:
        case CKC_K:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_RELEASE) return 20;
            break;

        /* Ring fingers */
        case CKC_S:
        case CKC_L:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_RELEASE) return 20;
            break;

        /* Pinkies */
        case CKC_A:
        case CKC_SCLN:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_SEQUENCE) return 250;
            if (timeout == SMTD_TIMEOUT_RELEASE) return 20;
            break;

        /* Thumbs */
        case CKC_TAB:
        case CKC_SPC:
        case CKC_ENT:
        case CKC_ESC:
            if (timeout == SMTD_TIMEOUT_SEQUENCE) return 200;
            if (timeout == SMTD_TIMEOUT_RELEASE) return 70;
            break;
    }

    return get_smtd_timeout_default(timeout);
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

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    if (mods & MOD_MASK_CTRL) {
        switch (keycode) {
            case CKC_A:                    /* Ctrl+A -> Ctrl+C */
                return C(KC_C);
            case KC_C:                    /* Ctrl+C -> Ctrl+V */
                return C(KC_V);
        }
    } else if (mods & MOD_MASK_GUI) {
        switch (keycode) {
            case CKC_A:                    /* Cmd+A -> Cmd+C */
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

#endif  /* REPEAT_KEY_ENABLE */

/*
 * Custom bahavior of keycodes
 */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_smtd(keycode, record)) { return false; }
    return process_record_keymap(keycode, record);
}

/*
 * Custom bahavior of layers
 */

layer_state_t layer_state_set_user(layer_state_t state) {
    state = layer_state_set_keymap(state);
    return state;
}

/*
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
    matrix_scan_keymap();
}

