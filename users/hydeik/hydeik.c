#include QMK_KEYBOARD_H

#include "quantum.h"
#include "hydeik.h"

__attribute__ ((weak))
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // if (!process_smtd(keycode, record)) { return false; }

  return process_record_keymap(keycode, record);
}

