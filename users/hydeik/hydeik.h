#pragma once

// Prototypes for keymap specific customized functions
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);
void matrix_init_keymap(void);
void matrix_scan_keymap(void);
void keyboard_post_init_keymap(void);
layer_state_t layer_state_set_keymap(layer_state_t state);

#include "definitions/layers.h"
#include "definitions/keycodes.h"
