/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "firmware_default.h"

#include <stdbool.h>
#include <stdint.h>

#include "qmk_settings.h"
#include "via.h"

/*
 * The id of Hold On Other Key Press among the Vial settings: its entry in
 * the table `protos` of quantum/qmk_settings.c, which has no name for it.
 * The Vial GUI names the setting by the same id.
 */
#define SETTING_HOLD_ON_OTHER_KEY_PRESS 23

/* True from `via_init_kb()` on, when VIA is about to reset the settings. */
static bool settings_reset_by_via = false;

/*
 * Switches Hold On Other Key Press on the way the Vial GUI does, which also
 * stores it. To be called after a settings reset only.
 */
static void apply_firmware_defaults(void) {
    const uint8_t on = 1;
    qmk_settings_set(SETTING_HOLD_ON_OTHER_KEY_PRESS, &on, sizeof(on));
}

/*
 * The first boot of a newly built firmware. `via_init()` resets the settings
 * after this hook, and nothing is called right after that reset, so the
 * default has to wait for `keyboard_post_init_user()`.
 */
void firmware_default_via_init(void) {
    settings_reset_by_via = !via_eeprom_is_valid();
}

void firmware_default_keyboard_post_init(void) {
    if (settings_reset_by_via) {
        settings_reset_by_via = false;
        apply_firmware_defaults();
    }
}

/*
 * A reset of the whole EEPROM, as after `EE_CLR`. `eeconfig_init_quantum()`
 * has reset the settings by the time it calls `eeconfig_init_user()`. This
 * reset comes after `via_init()`, and VIA may have found its stored data
 * valid on such a boot, so the path above is not enough.
 */
void firmware_default_eeconfig_init(void) {
    apply_firmware_defaults();
}
