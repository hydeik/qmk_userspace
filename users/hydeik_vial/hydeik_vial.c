#include QMK_KEYBOARD_H

#include "hydeik_vial.h"
#include "features/custom_oneshot.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

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

/*****************************************************************************
 * Exidex's oneshot mods & oneshot mod layer
 * (https://github.com/Exidex/qmk_firmware)
 *****************************************************************************/
bool is_oneshot_mod_key(uint16_t keycode) {
    switch (keycode) {
        case OSM_SFT:
        case OSM_CTL:
        case OSM_ALT:
        case OSM_GUI:
            return true;
        default:
            return false;
    }
}

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
        case MO(EXT_LAYER): /* layer 1 */
        case MO(FN_LAYER): /* layer 4 */
            return true;
        default:
            return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
        case OSL_MOD:
        case OSM_SFT:
        case OSM_CTL:
        case OSM_ALT:
        case OSM_GUI:
            return true;
        default:
            return false;
    }
}

oneshot_mod_state osm_shift_state = osm_up_unqueued;
oneshot_mod_state osm_ctrl_state = osm_up_unqueued;
oneshot_mod_state osm_alt_state = osm_up_unqueued;
oneshot_mod_state osm_gui_state = osm_up_unqueued;
oneshot_layer_state osl_mod_state = osl_up_unqueued;

/*
 * Custom bahavior of keycodes
 */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    update_oneshot_mod(
        &osl_mod_state,
        &osm_shift_state,
        KC_LSFT,
        OSM_SFT,
        keycode,
        record
    );

    update_oneshot_mod(
        &osl_mod_state,
        &osm_ctrl_state,
        KC_LCTL,
        OSM_CTL,
        keycode,
        record
    );

    update_oneshot_mod(
        &osl_mod_state,
        &osm_alt_state,
        KC_LALT,
        OSM_ALT,
        keycode,
        record
    );

    update_oneshot_mod(
        &osl_mod_state,
        &osm_gui_state,
        KC_LGUI,
        OSM_GUI,
        keycode,
        record
    );

    update_oneshot_layer(
        &osl_mod_state,
        &osm_shift_state,
        &osm_ctrl_state,
        &osm_alt_state,
        &osm_gui_state,
        OSL_MOD,
        MOD_LAYER,
        keycode,
        record
    );

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

/*
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
    matrix_scan_keymap();
}

