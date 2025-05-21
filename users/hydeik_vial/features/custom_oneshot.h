/*****************************************************************************
 * Exidex's custom oneshot mods & oneshot mod layer
 * (https://github.com/Exidex/qmk_firmware)
 *****************************************************************************/

#pragma once

#include QMK_KEYBOARD_H

/**
 * Represents the six states of the oneshot-mod-layer key.
 */
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

/**
 * Represents the five states of oneshot modifier keys.
 */
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

/**
 * Custom oneshot mod implementation.
 */
void update_oneshot_mod(
    oneshot_layer_state *layer_state,
    oneshot_mod_state *mod_state,
    uint16_t mod,
    uint16_t trigger,
    uint16_t keycode,
    keyrecord_t *record
);

/**
 * Oneshot implementation for the oenshot-mod-layer.
 */
void update_oneshot_layer(
    oneshot_layer_state *layer_state,
    oneshot_mod_state *shift_state,
    oneshot_mod_state *ctrl_state,
    oneshot_mod_state *lalt_state,
    oneshot_mod_state *ralt_state,
    oneshot_mod_state *gui_state,
    uint16_t trigger,
    uint16_t layer,
    uint16_t keycode,
    keyrecord_t *record
);

/**
 * To be implemented by the consumer. Layers one shot implementation needs to
 * know which keys are used as oneshot mods
 */
bool is_oneshot_mod_key(uint16_t keycode);

/**
 * To be implemented by the consumer. Defines keys to cancel oneshot mods.
 */
bool is_oneshot_cancel_key(uint16_t keycode);

/**
 * To be implemented by the consumer. Defines keys to ignore when determining
 * whether a oneshot mod has been used. Setting this to modifiers and layer
 * change keys allows stacking multiple oneshot modifiers, and carrying them
 * between layers.
 */
bool is_oneshot_ignored_key(uint16_t keycode);

