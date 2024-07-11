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
    switch (keycode) {
        CASE_SMTD_TOL(TAB_EXT, KC_TAB, _EXT, 2)
        CASE_SMTD_TOL(SPC_NUM, KC_SPC, _NUM, 2)
        CASE_SMTD_TOL(ENT_SYM, KC_ENT, _SYM, 2)
        CASE_SMTD_TOL(ESC_FUN, KC_ESC, _FUN, 2)

        CASE_SMTD_TOM_W_CAPS(HM_A, KC_A, KC_LCTL, 2)
        CASE_SMTD_TOM_W_CAPS(HM_S, KC_S, KC_LALT, 2)
        CASE_SMTD_TOM_W_CAPS(HM_D, KC_D, KC_LGUI, 2)
        CASE_SMTD_TOM_W_CAPS(HM_F, KC_F, KC_LSFT, 2)
        CASE_SMTD_TOM_W_CAPS(HM_J, KC_J, KC_RSFT, 2)
        CASE_SMTD_TOM_W_CAPS(HM_K, KC_K, KC_RGUI, 2)
        CASE_SMTD_TOM_W_CAPS(HM_L, KC_L, KC_RALT, 2)
        CASE_SMTD_TOM(HM_SCLN, KC_SCLN, KC_RCTL, 2)

        CASE_SMTD_TOM(HM_DOWN, KC_DOWN, KC_RSFT, 2)
        CASE_SMTD_TOM(HM_UP,   KC_UP,   KC_RGUI, 2)
        CASE_SMTD_TOM(HM_RGHT, KC_RGHT, KC_RALT, 2)
        CASE_SMTD_TOM(HM_BSPC, KC_BSPC, KC_RCTL, 2)

        CASE_SMTD_TOM(HM_DQT,  KC_DQT,  KC_LCTL, 2)
        CASE_SMTD_TOM(HM_LT,   KC_LT,   KC_LALT, 2)
        CASE_SMTD_TOM(HM_LPRN, KC_LPRN, KC_LGUI, 2)
        CASE_SMTD_TOM(HM_LCBR, KC_LCBR, KC_LSFT, 2)
        CASE_SMTD_TOM(HM_UNDS, KC_UNDS, KC_RSFT, 2)
        CASE_SMTD_TOM(HM_EQL,  KC_EQL,  KC_RGUI, 2)
        CASE_SMTD_TOM(HM_PLUS, KC_PLUS, KC_RALT, 2)
        CASE_SMTD_TOM(HM_COLN, KC_COLN, KC_RCTL, 2)

        CASE_SMTD_TOM(HM_1, KC_1, KC_LCTL, 2)
        CASE_SMTD_TOM(HM_2, KC_2, KC_LALT, 2)
        CASE_SMTD_TOM(HM_3, KC_3, KC_LGUI, 2)
        CASE_SMTD_TOM(HM_4, KC_4, KC_LSFT, 2)
        CASE_SMTD_TOM(HM_7, KC_7, KC_RSFT, 2)
        CASE_SMTD_TOM(HM_8, KC_8, KC_RGUI, 2)
        CASE_SMTD_TOM(HM_9, KC_9, KC_RALT, 2)
        CASE_SMTD_TOM(HM_0, KC_0, KC_RCTL, 2)
    }
}


smtd_state smtd_states[] = {
    /* Mod & Tap for layer */
    SMTD(TAB_EXT),
    SMTD(SPC_NUM),
    SMTD(ENT_SYM),
    SMTD(ESC_FUN),

    /* home-row mods */
    SMTD(HM_A),
    SMTD(HM_S),
    SMTD(HM_D),
    SMTD(HM_F),
    SMTD(HM_J),
    SMTD(HM_K),
    SMTD(HM_L),
    SMTD(HM_SCLN),

    SMTD(HM_DOWN),
    SMTD(HM_UP),
    SMTD(HM_RGHT),
    SMTD(HM_BSPC),

    SMTD(HM_DQT),
    SMTD(HM_LT),
    SMTD(HM_LPRN),
    SMTD(HM_LCBR),
    SMTD(HM_UNDS),
    SMTD(HM_EQL),
    SMTD(HM_PLUS),
    SMTD(HM_COLN),

    SMTD(HM_1),
    SMTD(HM_2),
    SMTD(HM_3),
    SMTD(HM_4),
    SMTD(HM_7),
    SMTD(HM_8),
    SMTD(HM_9),
    SMTD(HM_0),
};

size_t smtd_states_size = sizeof(smtd_states) / sizeof(smtd_states[0]);

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

        default:
            break;
    }

    return get_smtd_timeout_default(timeout);
}

