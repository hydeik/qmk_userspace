/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "fakes.h"
#include "firmware_default.h"
}

using testing::_;
using testing::InSequence;

namespace {

/*
 * The ids by which the Vial GUI names the tap-hold options, from the table
 * `protos` in quantum/qmk_settings.c of vial-qmk.
 */
const uint16_t SETTING_PERMISSIVE_HOLD         = 22;
const uint16_t SETTING_HOLD_ON_OTHER_KEY_PRESS = 23;
const uint16_t SETTING_RETRO_TAPPING           = 24;
const uint16_t SETTING_CHORDAL_HOLD            = 26;

/* What the Vial GUI shows for an on/off setting. */
bool shown_in_vial(uint16_t setting) {
    uint8_t value = 0xFF;
    EXPECT_EQ(vial_settings_get(setting, &value, sizeof(value)), 0);
    return value != 0;
}

/* The user switching a setting on or off in the Vial GUI. */
void set_in_vial(uint16_t setting, bool on) {
    uint8_t value = on ? 1 : 0;
    ASSERT_EQ(vial_settings_set(setting, &value, sizeof(value)), 0);
}

/*
 * The boot sequences below call firmware_default and the Vial settings in
 * the order in which vial-qmk calls them (quantum/main.c, quantum/keyboard.c,
 * quantum/via.c, quantum/eeconfig.c):
 *
 *   keyboard_setup()   qmk_settings_init()         load the stored settings
 *   keyboard_init()    via_init()
 *                        via_init_kb()
 *                        qmk_settings_reset()      if the EEPROM is not valid
 *                      quantum_init()
 *                        eeconfig_init()           if the EEPROM is disabled
 *                          qmk_settings_reset()
 *                          eeconfig_init_user()
 *                      keyboard_post_init_user()
 */

/* The first boot of a newly built firmware: VIA finds its EEPROM not valid. */
void boot_newly_built_firmware(void) {
    vial_settings_init();
    fake_via_eeprom_set_valid(false);
    firmware_default_via_init();
    vial_settings_reset();
    fake_via_eeprom_set_valid(true);
    firmware_default_keyboard_post_init();
}

/* The boot after the EEPROM was cleared with `EE_CLR`. */
void boot_after_eeprom_clear(void) {
    vial_settings_init();
    firmware_default_via_init();
    vial_settings_reset();
    firmware_default_eeconfig_init();
    firmware_default_keyboard_post_init();
}

/* Any other boot: the same firmware, the settings as they were stored. */
void power_cycle(void) {
    vial_settings_forget_loaded();
    vial_settings_init();
    firmware_default_via_init();
    firmware_default_keyboard_post_init();
}

} // namespace

class FirmwareDefault : public TestFixture {
  public:
    FirmwareDefault() {
        fake_stored_settings_erase();
        fake_via_eeprom_set_valid(false);
    }
};

TEST_F(FirmwareDefault, HoldOnOtherKeyPressIsOnAfterTheSettingsResetOfANewlyBuiltFirmware) {
    boot_newly_built_firmware();

    EXPECT_TRUE(shown_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS));
}

TEST_F(FirmwareDefault, TheDefaultIsStoredSoItIsStillOnAfterAPowerCycle) {
    boot_newly_built_firmware();

    power_cycle();

    EXPECT_TRUE(shown_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS));
}

TEST_F(FirmwareDefault, NoOtherTapHoldOptionIsSwitchedOn) {
    boot_newly_built_firmware();

    EXPECT_FALSE(shown_in_vial(SETTING_PERMISSIVE_HOLD));
    EXPECT_FALSE(shown_in_vial(SETTING_RETRO_TAPPING));
    EXPECT_FALSE(shown_in_vial(SETTING_CHORDAL_HOLD));
}

TEST_F(FirmwareDefault, HoldOnOtherKeyPressSwitchedOffInVialStaysOffAcrossAPowerCycle) {
    boot_newly_built_firmware();
    set_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS, false);

    power_cycle();

    EXPECT_FALSE(shown_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS));
}

TEST_F(FirmwareDefault, HoldOnOtherKeyPressSwitchedBackOnInVialStaysOnAcrossAPowerCycle) {
    boot_newly_built_firmware();
    set_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS, false);
    power_cycle();
    set_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS, true);

    power_cycle();

    EXPECT_TRUE(shown_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS));
}

TEST_F(FirmwareDefault, HoldOnOtherKeyPressIsOnAfterTheSettingsResetOfAnEepromClear) {
    boot_newly_built_firmware();
    set_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS, false);

    boot_after_eeprom_clear();

    EXPECT_TRUE(shown_in_vial(SETTING_HOLD_ON_OTHER_KEY_PRESS));
}

/*
 * The quick gesture of layer switching: the layer-tap key is still down when
 * another key is pressed and released, all inside the tapping term.
 */
TEST_F(FirmwareDefault, ALayerTapKeyRolledWithAnotherKeyIsAHoldOnANewlyBuiltFirmware) {
    TestDriver driver;
    InSequence s;
    KeymapKey  layer_tap{0, 0, 0, LT(1, KC_TAB)};
    KeymapKey  key_a{0, 1, 0, KC_A};
    KeymapKey  key_b{1, 1, 0, KC_B};
    set_keymap({layer_tap, key_a, key_b});
    boot_newly_built_firmware();

    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_NO_REPORT(driver);
    layer_tap.press();
    run_one_scan_loop();
    key_a.press();
    run_one_scan_loop();
    key_a.release();
    run_one_scan_loop();
    layer_tap.release();
    run_one_scan_loop();

    VERIFY_AND_CLEAR(driver);
}
