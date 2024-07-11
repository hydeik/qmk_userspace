#include QMK_KEYBOARD_H

#include "hydeik.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

__attribute__ ((weak))
void matrix_scan_keymap(void) {}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_layer_lock(keycode, record, LLOCK)) { return false; }
    if (!process_smtd(keycode, record)) { return false; }

    return process_record_keymap(keycode, record);
}

void matrix_scan_user(void) {
    layer_lock_task();
    matrix_scan_keymap();
}

