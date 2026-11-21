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

/*
 * What `via_eeprom_is_valid()` answers: whether the EEPROM was written by
 * this very firmware build.
 */
void fake_via_eeprom_set_valid(bool valid);

/* Fills the stored Vial settings with the bytes of an erased EEPROM. */
void fake_stored_settings_erase(void);
