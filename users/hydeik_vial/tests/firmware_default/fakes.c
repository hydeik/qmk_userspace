/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "fakes.h"

#include <stdint.h>
#include <string.h>

#include "quantum.h"
#include "qmk_settings.h"

/* quantum/via.c */

static bool via_eeprom_valid = false;

void fake_via_eeprom_set_valid(bool valid) {
    via_eeprom_valid = valid;
}

bool via_eeprom_is_valid(void) {
    return via_eeprom_valid;
}

/*
 * The hand of each key for Chordal Hold, which the firmware build generates
 * from the layout of the keyboard. No test switches Chordal Hold on.
 */
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {{0}};

/* quantum/dynamic_keymap.c: the Vial settings as stored in the EEPROM. */

static uint8_t stored_settings[sizeof(qmk_settings_t)];

void fake_stored_settings_erase(void) {
    memset(stored_settings, 0xFF, sizeof(stored_settings));
}

uint8_t dynamic_keymap_get_qmk_settings(uint16_t offset) {
    return stored_settings[offset];
}

void dynamic_keymap_set_qmk_settings(uint16_t offset, uint8_t value) {
    stored_settings[offset] = value;
}
