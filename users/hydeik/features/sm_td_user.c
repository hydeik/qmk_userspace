#include QMK_KEYBOARD_H

#include "sm_td.h"
#include "../definitions/layers.h"
#include "../definitions/keycodes.h"

/* utilities for layer manupulation */
#define RETURN_LAYER_NOT_SET 15

static uint8_t return_layer     = RETURN_LAYER_NOT_SET;
static uint8_t return_layer_cnt = 0;
#define LAYER_PUSH(layer)                              \
    return_layer_cnt++;                                \
    if (return_layer == RETURN_LAYER_NOT_SET) {        \
        return_layer = get_highest_layer(layer_state); \
    }                                                  \
    layer_move(layer);

#define LAYER_RESTORE()                          \
    if (return_layer_cnt > 0) {                  \
        return_layer_cnt--;                      \
        if (return_layer_cnt == 0) {             \
            layer_move(return_layer);            \
            return_layer = RETURN_LAYER_NOT_SET; \
        }                                        \
    }

/* utilities for manuplating SMTD */
#define CASE_SMTD_TOM(macro_key, tap_key, mod, threshold)         \
        case macro_key: {                                         \
            switch (action) {                                     \
                case SMTD_ACTION_TOUCH:                           \
                    break;                                        \
                case SMTD_ACTION_TAP:                             \
                    tap_code16(tap_key);                          \
                    break;                                        \
                case SMTD_ACTION_HOLD:                            \
                    if (tap_count < threshold) {                  \
                        register_mods(get_mods() | MOD_BIT(mod)); \
                    } else {                                      \
                        register_code16(tap_key);                 \
                    }                                             \
                    break;                                        \
                case SMTD_ACTION_RELEASE:                         \
                    if (tap_count < threshold) {                  \
                        unregister_mods(MOD_BIT(mod));            \
                    }                                             \
                    unregister_code16(tap_key);                   \
                    break;                                        \
            }                                                     \
            break;                                                \
        }

#define CASE_SMTD_TOM_W_CAPS(macro_key, tap_key, mod, threshold)  \
        case macro_key: {                                         \
            switch (action) {                                     \
                case SMTD_ACTION_TOUCH:                           \
                    break;                                        \
                case SMTD_ACTION_TAP:                             \
                    tap_code16(is_caps_word_on() ? LSFT(tap_key) : tap_key);     \
                    break;                                        \
                case SMTD_ACTION_HOLD:                            \
                    if (tap_count < threshold) {                  \
                        register_mods(get_mods() | MOD_BIT(mod)); \
                    } else {                                      \
                        tap_code16(is_caps_word_on() ? LSFT(tap_key) : tap_key); \
                    }                                             \
                    break;                                        \
                case SMTD_ACTION_RELEASE:                         \
                    if (tap_count < threshold) {                  \
                        unregister_mods(MOD_BIT(mod));            \
                    }                                             \
                    unregister_code16(is_caps_word_on() ? LSFT(tap_key) : tap_key);  \
                    break;                                        \
            }                                                     \
            break;                                                \
        }

#define CASE_SMTD_TOL(macro_key, tap_key, layer, threshold) \
        case macro_key: {                                   \
            switch (action) {                               \
                case SMTD_ACTION_TOUCH:                     \
                    break;                                  \
                case SMTD_ACTION_TAP:                       \
                    tap_code(tap_key);                      \
                    break;                                  \
                case SMTD_ACTION_HOLD:                      \
                    if (tap_count < threshold) {            \
                        LAYER_PUSH(layer);                  \
                    } else {                                \
                        register_code(tap_key);             \
                    }                                       \
                    break;                                  \
                case SMTD_ACTION_RELEASE:                   \
                    if (tap_count < threshold) {            \
                        LAYER_RESTORE();                    \
                    }                                       \
                    unregister_code(tap_key);               \
                    break;                                  \
            }                                               \
            break;                                          \
        }


void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {

     // CKC_SPACE = QK_USER,
     // CKC_ENTER,
     // CKC_ESC,
     // CKC_TAB,
  
     // /* home-row mods */
     // CKC_A,
     // CKC_S,
     // CKC_D,
     // CKC_F,
     // CKC_J,
     // CKC_K,
     // CKC_L,
     // CKC_SCLN,

     // CKC_DOWN,
     // CKC_UP,
     // CKC_RGHT, 
     // CKC_BSPC,

     // CKC_DQT,
     // CKC_LT,
     // CKC_LPRN,
     // CKC_LCBR,
     // CKC_UNDS,
     // CKC_PLUS,
     // CKC_COLN,

    switch (keycode) {
        CASE_SMTD_TOL(CKC_SPACE, KC_SPACE, L_NUM, 2)
        CASE_SMTD_TOL(CKC_ESC, KC_ESC, L_NUM, 2)
        CASE_SMTD_TOL(CKC_ENTER, KC_ENTER, L_FN, 2)
        CASE_SMTD_TOL(CKC_TAB, KC_TAB, L_FN, 2)

        CASE_SMTD_TOM_W_CAPS(CKC_A, KC_A, KC_LCTL, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_S, KC_S, KC_LALT, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_D, KC_D, KC_LGUI, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_F, KC_F, KC_LSFT, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_J, KC_J, KC_RSFT, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_K, KC_K, KC_RGUI, 2)
        CASE_SMTD_TOM_W_CAPS(CKC_L, KC_L, KC_RALT, 2)
        CASE_SMTD_TOM(CKC_SCLN, KC_SCLN, KC_RCTL, 2)

        CASE_SMTD_TOM(CKC_DOWN, KC_DOWN, KC_RIGHT_SHIFT, 2)
        CASE_SMTD_TOM(CKC_UP, KC_UP, KC_RIGHT_CTRL, 2)
        CASE_SMTD_TOM(CKC_RIGHT, KC_RIGHT, KC_RIGHT_ALT, 2)
        CASE_SMTD_TOM(CKC_BSPC, KC_BSPC, KC_RIGHT_ALT, 2)

        CASE_SMTD_TOM(CKC_4, KC_4, KC_LEFT_ALT, 2)
        CASE_SMTD_TOM(CKC_5, KC_5, KC_LEFT_CTRL, 2)
        CASE_SMTD_TOM(CKC_6, KC_6, KC_LEFT_SHIFT, 2)
        CASE_SMTD_TOM(CKC_CIRC, KC_CIRC, KC_LCMD, 2)
        CASE_SMTD_TOM(CKC_AT, KC_AT, KC_RIGHT_SHIFT, 2)
        CASE_SMTD_TOM(CKC_DOLL, KC_DOLLAR, KC_RIGHT_CTRL, 2)

        CASE_SMTD_TOM(CKC_F4, KC_F4, KC_LEFT_ALT, 2)
        CASE_SMTD_TOM(CKC_F5, KC_F5, KC_LEFT_CTRL, 2)
        CASE_SMTD_TOM(CKC_F6, KC_F6, KC_LEFT_SHIFT, 2)
        CASE_SMTD_TOM(CKC_F11, KC_F11, KC_LEFT_GUI, 2)
        CASE_SMTD_TOM(CKC_VOLU, KC_VOLU, KC_RIGHT_GUI, 2)


        case CKC_DF_COMBO: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    break;
                case SMTD_ACTION_TAP:
                    tap_code16(KC_BSPC);
                    break;
                case SMTD_ACTION_HOLD:
                    if (tap_count < 2) {
                        register_mods(get_mods() | MOD_BIT(KC_LSFT) | MOD_BIT(KC_LCTL));
                    } else {
                        register_code16(KC_BSPC);
                    }
                    break;
                case SMTD_ACTION_RELEASE:
                    if (tap_count < 2) {
                        unregister_mods(MOD_BIT(KC_LSFT) | MOD_BIT(KC_LCTL));
                    }
                    unregister_code16(KC_BSPC);
                    break;
            }
            break;
        }


        case CKC_NDOT: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    break;
                case SMTD_ACTION_TAP:
                    switch (tap_count) {
                        case 0:
                            tap_code16(KC_DOT);
                            break;
                        case 1:
                            tap_code16(KC_BSPC);
                            tap_code16(KC_COLN);
                            break;
                        case 2:
                            tap_code16(KC_BSPC);
                            tap_code16(KC_DOT);
                            tap_code16(KC_DOT);
                            tap_code16(KC_DOT);
                            break;
                        default:
                            tap_code16(KC_DOT);
                            break;
                    }
                    break;
                case SMTD_ACTION_HOLD:
                    if (tap_count < 2) {
                        register_mods(get_mods() | MOD_BIT(KC_LCMD));
                    } else {
                        register_code16(KC_DOT);
                    }
                    break;
                case SMTD_ACTION_RELEASE:
                    if (tap_count < 2) {
                        unregister_mods(MOD_BIT(KC_LCMD));
                    }
                    unregister_code16(KC_DOT);
                    break;
            }
            break;
        }

        case CKC_CURR: {
            switch (action) {
                case SMTD_ACTION_TOUCH:
                    switch (tap_count) {
                        case 0:
                            register_unicode(0x20BD);
                            break;
                        default:
                            tap_code16(KC_BSPC);
                            if (tap_count % 2 == 0) {
                                register_unicode(0x20BD);
                            } else {
                                register_unicode(0x20AC);
                            }
                            break;
                    }
                    break;
                case SMTD_ACTION_TAP:
                case SMTD_ACTION_HOLD:
                case SMTD_ACTION_RELEASE:
                    break;
            }
            break;
        }

    }
}
}


smtd_state smtd_states[] = {
    SMTD(CKC_SPACE),
    SMTD(CKC_ESC),
    SMTD(CKC_ENTER),
    SMTD(CKC_TAB),
    SMTD(CKC_DF_COMBO),

    SMTD(CKC_A),
    SMTD(CKC_S),
    SMTD(CKC_D),
    SMTD(CKC_F),
    SMTD(CKC_G),
    SMTD(CKC_H),
    SMTD(CKC_J),
    SMTD(CKC_K),
    SMTD(CKC_L),
    SMTD(CKC_Z),

    SMTD(CYR_F),
    SMTD(CYR_YI),
    SMTD(CYR_V),
    SMTD(CYR_A),
    SMTD(CYR_P),
    SMTD(CYR_R),
    SMTD(CYR_O),
    SMTD(CYR_L),
    SMTD(CYR_D),

    SMTD(CKC_NDOT),
    SMTD(CKC_4),
    SMTD(CKC_5),
    SMTD(CKC_6),
    SMTD(CKC_CIRC),
    SMTD(CKC_AT),
    SMTD(CKC_DOLL),

    SMTD(CKC_F4),
    SMTD(CKC_F5),
    SMTD(CKC_F6),
    SMTD(CKC_F11),
    SMTD(CKC_LEFT),
    SMTD(CKC_DOWN),
    SMTD(CKC_UP),
    SMTD(CKC_RIGHT),
    SMTD(CKC_VOLU),

    SMTD(CKC_CURR),

};
size_t smtd_states_size = sizeof(smtd_states) / sizeof(smtd_states[0]);

uint32_t get_smtd_timeout(uint16_t keycode, smtd_timeout timeout) {
    switch (keycode) {
        case CKC_A:
        case CKC_S:
        case CKC_D:
        case CKC_F:
        case CKC_G:
        case CKC_H:
        case CKC_J:
        case CKC_K:
        case CKC_L:
        case CKC_Z:
        case CYR_F:
        case CYR_YI:
        case CYR_V:
        case CYR_A:
        case CYR_P:
        case CYR_R:
        case CYR_O:
        case CYR_L:
        case CYR_D:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            break;

        case CKC_NDOT:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_SEQUENCE) return 250;
            break;

        case CKC_DF_COMBO:
            if (timeout == SMTD_TIMEOUT_TAP) return 300;
            if (timeout == SMTD_TIMEOUT_SEQUENCE) return 250;
            break;
    }

    return get_smtd_timeout_default(timeout);
}
