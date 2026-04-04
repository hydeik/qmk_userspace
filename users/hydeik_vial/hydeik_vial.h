/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include QMK_KEYBOARD_H

#include "layout.h"

#define SMART_MOD_TARGET_LAYER  _MOD

/****************************************************************************
 * Prototypes for keymap specific customized functions.
 ****************************************************************************/
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
// void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
layer_state_t layer_state_set_keymap(layer_state_t state);

