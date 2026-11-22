/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The QMK hooks that draw the indicators with RGB Matrix, on both halves of
 * a split keyboard. They are the adapter of the logic in features/indicator:
 * every decision is taken there (features/indicator.h describes the
 * behaviour, and docs/adr/0005-the-master-half-decides-every-indicator-state.md
 * the split). What this file holds of its own is how an indicator state
 * looks, the blink, and the calls into RGB Matrix and the split transport.
 *
 * rules.mk adds this file, and the logic, to the build of a keymap with
 * RGB Matrix only, so a keymap without it has none of these hooks, and
 * neither have the unit tests. This file is checked on the real keyboard.
 *
 * Both halves draw from one view:
 *
 * - `rgb_matrix_indicators_user` runs once per frame and renews the view.
 *   The master half builds it and sends it to the other half; the other half
 *   takes the view it received, or the empty view once that has expired.
 * - `rgb_matrix_indicators_advanced_user` draws the view, with the same code
 *   on both halves.
 * - `rgb_matrix_hsv_to_rgb` dims the ordinary effect while the view says
 *   that an indicator is shown.
 *
 * QMK calls none of the two hooks while RGB Matrix is switched off or the
 * keyboard is suspended: no indicator is shown then, and nothing is sent.
 */

#include QMK_KEYBOARD_H

#include <string.h>

#include <lib/lib8tion/lib8tion.h>

#include "atomic_util.h"
#include "sync_timer.h"
#include "transactions.h"
#include "transport.h"

#include "features/indicator.h"

/*
 * The brightness of an indicator, as the value of its HSV colour (0 to 255).
 * It is fixed: it does not follow the brightness that is set for the
 * ordinary effect, so the indicators stay visible at brightness 0. A
 * `config.h` can set both.
 */
#ifndef INDICATOR_BRIGHTNESS
#    define INDICATOR_BRIGHTNESS 100
#endif
/* The brightness of the dim look, of a trigger key that works now. */
#ifndef INDICATOR_BRIGHTNESS_DIM
#    define INDICATOR_BRIGHTNESS_DIM 25
#endif

/*
 * The time in milliseconds for which a blinking indicator is lit, and then
 * dark for as long. A `config.h` can set it.
 */
#ifndef INDICATOR_BLINK_INTERVAL
#    define INDICATOR_BLINK_INTERVAL 400
#endif

/*
 * The factor, in 256ths, that the brightness of the ordinary effect is
 * scaled by while an indicator is shown: 64 is a quarter. A `config.h` can
 * set it.
 */
#ifndef INDICATOR_EFFECT_SCALE
#    define INDICATOR_EFFECT_SCALE 64
#endif

_Static_assert(INDICATOR_VIEW_PACKED_SIZE <= RPC_M2S_BUFFER_SIZE, "A packed view does not fit into one split transaction: raise RPC_M2S_BUFFER_SIZE.");

/*****************************************************************************
 * The view
 *****************************************************************************/

/*
 * The view that this half draws from. Only the main loop touches it: the
 * hook that runs once per frame renews it, the other two read it.
 */
static indicator_view_t view;

/*
 * The last packed view that the other half received, and when. The handler
 * of the split transaction writes it in the transport thread and the main
 * loop reads it, so both touch it only inside an atomic block.
 */
static struct {
    uint8_t  packed[INDICATOR_VIEW_PACKED_SIZE];
    uint32_t arrival_time;
    bool     has_arrived;
} received;

/* The handler of the split transaction. It runs in the transport thread. */
static void receive_view(uint8_t in_size, const void *in_data, uint8_t out_size, void *out_data) {
    if (in_size != INDICATOR_VIEW_PACKED_SIZE) {
        return;
    }

    const uint32_t now = timer_read32();

    ATOMIC_BLOCK_FORCEON {
        memcpy(received.packed, in_data, INDICATOR_VIEW_PACKED_SIZE);
        received.arrival_time = now;
        received.has_arrived  = true;
    }
}

/* The master half: build the view, and send it when the logic says so. */
static void renew_view_on_master_half(void) {
    /* The packed view of the last send that succeeded. */
    static uint8_t  sent[INDICATOR_VIEW_PACKED_SIZE];
    static bool     last_send_failed;
    static uint32_t last_send_time;

    uint8_t packed[INDICATOR_VIEW_PACKED_SIZE];

    indicator_view_build(&view);
    indicator_view_pack(&view, packed);

    const bool changed = memcmp(packed, sent, sizeof(packed)) != 0;

    if (!indicator_view_should_send(changed, last_send_failed, timer_elapsed32(last_send_time))) {
        return;
    }

    last_send_failed = !transaction_rpc_send(INDICATOR_VIEW_TRANSACTION, sizeof(packed), packed);
    last_send_time   = timer_read32();
    if (!last_send_failed) {
        memcpy(sent, packed, sizeof(sent));
    }
}

/*
 * The other half: take the view that was received, or the empty view when
 * the logic says that it has expired.
 */
static void renew_view_on_other_half(void) {
    uint8_t  packed[INDICATOR_VIEW_PACKED_SIZE];
    uint32_t arrival_time;
    bool     has_arrived;

    ATOMIC_BLOCK_FORCEON {
        memcpy(packed, received.packed, sizeof(packed));
        arrival_time = received.arrival_time;
        has_arrived  = received.has_arrived;
    }

    if (indicator_view_has_expired(has_arrived, timer_elapsed32(arrival_time))) {
        memset(&view, 0, sizeof(view));
    } else {
        indicator_view_unpack(packed, &view);
    }
}

/*
 * Once per frame, on both halves. It sets no colour.
 *
 * The transaction is registered here, on the first frame and on both halves,
 * because either half can be the master. A half drops a transaction that
 * arrives before it has registered; the master half sends again every
 * `INDICATOR_VIEW_SEND_INTERVAL`, which makes up for it.
 */
bool rgb_matrix_indicators_user(void) {
    static bool is_registered;

    if (!is_registered) {
        transaction_register_rpc(INDICATOR_VIEW_TRANSACTION, receive_view);
        is_registered = true;
    }

    if (is_keyboard_master()) {
        renew_view_on_master_half();
    } else {
        renew_view_on_other_half();
    }
    return true;
}

/*****************************************************************************
 * The look
 *****************************************************************************/

typedef struct {
    hsv_t hsv;
    bool  blinks;
} indicator_look_t;

/*
 * How each indicator state looks. `INDICATOR_STATE_NONE` has no look, and
 * neither has a number beyond this table, which a view that did not come
 * from `indicator_view_pack()` can hold: both show nothing.
 */
static const indicator_look_t looks[] = {
    /* Orange. */
    [INDICATOR_STATE_PENDING_ONE_SHOT_MOD] = {.hsv = {21, 255, INDICATOR_BRIGHTNESS}, .blinks = true},
    /* Magenta. */
    [INDICATOR_STATE_LATCHED_ONE_SHOT_MOD] = {.hsv = {213, 255, INDICATOR_BRIGHTNESS}},
    /* Blue. */
    [INDICATOR_STATE_TRIGGER_KEY_WORKING] = {.hsv = {170, 255, INDICATOR_BRIGHTNESS_DIM}},
    /* Green. */
    [INDICATOR_STATE_TARGET_LAYER_ON] = {.hsv = {85, 255, INDICATOR_BRIGHTNESS}},
    /* Cyan. */
    [INDICATOR_STATE_TARGET_LAYER_PENDING] = {.hsv = {128, 255, INDICATOR_BRIGHTNESS}, .blinks = true},
    /* White. */
    [INDICATOR_STATE_TARGET_LAYER_LOCKED] = {.hsv = {0, 0, INDICATOR_BRIGHTNESS}},
};

/*
 * Draw the indicators of the view on the LEDs `led_min` to `led_max - 1`,
 * which are LEDs of this half. QMK calls it after the ordinary effect has
 * drawn them, so an indicator replaces the colour of the effect.
 *
 * A colour is set with `RGB_MATRIX_INDICATOR_SET_COLOR` alone, which leaves
 * an LED outside that window untouched: in vial-qmk a bare
 * `rgb_matrix_set_color()` with an LED of the other half lights a wrong LED
 * of this half. A key position without an LED shows nothing.
 *
 * A blinking indicator is black while it is dark, not the colour of the
 * effect. The phase comes from the synchronised timer, so both halves blink
 * in step. The colours are converted with `hsv_to_rgb()`, not with
 * `rgb_matrix_hsv_to_rgb()`, so the dimming below leaves them alone.
 */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    const bool is_blink_lit = (sync_timer_read32() / INDICATOR_BLINK_INTERVAL) % 2 == 0;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            const uint8_t state = indicator_view_state_at(&view, (keypos_t){.row = row, .col = col});

            if (state == INDICATOR_STATE_NONE || state >= ARRAY_SIZE(looks)) {
                continue;
            }

            const indicator_look_t *look = &looks[state];
            const rgb_t             rgb  = (look->blinks && !is_blink_lit) ? (rgb_t){0} : hsv_to_rgb(look->hsv);

            uint8_t       leds[LED_HITS_TO_REMEMBER];
            const uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, leds);

            for (uint8_t i = 0; i < led_count; i++) {
                RGB_MATRIX_INDICATOR_SET_COLOR(leds[i], rgb.r, rgb.g, rgb.b);
            }
        }
    }
    return true;
}

/*****************************************************************************
 * Dimming the ordinary effect
 *****************************************************************************/

/*
 * QMK converts the colours of the ordinary effect with this function, and
 * defines it weak. While an indicator is shown, on either half, the effect
 * is dimmed on the whole keyboard so that the indicators stand out. Nothing
 * of it reaches the stored settings.
 */
rgb_t rgb_matrix_hsv_to_rgb(hsv_t hsv) {
    if (view.any_shown) {
        hsv.v = scale8(hsv.v, INDICATOR_EFFECT_SCALE);
    }
    return hsv_to_rgb(hsv);
}
