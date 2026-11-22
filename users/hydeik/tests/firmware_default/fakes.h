/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Test-only stand-ins for the parts of vial-qmk that firmware_default and
 * qmk_settings.c call and that the unit tests are built without.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * What `via_eeprom_is_valid()` answers: whether the EEPROM was written by
 * this very firmware build.
 */
void fake_via_eeprom_set_valid(bool valid);

/* Fills the stored Vial settings with the bytes of an erased EEPROM. */
void fake_stored_settings_erase(void);

/*
 * The Vial settings of vial-qmk, reached through C. Its header
 * quantum/qmk_settings.h cannot be included from a C++ test source: it uses
 * `_Static_assert`, which g++ rejects.
 */

/* `qmk_settings_init()`: loads the stored settings, as at every boot. */
void vial_settings_init(void);

/* `qmk_settings_reset()`: what VIA and an EEPROM clear do to the settings. */
void vial_settings_reset(void);

/* `qmk_settings_get()`: reads a setting as the Vial GUI does. 0 on success. */
int vial_settings_get(uint16_t setting, void *value, size_t size);

/* `qmk_settings_set()`: writes a setting as the Vial GUI does. 0 on success. */
int vial_settings_set(uint16_t setting, const void *value, size_t size);

/* Forgets the settings held in memory, as a power cycle does. */
void vial_settings_forget_loaded(void);
