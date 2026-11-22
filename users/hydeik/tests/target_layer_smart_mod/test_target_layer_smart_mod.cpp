/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * target_layer together with smart_mod, wired as in the hydeik keymap family
 * and on the layers of that family.
 *
 * The tests named `Keymap_...` are the transitions of the resolution of issue
 * #39, "Which transitions does hydeik_vial get, and where do its trigger keys
 * sit?". The tests named `Issue41_...` are the points of the resolution of
 * issue #41, "How does layer switching interact with the smart mod layer and
 * the one-shot mods of hydeik_vial?"; the second part of such a name is the
 * section of that resolution, listed below. The tests named `Mou_...` are
 * the transitions and the keys of the MOU layer, by the resolution of issue
 * #55, "How is the MOU layer reached and what does it hold?". The tests named
 * `Issue84_...` are what the two state machines answer about their state
 * where both matter, by the resolution of issue #84, "What do smart_mod and
 * target_layer answer about their state, and how is the indicator module
 * cut?".
 *
 * - `TriggerKey`: a trigger key is a layer-switching key to `smart_mod`.
 * - `OneShotModKeys`: one-shot mod keys on a pending target layer.
 * - `Rule15`: the amended timeout of a pending target layer.
 * - `SmartModLayer`: the smart mod layer above a target layer.
 * - `CapsWord`: a trigger key and Caps Word.
 * - `Order3` and `Order4`: requirements 3 and 4 on the order of the two
 *   state machines.
 */

#include <initializer_list>

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "mouse_report_util.hpp"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "caps_word.h"
#include "keyboard.h"
#include "layer_lock.h"
#include "smart_mod.h"
#include "test_glue.h"
#include "target_layer.h"
}

using testing::_;
using testing::AnyNumber;
using testing::InSequence;

/*
 * The keymap of the tests is that of users/hydeik/layout.h: three rows
 * of ten finger keys, and the four thumb keys in the columns 3 to 6 of a
 * fourth row. The keys that the tests press, by what they are on each layer
 * (an empty cell is a transparent key, `-` a key that does nothing, `Ctl*`
 * and `Sft*` one-shot mod keys):
 *
 * | Key            | BASE    | SYM     | NUM     | NAV     | MOU     | FUN     | MOD  |
 * | -------------- | ------- | ------- | ------- | ------- | ------- | ------- | ---- |
 * | `tab_sym`      | TAB_SYM |         | Esc     |         |         |         |      |
 * | `smt_mod`      | SMT_MOD |         |         |         |         |         |      |
 * | `spc_mod`      | SPC_MOD |         |         |         | Btn1    |         |      |
 * | `ent_num`      | ENT_NUM | Esc     |         |         | Btn2    |         |      |
 * | `corner`       | /       | MO(NAV) | MO(FUN) | MO(NAV) | MO(NAV) | MO(FUN) |      |
 * | `left_corner`  | Z       | MO(MOU) | ~       | MO(MOU) | MO(MOU) | -       |      |
 * | `left_pinky`   | A       | *       | 0       | Ctl*    | Ctl*    | -       | Ctl* |
 * | `left_index`   | F       | :       | 3       | Sft*    | Sft*    | F1      | Sft* |
 * | `right_index`  | J       | "       | '       | Down    | MsDown  | Sft*    | Sft* |
 * | `right_pinky`  | ;       | ;       | *       | Bspc    | Btn3    | Ctl*    | Ctl* |
 * | `left_middle`  | C       | @       | 8       | -       | -       | F5      |      |
 * | `layer_lock`   | N       | -       | LLCK    | LLCK    | LLCK    | LLCK    |      |
 *
 * `left_middle` and `layer_lock` are on the bottom row, the others of the
 * fingers on the home row. `corner` is the bottom-right corner and
 * `left_corner` the bottom-left one. `Btn1` to `Btn3` are the mouse buttons
 * and `MsDown` moves the pointer down.
 *
 * The tests are built with QMK's stock tap-hold behaviour, so a holding key
 * or a smart mod layer key is held only once the tapping term has passed.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class TargetLayerSmartMod : public TestFixture {
   public:
    static constexpr uint8_t COLUMNS      = 10;
    static constexpr uint8_t FINGER_KEYS  = 30;
    static constexpr uint8_t THUMB_ROW    = 3;
    static constexpr uint8_t THUMB_COLUMN = 3;

    /* The row and the column of the key `index` of a layer of `test_layout`. */
    static uint8_t row_of(uint8_t index) {
        return index < FINGER_KEYS ? index / COLUMNS : THUMB_ROW;
    }
    static uint8_t column_of(uint8_t index) {
        return index < FINGER_KEYS ? index % COLUMNS : THUMB_COLUMN + (index - FINGER_KEYS);
    }

    /* The key `index` of the layer `layer` of `test_layout`. */
    static KeymapKey layout_key(uint8_t layer, uint8_t index) {
        return KeymapKey{layer, column_of(index), row_of(index), test_layout[layer][index]};
    }

    KeymapKey tab_sym     = layout_key(TEST_LAYER_BASE, 30);
    KeymapKey smt_mod     = layout_key(TEST_LAYER_BASE, 31);
    KeymapKey spc_mod     = layout_key(TEST_LAYER_BASE, 32);
    KeymapKey ent_num     = layout_key(TEST_LAYER_BASE, 33);
    KeymapKey corner      = layout_key(TEST_LAYER_BASE, 29);
    KeymapKey left_corner = layout_key(TEST_LAYER_BASE, 20);
    KeymapKey left_pinky  = layout_key(TEST_LAYER_BASE, 10);
    KeymapKey left_index  = layout_key(TEST_LAYER_BASE, 13);
    KeymapKey right_index = layout_key(TEST_LAYER_BASE, 16);
    KeymapKey right_pinky = layout_key(TEST_LAYER_BASE, 19);
    KeymapKey left_middle = layout_key(TEST_LAYER_BASE, 22);
    KeymapKey layer_lock  = layout_key(TEST_LAYER_BASE, 25);

    TargetLayerSmartMod() {
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

    ~TargetLayerSmartMod() {
        /*
         * Caps Word outlives the clean-up of the test fixture otherwise. A
         * test that turns it on must also turn it off itself, while its
         * `TestDriver` is still there: turning it off sends a report.
         */
        caps_word_off();
    }

    /* Press `key` and keep it down until QMK has made it a hold. */
    void hold(KeymapKey &key) {
        key.press();
        idle_for(TAPPING_TERM + 1);
    }

    /*
     * Run one scan loop that takes less than a millisecond: the time does
     * not move on, as it does not in most scan loops of the firmware.
     */
    void scan_within_the_same_millisecond() {
        keyboard_task();
        housekeeping_task();
    }

    /* Release `key`. */
    void let_go(KeymapKey &key) {
        key.release();
        run_one_scan_loop();
    }

    /* Press `key`, and release it 1 ms later. */
    void tap(KeymapKey &key) {
        key.press();
        run_one_scan_loop();
        key.release();
        run_one_scan_loop();
    }

    /* Expect that exactly `layers` are on, besides the base layer. */
    void expect_layers_on(std::initializer_list<uint8_t> layers) {
        layer_state_t expected = 0;
        for (uint8_t layer : layers) {
            expected |= (layer_state_t)1 << layer;
        }
        EXPECT_EQ(layer_state, expected);
    }

    /* Hold `key`, a holding key or a smart mod layer key: nothing is sent. */
    void hold_silently(TestDriver &driver, KeymapKey &key) {
        EXPECT_NO_REPORT(driver);
        hold(key);
        VERIFY_AND_CLEAR(driver);
    }

    /* Tap `key` and expect that nothing is sent to the host. */
    void tap_silently(TestDriver &driver, KeymapKey &key) {
        EXPECT_NO_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /* Release `key` and expect that nothing is sent to the host. */
    void let_go_silently(TestDriver &driver, KeymapKey &key) {
        EXPECT_NO_REPORT(driver);
        let_go(key);
        VERIFY_AND_CLEAR(driver);
    }

    /* Release `key` and expect the empty report: no modifier remains. */
    void let_go_leaving_no_modifier(TestDriver &driver, KeymapKey &key) {
        EXPECT_EMPTY_REPORT(driver);
        let_go(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * The quick gesture of the left thumb: hold `TAB_SYM`, tap the corner
     * key, release `TAB_SYM` at once. NAV is a locked target layer then.
     */
    void lock_nav(TestDriver &driver) {
        EXPECT_NO_REPORT(driver);
        hold(tab_sym);
        tap(corner);
        let_go(tab_sym);
        VERIFY_AND_CLEAR(driver);
        expect_layers_on({TEST_LAYER_NAV});
    }

    /*
     * The quick gesture of the right thumb: hold `ENT_NUM`, tap the corner
     * key, release `ENT_NUM` at once. FUN is a pending target layer then.
     */
    void leave_fun_pending(TestDriver &driver) {
        EXPECT_NO_REPORT(driver);
        hold(ent_num);
        tap(corner);
        let_go(ent_num);
        VERIFY_AND_CLEAR(driver);
        expect_layers_on({TEST_LAYER_FUN});
    }

    /*
     * The quick gesture of the left thumb with the left corner key: hold
     * `TAB_SYM`, tap the left corner key, release `TAB_SYM` at once. MOU is a
     * locked target layer then.
     */
    void lock_mou(TestDriver &driver) {
        EXPECT_NO_REPORT(driver);
        hold(tab_sym);
        tap(left_corner);
        let_go(tab_sym);
        VERIFY_AND_CLEAR(driver);
        expect_layers_on({TEST_LAYER_MOU});
    }

    /*
     * Tap `key`, a mouse button with the bit `button` in the mouse report,
     * and expect a click: the button down and then up, with the pointer and
     * the wheel at rest, and nothing sent as a keyboard.
     */
    void expect_click(TestDriver &driver, KeymapKey &key, uint8_t button) {
        InSequence sequence;
        EXPECT_NO_REPORT(driver);
        EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, button));
        EXPECT_EMPTY_MOUSE_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /* Tap `key`, which sends `code`, and expect that no modifier applies. */
    void expect_unmodified_tap(TestDriver &driver, KeymapKey &key, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * Tap `key`, which sends `code`, and expect that the registered modifier
     * `mod` applies to it and stays registered afterwards.
     */
    void expect_tap_with_registered(TestDriver &driver, KeymapKey &key, uint8_t code, uint8_t mod) {
        EXPECT_REPORT(driver, (mod, code));
        EXPECT_REPORT(driver, (mod));
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * Tap `key`, which sends `code`, and expect that it consumes the pending
     * one-shot mod `mod`: the modifier is registered at the key press and
     * dropped once the report of the key has been sent.
     */
    void expect_tap_consuming(TestDriver &driver, KeymapKey &key, uint8_t code, uint8_t mod) {
        InSequence sequence;
        EXPECT_REPORT(driver, (mod));
        EXPECT_REPORT(driver, (mod, code));
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * With no smart mod layer key held, tap the one-shot mod key `key` of the
     * modifier `mod`, which leaves a pending one-shot mod: the modifier is
     * registered while the key is down and the host sees none afterwards.
     */
    void tap_one_shot_mod_key(TestDriver &driver, KeymapKey &key, uint8_t mod) {
        EXPECT_REPORT(driver, (mod));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    /*
     * With a smart mod layer key held and the smart mod layer on, tap the
     * one-shot mod key `key` of the modifier `mod`, which latches it: the
     * modifier is registered at the press and stays so after the release.
     */
    void latch(TestDriver &driver, KeymapKey &key, uint8_t mod) {
        EXPECT_REPORT(driver, (mod));
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }
};

/****************************************************************************
 * The transitions of the keymap (issue #39)
 ****************************************************************************/

/* Every `TO()` key is gone: the layers are switched by target_layer alone. */
TEST_F(TargetLayerSmartMod, Keymap_NoToKeyIsLeft) {
    for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
        for (uint8_t index = 0; index < TEST_LAYOUT_KEYS; index++) {
            EXPECT_FALSE(IS_QK_TO(test_layout[layer][index])) << "layer " << +layer << ", key " << +index;
        }
    }
}

/*
 * `TAB_SYM` and the corner key reach NAV, which stays on while `TAB_SYM` is
 * held. Arrows are typed, and the release does not lock: a key was typed.
 */
TEST_F(TargetLayerSmartMod, Keymap_TabSymAndTheCornerKeyReachNavWhileHeld) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    expect_unmodified_tap(driver, right_index, KC_DOWN);

    let_go_silently(driver, tab_sym);
    expect_layers_on({});
}

/* A quick release of `TAB_SYM` locks NAV, and the corner key leaves it. */
TEST_F(TargetLayerSmartMod, Keymap_QuickReleaseOfTabSymLocksNavAndTheCornerKeyLeavesIt) {
    TestDriver driver;

    lock_nav(driver);
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_NAV));

    /* Locked, it is no pending target layer: it outlives a typed key. */
    expect_unmodified_tap(driver, right_index, KC_DOWN);
    expect_unmodified_tap(driver, right_index, KC_DOWN);
    expect_layers_on({TEST_LAYER_NAV});

    tap_silently(driver, corner);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_NAV));
    expect_unmodified_tap(driver, right_index, KC_J);
}

/* The Layer Lock key leaves a locked NAV too. */
TEST_F(TargetLayerSmartMod, Keymap_LayerLockKeyLeavesALockedNav) {
    TestDriver driver;

    lock_nav(driver);

    tap_silently(driver, layer_lock);
    expect_layers_on({});
    expect_unmodified_tap(driver, right_index, KC_J);
}

/*
 * The thumb keys of NAV are transparent: on a locked NAV they are those of
 * BASE, and no `TO()` key is needed to leave.
 */
TEST_F(TargetLayerSmartMod, Keymap_ThumbKeysOfALockedNavAreThoseOfBase) {
    TestDriver driver;

    lock_nav(driver);

    expect_unmodified_tap(driver, tab_sym, KC_TAB);
    expect_unmodified_tap(driver, spc_mod, KC_SPC);
    expect_unmodified_tap(driver, ent_num, KC_ENT);
    expect_layers_on({TEST_LAYER_NAV});
}

/*
 * `ENT_NUM` and the corner key reach FUN, which stays on while `ENT_NUM` is
 * held.
 */
TEST_F(TargetLayerSmartMod, Keymap_EntNumAndTheCornerKeyReachFunWhileHeld) {
    TestDriver driver;

    hold_silently(driver, ent_num);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_FUN});

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_unmodified_tap(driver, left_middle, KC_F5);

    let_go_silently(driver, ent_num);
    expect_layers_on({});
}

/* A quick release of `ENT_NUM` leaves FUN pending, for one key. */
TEST_F(TargetLayerSmartMod, Keymap_QuickReleaseOfEntNumLeavesFunPendingForOneKey) {
    TestDriver driver;

    leave_fun_pending(driver);
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_FUN));

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
    expect_unmodified_tap(driver, left_middle, KC_C);
}

/* The corner key of FUN is its trigger key: it cancels a pending FUN. */
TEST_F(TargetLayerSmartMod, Keymap_CornerKeyCancelsAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    tap_silently(driver, corner);
    expect_layers_on({});
    expect_unmodified_tap(driver, left_middle, KC_C);
}

/*
 * The corner key pressed again while the thumb is held goes back to the
 * holding layer, on either hand.
 */
TEST_F(TargetLayerSmartMod, Keymap_CornerKeyPressedAgainGoesBackToTheHoldingLayer) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    tap_silently(driver, corner);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM});
    let_go_silently(driver, tab_sym);

    hold_silently(driver, ent_num);
    tap_silently(driver, corner);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_NUM});
    let_go_silently(driver, ent_num);
    expect_layers_on({});
}

/****************************************************************************
 * The MOU layer (issue #55)
 ****************************************************************************/

/*
 * `TAB_SYM` and the left corner key reach MOU, which stays on while `TAB_SYM`
 * is held. A release that is no quick release switches it off.
 */
TEST_F(TargetLayerSmartMod, Mou_TabSymAndTheLeftCornerKeyReachMouWhileHeld) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    tap_silently(driver, left_corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_MOU});

    EXPECT_NO_REPORT(driver);
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_MOU});

    let_go_silently(driver, tab_sym);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_MOU));
}

/*
 * A quick release of `TAB_SYM` locks MOU, and the left corner key, its
 * trigger key, leaves it. The right thumb keys of a locked MOU are the left
 * and the right mouse button, and the right pinky is the middle one.
 */
TEST_F(TargetLayerSmartMod, Mou_QuickReleaseOfTabSymLocksMouAndTheLeftCornerKeyLeavesIt) {
    TestDriver driver;

    lock_mou(driver);
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_MOU));

    /* Locked, it is no pending target layer: it outlives a typed key. */
    expect_click(driver, spc_mod, MOUSE_BTN1);
    expect_click(driver, ent_num, MOUSE_BTN2);
    expect_click(driver, right_pinky, MOUSE_BTN3);
    expect_layers_on({TEST_LAYER_MOU});

    tap_silently(driver, left_corner);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_MOU));
    expect_unmodified_tap(driver, spc_mod, KC_SPC);
}

/* The Layer Lock key leaves a locked MOU too. */
TEST_F(TargetLayerSmartMod, Mou_LayerLockKeyLeavesALockedMou) {
    TestDriver driver;

    lock_mou(driver);

    tap_silently(driver, layer_lock);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_MOU));
    expect_unmodified_tap(driver, spc_mod, KC_SPC);
}

/*
 * The two corners mean the same on SYM, NAV and MOU. While `TAB_SYM` stays
 * held, the corner key, the trigger key of NAV, replaces MOU with NAV, and
 * the left corner key, the trigger key of MOU, replaces NAV with MOU.
 */
TEST_F(TargetLayerSmartMod, Mou_CornerKeysSwapMouAndNavWhileTabSymStaysHeld) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    tap_silently(driver, left_corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_MOU});

    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});
    expect_unmodified_tap(driver, right_index, KC_DOWN);

    tap_silently(driver, left_corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_MOU});
    expect_click(driver, spc_mod, MOUSE_BTN1);

    let_go_silently(driver, tab_sym);
    expect_layers_on({});
}

/*
 * A quick release of `TAB_SYM` after the swap locks the target layer that is
 * on, not the one it replaced, in either direction.
 */
TEST_F(TargetLayerSmartMod, Mou_QuickReleaseAfterASwapLocksTheTargetLayerThatIsOn) {
    TestDriver driver;

    EXPECT_NO_REPORT(driver);
    hold(tab_sym);
    tap(left_corner);
    tap(corner);
    let_go(tab_sym);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({TEST_LAYER_NAV});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_NAV));
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_MOU));

    tap_silently(driver, corner);
    expect_layers_on({});

    EXPECT_NO_REPORT(driver);
    hold(tab_sym);
    tap(corner);
    tap(left_corner);
    let_go(tab_sym);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({TEST_LAYER_MOU});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_MOU));
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_NAV));
}

/*
 * A mouse button is a typed key: a click consumes a pending one-shot mod. The
 * modifier is registered before the button goes down and dropped once the
 * button has been sent, and the next click is a plain one.
 */
TEST_F(TargetLayerSmartMod, Mou_MouseButtonConsumesAPendingOneShotMod) {
    TestDriver driver;

    lock_mou(driver);
    tap_one_shot_mod_key(driver, left_pinky, KC_LCTL);

    {
        InSequence sequence;
        EXPECT_REPORT(driver, (KC_LCTL));
        EXPECT_MOUSE_REPORT(driver, (0, 0, 0, 0, MOUSE_BTN1));
        EXPECT_EMPTY_REPORT(driver);
        EXPECT_EMPTY_MOUSE_REPORT(driver);
        tap(spc_mod);
        VERIFY_AND_CLEAR(driver);
    }

    expect_click(driver, spc_mod, MOUSE_BTN1);
    expect_layers_on({TEST_LAYER_MOU});
}

/*
 * A one-shot mod key of MOU that is held is an ordinary modifier: it applies
 * to the click, and nothing is pending once it is released.
 */
TEST_F(TargetLayerSmartMod, Mou_HeldOneShotModKeyModifiesAClick) {
    TestDriver driver;

    lock_mou(driver);

    EXPECT_REPORT(driver, (KC_LSFT));
    left_index.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    /* Shift stays registered while the button goes down and up. */
    expect_click(driver, spc_mod, MOUSE_BTN1);

    let_go_leaving_no_modifier(driver, left_index);

    expect_click(driver, spc_mod, MOUSE_BTN1);
    expect_layers_on({TEST_LAYER_MOU});
}

/****************************************************************************
 * Issue #41: a trigger key is a layer-switching key to smart_mod
 ****************************************************************************/

/*
 * On a locked NAV: tap `COS_LCTL`, leave NAV with the trigger key, and the
 * next key on BASE gets Ctrl.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_LeavingALockedNavDoesNotConsumeAPendingOneShotMod) {
    TestDriver driver;

    lock_nav(driver);
    tap_one_shot_mod_key(driver, left_pinky, KC_LCTL);

    tap_silently(driver, corner);
    expect_layers_on({});

    expect_tap_consuming(driver, right_index, KC_J, KC_LCTL);
    expect_unmodified_tap(driver, right_index, KC_J);
}

/*
 * Cancelling a pending FUN with the trigger key leaves pending one-shot mods
 * pending.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_CancellingAPendingFunLeavesPendingOneShotModsPending) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);

    tap_silently(driver, corner);
    expect_layers_on({});

    expect_tap_consuming(driver, left_middle, KC_C, KC_RCTL);
}

/*
 * The trigger gesture does not consume a pending one-shot mod either: tap
 * Shift, go to NAV with `TAB_SYM` and the trigger key, and the arrow gets
 * Shift.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_TriggerGestureDoesNotConsumeAPendingOneShotMod) {
    TestDriver driver;

    tap_silently(driver, smt_mod);
    hold_silently(driver, tab_sym);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    expect_tap_consuming(driver, right_index, KC_DOWN, KC_LSFT);
    let_go_silently(driver, tab_sym);
}

/*
 * A trigger key is not a typed key for the smart mod layer: latched one-shot
 * mods stay latched. Hold `SPC_MOD`, latch Ctrl, go to NAV with `TAB_SYM`
 * and the trigger key: Ctrl+arrows.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_LatchedOneShotModsStayLatchedAcrossTheTriggerKey) {
    TestDriver driver;

    hold_silently(driver, spc_mod);
    latch(driver, left_pinky, KC_LCTL);

    hold_silently(driver, tab_sym);
    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    expect_tap_with_registered(driver, right_index, KC_DOWN, KC_LCTL);
    expect_tap_with_registered(driver, right_index, KC_DOWN, KC_LCTL);

    let_go_silently(driver, tab_sym);
    expect_layers_on({});
    let_go_leaving_no_modifier(driver, spc_mod);
}

/*
 * A trigger key is not a typed key for the smart mod layer: the release
 * window stays open. With `TAB_SYM` held: hold `SPC_MOD`, latch Ctrl, press
 * the trigger key and release `SPC_MOD` within the release window. Ctrl is a
 * pending one-shot mod then, for the arrow on NAV.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_ReleaseWindowStaysOpenAcrossTheTriggerKey) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    hold_silently(driver, spc_mod);
    latch(driver, left_pinky, KC_LCTL);

    tap_silently(driver, corner);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go_leaving_no_modifier(driver, spc_mod);

    expect_tap_consuming(driver, right_index, KC_DOWN, KC_LCTL);
    expect_unmodified_tap(driver, right_index, KC_DOWN);
    let_go_silently(driver, tab_sym);
}

/*
 * Like every key press, a trigger key ends the tap of a one-shot mod key
 * that is still down: `COS_LSFT` down, trigger key, `COS_LSFT` up leaves
 * nothing pending.
 */
TEST_F(TargetLayerSmartMod, Issue41_TriggerKey_EndsTheTapOfAOneShotModKeyThatIsStillDown) {
    TestDriver driver;

    lock_nav(driver);

    EXPECT_REPORT(driver, (KC_LSFT));
    left_index.press();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    tap_silently(driver, corner);
    expect_layers_on({});

    let_go_leaving_no_modifier(driver, left_index);
    expect_unmodified_tap(driver, right_index, KC_J);
}

/****************************************************************************
 * Issue #41: one-shot mod keys on a pending target layer
 ****************************************************************************/

/*
 * A tapped one-shot mod key is not a typed key: the pending target layer
 * lasts until the modified key. Pending FUN, tap `COS_RCTL`, F5 sends
 * Ctrl+F5.
 */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_TappedOneShotModKeyDoesNotUseUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
}

/*
 * A held one-shot mod key is not a typed key either: it is an ordinary
 * modifier for the key of the pending target layer.
 */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_HeldOneShotModKeyDoesNotUseUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    EXPECT_REPORT(driver, (KC_RCTL));
    right_pinky.press();
    idle_for(SMART_MOD_ONE_SHOT_TERM + 100);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_with_registered(driver, left_middle, KC_F5, KC_RCTL);
    expect_layers_on({});

    let_go_leaving_no_modifier(driver, right_pinky);
}

/*
 * A tap of `SMT_MOD` counts as a one-shot mod key press: it sends nothing,
 * makes Shift pending and does not use up the pending target layer.
 */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_TapOfSmtModDoesNotUseUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    tap_silently(driver, smt_mod);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_LSFT);
    expect_layers_on({});
}

/* So does the tap of `SMT_MOD` that cancels the pending one-shot mods. */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_CancellingTapOfSmtModDoesNotUseUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);

    tap_silently(driver, smt_mod);
    expect_layers_on({TEST_LAYER_FUN});

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
}

/* So does the double tap of `SMT_MOD` that turns Caps Word on. */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_DoubleTapOfSmtModForCapsWordDoesNotUseUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    tap_silently(driver, smt_mod);
    tap_silently(driver, smt_mod);
    EXPECT_TRUE(is_caps_word_on());
    expect_layers_on({TEST_LAYER_FUN});

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
}

/*
 * A tap of `SPC_MOD` is Space, a typed key: Space is sent and the pending
 * target layer ends.
 */
TEST_F(TargetLayerSmartMod, Issue41_OneShotModKeys_TapOfSpcModIsATypedKeyThatUsesUpAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    expect_unmodified_tap(driver, spc_mod, KC_SPC);
    expect_layers_on({});
}

/****************************************************************************
 * Issue #41: rule 15 amended, the timeout of a pending target layer
 ****************************************************************************/

/*
 * Every press of a one-shot mod key restarts the timeout of a pending target
 * layer, so FUN is still there for the modified key.
 */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_OneShotModKeyPressRestartsTheTimeoutOfAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 500);

    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);
    idle_for(1000);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
    expect_layers_on({});
}

/* Every tap of `SMT_MOD` restarts the timeout too. */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_TapOfSmtModRestartsTheTimeoutOfAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 500);

    tap_silently(driver, smt_mod);
    idle_for(1000);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_LSFT);
    expect_layers_on({});
}

/*
 * A pending target layer does not time out while a smart mod layer key is
 * held, and its timeout starts anew when that key is released.
 */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_PendingFunDoesNotTimeOutWhileASmartModLayerKeyIsHeld) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    leave_fun_pending(driver);

    EXPECT_NO_REPORT(driver);
    hold(spc_mod);
    idle_for(2 * TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({TEST_LAYER_FUN, TEST_LAYER_MOD});

    let_go(spc_mod);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 10);
    expect_layers_on({TEST_LAYER_FUN});

    idle_for(20);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * The pending FUN does not end before one-shot mods that became pending at
 * the release of a smart mod layer key: the timeout of FUN starts anew no
 * earlier than theirs. Here Ctrl is latched and `SPC_MOD` released within
 * the release window, and the key comes in the last millisecond before the
 * pending Ctrl ends, after a scan loop that took less than that millisecond.
 * It is the key of FUN with Ctrl, never a key of BASE with Ctrl.
 */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_PendingFunDoesNotEndBeforeOneShotModsMadePendingAtTheReleaseOfASmartModLayerKey) {
    TestDriver driver;

    leave_fun_pending(driver);
    hold_silently(driver, spc_mod);
    latch(driver, right_pinky, KC_RCTL);
    let_go_leaving_no_modifier(driver, spc_mod);
    expect_layers_on({TEST_LAYER_FUN});

    /* One scan loop has passed since the release of `SPC_MOD`. */
    EXPECT_NO_REPORT(driver);
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 2);
    scan_within_the_same_millisecond();
    VERIFY_AND_CLEAR(driver);

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
    expect_layers_on({});
}

/*
 * The same at the very moment at which the pending Ctrl ends: the key is
 * that of FUN without Ctrl, never a key of BASE with Ctrl.
 */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_PendingFunIsStillOnWhenOneShotModsMadePendingAtTheReleaseOfASmartModLayerKeyEnd) {
    TestDriver driver;

    leave_fun_pending(driver);
    hold_silently(driver, spc_mod);
    latch(driver, right_pinky, KC_RCTL);
    let_go_leaving_no_modifier(driver, spc_mod);

    /* One scan loop has passed since the release of `SPC_MOD`. */
    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 1);

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
}

/****************************************************************************
 * Issue #41: the smart mod layer above a target layer
 ****************************************************************************/

/*
 * Locked NAV: hold `SMT_MOD`, latch Shift, the smart mod layer switches
 * itself off, Shift+arrows select. Releasing `SMT_MOD` releases Shift; NAV
 * stays locked.
 */
TEST_F(TargetLayerSmartMod, Issue41_SmartModLayer_WorksAboveALockedNav) {
    TestDriver driver;

    lock_nav(driver);

    hold_silently(driver, smt_mod);
    expect_layers_on({TEST_LAYER_NAV, TEST_LAYER_MOD});

    latch(driver, left_index, KC_LSFT);
    expect_layers_on({TEST_LAYER_NAV});

    expect_tap_with_registered(driver, right_index, KC_DOWN, KC_LSFT);
    expect_tap_with_registered(driver, right_index, KC_DOWN, KC_LSFT);

    let_go_leaving_no_modifier(driver, smt_mod);
    expect_layers_on({TEST_LAYER_NAV});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_NAV));
}

/*
 * Pending FUN: holding `SPC_MOD` is not a typed key, so FUN stays pending;
 * latch Ctrl, F5 sends Ctrl+F5 and uses up FUN; releasing `SPC_MOD` releases
 * Ctrl.
 */
TEST_F(TargetLayerSmartMod, Issue41_SmartModLayer_WorksAboveAPendingFun) {
    TestDriver driver;

    leave_fun_pending(driver);

    hold_silently(driver, spc_mod);
    expect_layers_on({TEST_LAYER_FUN, TEST_LAYER_MOD});

    latch(driver, left_pinky, KC_LCTL);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_with_registered(driver, left_middle, KC_F5, KC_LCTL);
    expect_layers_on({});

    let_go_leaving_no_modifier(driver, spc_mod);
}

/*
 * A held target layer: with `TAB_SYM` and NAV held, the smart mod layer
 * latches Shift for the arrows, and neither state machine switches the
 * other's layer off.
 */
TEST_F(TargetLayerSmartMod, Issue41_SmartModLayer_WorksAboveAHeldNav) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    tap_silently(driver, corner);

    hold_silently(driver, smt_mod);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV, TEST_LAYER_MOD});

    latch(driver, left_index, KC_LSFT);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    expect_tap_with_registered(driver, right_index, KC_DOWN, KC_LSFT);

    let_go_leaving_no_modifier(driver, smt_mod);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go_silently(driver, tab_sym);
    expect_layers_on({});
}

/****************************************************************************
 * Issue #41: Caps Word
 ****************************************************************************/

/*
 * A trigger key does not turn Caps Word off, neither when it switches a
 * target layer on nor when it leaves a locked one.
 *
 * Caps Word sends reports of its own around a shifted letter (a lone Shift,
 * an empty report), so those are allowed in any number.
 */
TEST_F(TargetLayerSmartMod, Issue41_CapsWord_TriggerKeyKeepsCapsWordOn) {
    TestDriver driver;

    tap_silently(driver, smt_mod);
    tap_silently(driver, smt_mod);
    ASSERT_TRUE(is_caps_word_on());

    lock_nav(driver);
    EXPECT_TRUE(is_caps_word_on());

    tap_silently(driver, corner);
    expect_layers_on({});
    EXPECT_TRUE(is_caps_word_on());

    EXPECT_REPORT(driver, (KC_LSFT)).Times(AnyNumber());
    EXPECT_REPORT(driver, (KC_LSFT, KC_J));
    EXPECT_EMPTY_REPORT(driver).Times(AnyNumber());
    tap(right_index);
    VERIFY_AND_CLEAR(driver);

    /*
     * Space ends Caps Word. It must be off before `driver` goes: turning it
     * off sends a report.
     */
    EXPECT_REPORT(driver, (KC_SPC));
    EXPECT_EMPTY_REPORT(driver).Times(AnyNumber());
    tap(spc_mod);
    VERIFY_AND_CLEAR(driver);
    EXPECT_FALSE(is_caps_word_on());
}

/****************************************************************************
 * Issue #41: the order of the two state machines
 ****************************************************************************/

/*
 * Requirement 3. For a key that uses up a pending target layer and a pending
 * one-shot mod at once: the modifier is registered, the key is sent from the
 * target layer with it, and only then the layer goes off and the modifier is
 * dropped. The next key is a plain key of BASE.
 */
TEST_F(TargetLayerSmartMod, Issue41_Order3_KeyThatUsesUpAPendingFunAndAPendingOneShotModIsSentFromFunWithTheModifier) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
    expect_layers_on({});

    expect_unmodified_tap(driver, left_middle, KC_C);
}

/*
 * Requirement 3, with the second key rolled over the first: it is a plain
 * key of BASE, with neither the layer nor the modifier of the first.
 */
TEST_F(TargetLayerSmartMod, Issue41_Order3_KeyRolledOverTheOneThatUsedBothUpIsAPlainKeyOfBase) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);

    {
        InSequence sequence;
        EXPECT_REPORT(driver, (KC_RCTL));
        EXPECT_REPORT(driver, (KC_RCTL, KC_F5));
        EXPECT_REPORT(driver, (KC_F5));
        EXPECT_REPORT(driver, (KC_F5, KC_J));
        EXPECT_REPORT(driver, (KC_J));
        EXPECT_EMPTY_REPORT(driver);
    }
    left_middle.press();
    run_one_scan_loop();
    right_index.press();
    run_one_scan_loop();
    left_middle.release();
    run_one_scan_loop();
    right_index.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

/*
 * Requirement 4. A tap of `SMT_MOD` on a pending FUN starts both timeouts:
 * that of the pending Shift at its press, and that of FUN anew at its
 * release, here one scan loop later. Up to the last moment before the
 * pending Shift ends, the key is that of FUN with Shift.
 */
TEST_F(TargetLayerSmartMod, Issue41_Order4_JustBeforeThePendingOneShotModEndsTheKeyIsThatOfFunWithIt) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_silently(driver, smt_mod);
    /* Two scan loops have passed since the press of that tap. */
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 3);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_LSFT);
}

/*
 * Requirement 4. A key pressed at the very moment at which the pending
 * one-shot mod ends finds the pending target layer not ended before it: it is
 * the key of FUN without Shift, never a key of BASE with Shift.
 */
TEST_F(TargetLayerSmartMod, Issue41_Order4_WhenThePendingOneShotModEndsThePendingFunHasNotEndedBeforeIt) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_silently(driver, smt_mod);
    /* Two scan loops have passed since the press of that tap. */
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 2);

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
}

/*
 * Requirement 4. The release of a tapped one-shot mod key starts both
 * timeouts at the same moment, so they end together. A key pressed at that
 * very moment finds the pending one-shot mod ended and the pending target
 * layer not ended before it: it is the key of FUN without Ctrl, never a key
 * of BASE with Ctrl.
 */
TEST_F(TargetLayerSmartMod, Issue41_Order4_WhenBothTimeoutsEndTogetherThePendingFunDoesNotEndBeforeThePendingOneShotMod) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);
    /* One scan loop has passed since the release of that tap. */
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 1);

    expect_unmodified_tap(driver, left_middle, KC_F5);
    expect_layers_on({});
}

/* Requirement 4. Once both timeouts have ended, nothing of either is left. */
TEST_F(TargetLayerSmartMod, Issue41_Order4_AfterBothTimeoutsEndedTheKeyIsAPlainKeyOfBase) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_silently(driver, smt_mod);
    /* One scan loop has passed since the release of that tap. */
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({});

    expect_unmodified_tap(driver, left_middle, KC_C);
}

/****************************************************************************
 * Issue #41: rule 15 amended again, a one-shot mod key that is tapped slowly
 ****************************************************************************/

/*
 * The release of a one-shot mod key restarts the timeout of a pending target
 * layer too. The timeout of the pending one-shot mod counts from the release
 * of its tap, up to a One Shot Term after the press, so a timeout of FUN
 * counted from the press would end FUN that much before the pending one-shot
 * mod, and a key pressed in between would be a key of BASE with the
 * modifier. Here the tap takes 150 ms, and the key comes 50 ms before the
 * pending one-shot mod ends.
 */
TEST_F(TargetLayerSmartMod, Issue41_Rule15_PendingFunDoesNotEndBeforeAOneShotModThatWasTappedSlowly) {
    TestDriver driver;

    leave_fun_pending(driver);

    EXPECT_REPORT(driver, (KC_RCTL));
    EXPECT_EMPTY_REPORT(driver);
    right_pinky.press();
    idle_for(150);
    right_pinky.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    idle_for(SMART_MOD_ONE_SHOT_TIMEOUT - 50);
    expect_layers_on({TEST_LAYER_FUN});

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
}

/****************************************************************************
 * Issue #84: what the two state machines answer about their state
 ****************************************************************************/

/*
 * A key that uses up a pending FUN and a pending one-shot mod at once takes
 * both out of the answers.
 */
TEST_F(TargetLayerSmartMod, Issue84_KeyThatUsesUpAPendingFunAndAPendingOneShotModTakesBothOutOfTheAnswers) {
    TestDriver driver;

    leave_fun_pending(driver);
    tap_one_shot_mod_key(driver, right_pinky, KC_RCTL);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_FUN), TARGET_LAYER_STATE_PENDING);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_RCTRL);

    expect_tap_consuming(driver, left_middle, KC_F5, KC_RCTL);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_FUN), TARGET_LAYER_STATE_OFF);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
}

/*
 * One-shot mods chosen on the smart mod layer above a locked NAV are latched
 * while NAV stays locked; the smart mod layer is no target layer and is off
 * to target_layer. A quick release of `SMT_MOD` makes them pending, and the
 * trigger key that leaves NAV does not consume them.
 */
TEST_F(TargetLayerSmartMod, Issue84_OneShotModsChosenAboveALockedNavAreAnsweredBesideIt) {
    TestDriver driver;

    lock_nav(driver);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);

    hold_silently(driver, smt_mod);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_MOD), TARGET_LAYER_STATE_OFF);

    latch(driver, left_index, KC_LSFT);
    EXPECT_EQ(smart_mod_latched_mods(), MOD_BIT_LSHIFT);
    EXPECT_EQ(smart_mod_pending_mods(), 0);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);

    let_go_leaving_no_modifier(driver, smt_mod);
    EXPECT_EQ(smart_mod_latched_mods(), 0);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_LOCKED);

    tap_silently(driver, corner);
    EXPECT_EQ(target_layer_state_of(TEST_LAYER_NAV), TARGET_LAYER_STATE_OFF);
    EXPECT_EQ(smart_mod_pending_mods(), MOD_BIT_LSHIFT);
}

/*
 * By the table of entries of the keymap family: `TAB_SYM` makes the trigger
 * keys of NAV and MOU work and `ENT_NUM` that of FUN. A held smart mod layer
 * key is no holding key and makes no trigger key work.
 */
TEST_F(TargetLayerSmartMod, Issue84_TriggerKeysWorkWhileTheirHoldingKeyIsHeldAndNotForASmartModLayerKey) {
    TestDriver driver;

    hold_silently(driver, tab_sym);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_MOU));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));
    let_go_silently(driver, tab_sym);

    hold_silently(driver, ent_num);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_MOU));
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));
    let_go_silently(driver, ent_num);

    hold_silently(driver, spc_mod);
    for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
        EXPECT_FALSE(target_layer_is_trigger_key_working(layer)) << "layer " << +layer;
    }
    let_go_silently(driver, spc_mod);
}
