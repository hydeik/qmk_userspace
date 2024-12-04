#include QMK_KEYBOARD_H

#ifdef LAYER_LOCK_ENABLE
#include "features/layer_lock.h"
#endif  /* LAYER_LOCK_ENABLE */

#include "vial.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}


/*****************************************************************************
 * Exidex's oneshot mods & oneshot mod layer
 * (https://github.com/Exidex/qmk_firmware)
 *****************************************************************************/
typedef enum {
    /* default, waiting for layer to be pressed */
    osl_up_unqueued,
    /* layer pressed and released without pressing mod key, next modifier
       press will have layer enabled, on all mod release layer will be
       disabled */
    osl_up_queued,
    /* layer was pressed but released when some mods were still held,
       on all mod release layer will be disabled */
    osl_up_pending_used,
    /* layer pressed and held, all mod presses will have layer enabled,
       until all mods are released */
    osl_down_unused,
    /* layer pressed and held, some mods are still pressed */
    osl_down_pending_used,
    /* mods were pressed but layer is still held,
       on layer release layer will be disabled */
    osl_down_used,
} oneshot_layer_state;

typedef enum {
    /* default, waiting for mod to be pressed */
    osm_up_unqueued,
    /* mod pressed and held, all other presses will be with this modifier
       enabled, until mod released */
    osm_down_unused,
    /* other key pressed while mod is held, on mod release modifier will
       be disabled */
    osm_down_used,
    /* mod pressed and released without pressing other key, next press will
       have modifier enabled */
    osm_up_queued,
    /* other key pressed and released while layer and mod are active,
       next presses will have modifier enabled until layer is released */
    osm_up_queued_with_layer,
} oneshot_mod_state;

oneshot_mod_state osm_shift_state = osm_up_unqueued;
oneshot_mod_state osm_ctrl_state = osm_up_unqueued;
oneshot_mod_state osm_alt_state = osm_up_unqueued;
oneshot_mod_state osm_gui_state = osm_up_unqueued;

bool is_oneshot_cancel_key(uint16_t keycode) {
    switch (keycode) {
        case MO(_EXT): /* layer 1 */
        case MO(_FUN): /* layer 4 */
            return true;
        default:
            return false;
    }
}

bool is_oneshot_ignored_key(uint16_t keycode) {
    switch (keycode) {
        case OSL_MOD:
        case OSM_SHFT:
        case OSM_CTRL:
        case OSM_ALT:
        case OSM_GUI:
            return true;
        default:
            return false;
    }
}

void update_oneshot_mod(
    oneshot_layer_state *layer_state,
    oneshot_mod_state *mod_state,
    uint16_t mod,
    uint16_t trigger,
    uint16_t keycode,
    keyrecord_t *record
) {
    if (keycode == trigger) {
        if (record->event.pressed) {
            /* Trigger key-down */
            if (*mod_state == osm_up_unqueued) {
                register_code(mod);
            }
            *mod_state = osm_down_unused;
        } else {
            /* Trigger key-up */
            switch (*mod_state) {
                case osm_down_unused:
                    /* If we didn't use the mod while trigger was held, queue it. */
                    *mod_state = osm_up_queued;
                    break;
                case osm_down_used:
                    /* If we did use the mod while trigger was held, unregister it. */
                    *mod_state = osm_up_unqueued;
                    unregister_code(mod);
                    break;
                default:
                    break;
            }
        }
    } else {
        if (record->event.pressed) {
            if (is_oneshot_cancel_key(keycode) && *mod_state != osm_up_unqueued) {
                /* Cancel oneshot on designated cancel keydown. */
                *mod_state = osm_up_unqueued;
                unregister_code(mod);
            }
        } else {
            if (!is_oneshot_ignored_key(keycode)) {
                /* On non-ignored keyup, mark the oneshot as used. */
                switch (*mod_state) {
                    case osm_down_unused:
                        *mod_state = osm_down_used;
                        break;
                    case osm_up_queued:
                        switch (*layer_state) {
                            case osl_up_pending_used: /* because some other mod is still pressed */
                            case osl_down_unused:
                            case osl_down_pending_used:
                            case osl_down_used:
                                *mod_state = osm_up_queued_with_layer;
                                break;
                            default:
                                *mod_state = osm_up_unqueued;
                                unregister_code(mod);
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

oneshot_layer_state osl_mod_state = osl_up_unqueued;

uint16_t pressed_one_shot_mods = 0;

#define CUSTOM_ONE_SHOT_MOD_GET_MODS(kc) ((kc)&0x1F)

bool is_oneshot_mod_key(uint16_t keycode) {
    switch (keycode) {
        case OSM_SHFT:
        case OSM_CTRL:
        case OSM_ALT:
        case OSM_GUI:
            return true;
        default:
            return false;
    }
}

void update_oneshot_layer(
    oneshot_layer_state *layer_state,
    oneshot_mod_state *shift_state,
    oneshot_mod_state *ctrl_state,
    oneshot_mod_state *alt_state,
    oneshot_mod_state *gui_state,
    uint16_t trigger,
    uint16_t layer,
    uint16_t keycode,
    keyrecord_t *record
) {
    if (keycode == trigger) {
        if (record->event.pressed) {
            if (*layer_state == osl_up_unqueued) {
                layer_on(layer);
            }
            *layer_state = osl_down_unused;
        } else {
            switch (*layer_state) {
                case osl_down_unused:
                    *layer_state = osl_up_queued;
                    break;
                case osl_down_used:
                    *layer_state = osl_up_unqueued;
                    layer_off(layer);

                    {
                        if (*shift_state == osm_up_queued_with_layer) {
                            *shift_state = osm_up_unqueued;
                            unregister_code(KC_LSFT);
                        }
                        if (*ctrl_state == osm_up_queued_with_layer) {
                            *ctrl_state = osm_up_unqueued;
                            unregister_code(KC_LCTL);
                        }
                        if (*alt_state == osm_up_queued_with_layer) {
                            *alt_state = osm_up_unqueued;
                            unregister_code(KC_LALT);
                        }
                        if (*gui_state == osm_up_queued_with_layer) {
                            *gui_state = osm_up_unqueued;
                            unregister_code(KC_LGUI);
                        }
                    }

                    break;
                case osl_down_pending_used:
                    *layer_state = osl_up_pending_used;
                    break;
                default:
                    break;
            }
        }
    } else {
        if (record->event.pressed) {
            if (is_oneshot_mod_key(keycode)) {
                pressed_one_shot_mods |= CUSTOM_ONE_SHOT_MOD_GET_MODS(keycode);
            }
        } else {
            if (is_oneshot_mod_key(keycode)) {
                pressed_one_shot_mods &= CUSTOM_ONE_SHOT_MOD_GET_MODS(~(CUSTOM_ONE_SHOT_MOD_GET_MODS(keycode)));
            }

            switch (*layer_state) {
                case osl_down_pending_used:
                case osl_down_unused:
                    if (is_oneshot_mod_key(keycode)) {
                        if (pressed_one_shot_mods) {
                            *layer_state = osl_down_pending_used;
                        } else {
                            *layer_state = osl_down_used;
                            layer_off(layer);
                        }
                    }
                    break;
                case osl_up_queued:
                case osl_up_pending_used:
                    if (is_oneshot_mod_key(keycode)) {
                        if (pressed_one_shot_mods) {
                            *layer_state = osl_up_pending_used;
                        } else {
                            *layer_state = osl_up_unqueued;
                            layer_off(layer);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
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

/*
 * Custom bahavior of keycodes
 */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef LAYER_LOCK_ENABLE
    if (!process_layer_lock(keycode, record, LLOCK)) { return false; }
#endif  /* LAYER_LOCK_ENABLE */

    update_oneshot_mod(
        &osl_mod_state,
        &osm_shift_state,
        KC_LSFT,
        OSM_SHFT,
        keycode,
        record
    );

    update_oneshot_mod(
        &osl_mod_state,
        &osm_ctrl_state,
        KC_LCTL,
        OSM_CTRL,
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
        _MOD,
        keycode,
        record
    );

    return process_record_keymap(keycode, record);
}

/*
 * Custom matrix scanning code
 */
void matrix_scan_user(void) {
#if defined(LAYER_LOCK_ENABLE) && defined(LAYER_LOCK_IDLE_TIMEOUT)
    layer_lock_task();
#endif  /* LAYER_LOCK_ENABLE */
    matrix_scan_keymap();
}

