/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Common core: layout adapter for the mothwing.
 *
 * Include this file from keymap.c only: it contains the definition of
 * `chordal_hold_layout`, so it must be compiled into exactly one file.
 */

#pragma once

#include QMK_KEYBOARD_H

/* clang-format off */

/*
 * Places the 34 keys of a layer in the layout of the mothwing. The positions
 * the 34 keys do not use are KC_NO.
 *
 * LAYOUT_HYDEIK_34 is variadic so that a layer passed as a single macro
 * (e.g. BASE_LAYER) is expanded into its 34 keys before they are placed.
 */
#define LAYOUT_HYDEIK_34(...) LAYOUT_HYDEIK_34_KEYS(__VA_ARGS__)
#define LAYOUT_HYDEIK_34_KEYS( \
    L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, \
    L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, \
    L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, \
                   L16, L17, R16, R17                 \
    ) \
    LAYOUT( \
    KC_NO, L01, L02, L03, L04, L05, R01, R02, R03, R04, R05, KC_NO, \
    KC_NO, L06, L07, L08, L09, L10, R06, R07, R08, R09, R10, KC_NO, \
    KC_NO, L11, L12, L13, L14, L15, R11, R12, R13, R14, R15, KC_NO, \
            KC_NO, L16, L17, KC_NO, KC_NO, R16, R17, KC_NO          \
    )

/* Hand assignment of every key for Chordal Hold, mirroring the placement above. */
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT(
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',   'R', 'R', 'R', 'R', 'R', 'R',
              '*', '*', '*', '*',   '*', '*', '*', '*'
);

/* clang-format on */
