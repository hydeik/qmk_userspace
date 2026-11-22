/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The hydeik keymap family with Hold On Other Key Press: what the setting is
 * there for, by the resolution of issue #59, "How do the build targets
 * without Vial get their tap-hold behaviour?".
 *
 * A holding key or a smart mod layer key is a hold as soon as another key is
 * pressed, so the gestures of both state machines work inside the tapping
 * term. With QMK's stock tap-hold behaviour, every scenario here would type
 * the tap keycodes of the keys instead.
 *
 * The state machines are wired as in the firmware and the layers are those of
 * the family, as in tests/target_layer_smart_mod/, which tests them in full
 * with QMK's stock tap-hold behaviour.
 */

#include <initializer_list>

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "layer_lock.h"
#include "smart_mod.h"
#include "target_layer.h"
#include "test_glue.h"
}

using testing::_;
using testing::InSequence;

/*
 * The keymap of the tests is that of users/hydeik/layout.h: three rows of ten
 * finger keys, and the four thumb keys in the columns 3 to 6 of a fourth row.
 * The keys that the tests press, by what they are on each layer (an empty
 * cell is a transparent key, `Sft*` a one-shot mod key, `Btn1` and `Btn2`
 * mouse buttons, `MsDown` the pointer down):
 *
 * | Key           | BASE    | SYM     | NUM     | NAV     | MOU     | FUN     | MOD  |
 * | ------------- | ------- | ------- | ------- | ------- | ------- | ------- | ---- |
 * | `tab_sym`     | TAB_SYM |         | Esc     |         |         |         |      |
 * | `spc_mod`     | SPC_MOD |         |         |         | Btn1    |         |      |
 * | `ent_num`     | ENT_NUM | Esc     |         |         | Btn2    |         |      |
 * | `corner`      | /       | MO(NAV) | MO(FUN) | MO(NAV) | MO(NAV) | MO(FUN) |      |
 * | `left_index`  | F       | :       | 3       | Sft*    | Sft*    | F1      | Sft* |
 * | `right_index` | J       | "       | '       | Left    | MsDown  | Sft*    | Sft* |
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan
 * loop, so every key event here is 1 ms after the one before it, and every
 * gesture ends long before the tapping term has passed.
 */
class HoldOnOtherKeyPress : public TestFixture {
   public:
    static constexpr uint8_t COLUMNS      = 10;
    static constexpr uint8_t FINGER_KEYS  = 30;
    static constexpr uint8_t THUMB_ROW    = 3;
    static constexpr uint8_t THUMB_COLUMN = 3;

    /* The key `index` of the layer `layer` of `test_layout`. */
    static KeymapKey layout_key(uint8_t layer, uint8_t index) {
        const uint8_t row    = index < FINGER_KEYS ? index / COLUMNS : THUMB_ROW;
        const uint8_t column = index < FINGER_KEYS ? index % COLUMNS : THUMB_COLUMN + (index - FINGER_KEYS);
        return KeymapKey{layer, column, row, test_layout[layer][index]};
    }

    KeymapKey tab_sym     = layout_key(TEST_LAYER_BASE, 30);
    KeymapKey spc_mod     = layout_key(TEST_LAYER_BASE, 32);
    KeymapKey ent_num     = layout_key(TEST_LAYER_BASE, 33);
    KeymapKey corner      = layout_key(TEST_LAYER_BASE, 29);
    KeymapKey left_index  = layout_key(TEST_LAYER_BASE, 13);
    KeymapKey right_index = layout_key(TEST_LAYER_BASE, 16);

    HoldOnOtherKeyPress() {
        layer_lock_all_off();
        layer_clear();
        target_layer_reset();
        smart_mod_reset();

        for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
            for (uint8_t index = 0; index < TEST_LAYOUT_KEYS; index++) {
                add_key(layout_key(layer, index));
            }
        }
    }

    /* Press `key`. */
    void push(KeymapKey &key) {
        key.press();
        run_one_scan_loop();
    }

    /* Release `key`. */
    void let_go(KeymapKey &key) {
        key.release();
        run_one_scan_loop();
    }

    /* Press `key`, and release it 1 ms later. */
    void tap(KeymapKey &key) {
        push(key);
        let_go(key);
    }

    /* Expect that exactly `layers` are on, besides the base layer. */
    void expect_layers_on(std::initializer_list<uint8_t> layers) {
        layer_state_t expected = 0;
        for (uint8_t layer : layers) {
            expected |= (layer_state_t)1 << layer;
        }
        EXPECT_EQ(layer_state, expected);
    }

    /* Tap `key`, which sends `code`, and expect that no modifier applies. */
    void expect_unmodified_tap(TestDriver &driver, KeymapKey &key, uint8_t code) {
        InSequence sequence;
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }
};

/*
 * The quick gesture of the left thumb, inside the tapping term: `TAB_SYM`
 * down, the trigger key tapped, `TAB_SYM` up. NAV is entered, as a locked
 * target layer, and neither Tab nor the `/` of the corner key is typed.
 */
TEST_F(HoldOnOtherKeyPress, QuickGestureInsideTheTappingTermEntersNavLocked) {
    TestDriver driver;

    EXPECT_NO_REPORT(driver);
    push(tab_sym);
    tap(corner);
    let_go(tab_sym);
    VERIFY_AND_CLEAR(driver);

    expect_layers_on({TEST_LAYER_NAV});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_NAV));
    expect_unmodified_tap(driver, right_index, KC_LEFT);
}

/*
 * The quick gesture of the right thumb, inside the tapping term: `ENT_NUM`
 * down, the trigger key tapped, `ENT_NUM` up. FUN is entered, as a pending
 * target layer for the next key, and Enter is not typed.
 */
TEST_F(HoldOnOtherKeyPress, QuickGestureInsideTheTappingTermEntersFunPending) {
    TestDriver driver;

    EXPECT_NO_REPORT(driver);
    push(ent_num);
    tap(corner);
    let_go(ent_num);
    VERIFY_AND_CLEAR(driver);

    expect_layers_on({TEST_LAYER_FUN});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_FUN));
    expect_unmodified_tap(driver, left_index, KC_F1);
    expect_layers_on({});
}

/*
 * A smart mod layer key pressed together with the next key is a hold:
 * `SPC_MOD` down and, 1 ms later, the key of Shift on the smart mod layer.
 * Shift is a latched one-shot mod for the keys that follow, and neither
 * Space nor the `F` of that key is typed.
 */
TEST_F(HoldOnOtherKeyPress, SmartModLayerKeyPressedTogetherWithTheNextKeyIsAHold) {
    TestDriver driver;

    EXPECT_NO_REPORT(driver);
    push(spc_mod);
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_LSFT));
    push(left_index);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({TEST_LAYER_MOD});

    EXPECT_NO_REPORT(driver);
    let_go(left_index);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({});

    {
        InSequence sequence;
        EXPECT_REPORT(driver, (KC_LSFT, KC_J));
        EXPECT_REPORT(driver, (KC_LSFT));
        tap(right_index);
        VERIFY_AND_CLEAR(driver);
    }

    EXPECT_EMPTY_REPORT(driver);
    let_go(spc_mod);
    VERIFY_AND_CLEAR(driver);
}
