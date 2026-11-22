/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The rules of the quick release of a holding key: the pending target layer
 * and the locked target layer. "Rule N" refers to the numbered rules of the
 * resolution of issue #38, "How should the layer switch behave?". The
 * amendments of that resolution are named by the issue that made them:
 * "Amendment39" is the locked target layer and the changed rule 18,
 * "Amendment41" the changed rule 15, and "Issue40" a point of issue #40. Each
 * test carries the number of its rule or amendment in its name.
 */

#include <initializer_list>

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "layer_lock.h"
#include "target_layer.h"
#include "test_glue.h"
}

using testing::_;
using testing::AnyNumber;
using testing::InSequence;

/*
 * The keymap of the tests, one row of ten keys. An empty cell is a
 * transparent key. PEN is the target layer that a quick release leaves
 * pending, LCK the one that it locks.
 *
 * | Column | 0       | 1       | 2       | 3    | 4       | 5       | 6       | 7         | 8    | 9       |
 * | ------ | ------- | ------- | ------- | ---- | ------- | ------- | ------- | --------- | ---- | ------- |
 * | BASE   | TAB_SYM | ENT_NUM | A       | B    | C       | D       | QK_LLCK | OSM(LCTL) | LSFT | E       |
 * | SYM    |         |         | MO(LCK) | 1    |         | MO(PEN) |         |           |      |         |
 * | NUM    |         |         | MO(LCK) | 2    |         | MO(PEN) |         |           |      | MO(FUN) |
 * | FUN    |         |         |         | F12  |         |         |         |           |      | MO(FUN) |
 * | PEN    |         |         |         | F1   | F2      | MO(PEN) |         |           |      |         |
 * | LCK    |         |         | MO(LCK) | LEFT | MS_BTN1 |         |         |           |      |         |
 *
 * `key_lck`, `key_pen` and `key_fun` are the positions of the trigger keys.
 * The quick release of FUN does nothing.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class TargetLayerQuickRelease : public TestFixture {
   public:
    static constexpr uint8_t COLUMNS = 10;

    KeymapKey key_tab_sym{TEST_LAYER_BASE, 0, 0, TAB_SYM};
    KeymapKey key_ent_num{TEST_LAYER_BASE, 1, 0, ENT_NUM};
    KeymapKey key_lck{TEST_LAYER_BASE, 2, 0, KC_A};
    KeymapKey key_typed{TEST_LAYER_BASE, 3, 0, KC_B};
    KeymapKey key_typed_next{TEST_LAYER_BASE, 4, 0, KC_C};
    KeymapKey key_pen{TEST_LAYER_BASE, 5, 0, KC_D};
    KeymapKey key_layer_lock{TEST_LAYER_BASE, 6, 0, QK_LLCK};
    KeymapKey key_one_shot_ctrl{TEST_LAYER_BASE, 7, 0, OSM(MOD_LCTL)};
    KeymapKey key_shift{TEST_LAYER_BASE, 8, 0, KC_LSFT};
    KeymapKey key_fun{TEST_LAYER_BASE, 9, 0, KC_E};

    TargetLayerQuickRelease() {
        layer_lock_all_off();
        layer_clear();
        target_layer_reset();
        test_pending_timeout_suspended = false;

        for (const KeymapKey &key : {key_tab_sym, key_ent_num, key_lck, key_typed, key_typed_next, key_pen, key_layer_lock, key_one_shot_ctrl, key_shift, key_fun}) {
            add_key(key);
        }
        for (const KeymapKey &key : {
                 KeymapKey{TEST_LAYER_SYM, 2, 0, MO(TEST_LAYER_LCK)},
                 KeymapKey{TEST_LAYER_SYM, 3, 0, KC_1},
                 KeymapKey{TEST_LAYER_SYM, 5, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_NUM, 2, 0, MO(TEST_LAYER_LCK)},
                 KeymapKey{TEST_LAYER_NUM, 3, 0, KC_2},
                 KeymapKey{TEST_LAYER_NUM, 5, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_NUM, 9, 0, MO(TEST_LAYER_FUN)},
                 KeymapKey{TEST_LAYER_FUN, 3, 0, KC_F12},
                 KeymapKey{TEST_LAYER_FUN, 9, 0, MO(TEST_LAYER_FUN)},
                 KeymapKey{TEST_LAYER_PEN, 3, 0, KC_F1},
                 KeymapKey{TEST_LAYER_PEN, 4, 0, KC_F2},
                 KeymapKey{TEST_LAYER_PEN, 5, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_LCK, 2, 0, MO(TEST_LAYER_LCK)},
                 KeymapKey{TEST_LAYER_LCK, 3, 0, KC_LEFT},
                 KeymapKey{TEST_LAYER_LCK, 4, 0, MS_BTN1},
             }) {
            add_key(key);
        }
        /* Every cell that is still empty is a transparent key. */
        for (uint8_t layer = TEST_LAYER_SYM; layer < TEST_LAYER_COUNT; layer++) {
            for (uint8_t column = 0; column < COLUMNS; column++) {
                if (find_key(layer, {column, 0}) == nullptr) {
                    add_key(KeymapKey{layer, column, 0, KC_TRNS});
                }
            }
        }
    }

    /* Press `key` and keep it down until QMK has made it a hold. */
    void hold(KeymapKey &key) {
        key.press();
        idle_for(TAPPING_TERM + 1);
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

    /*
     * Tap the trigger key `trigger`, then release the holding key `holding`
     * `ms` after the press of the trigger key. `ms` is at least 2.
     */
    void tap_and_let_go_after(KeymapKey &trigger, KeymapKey &holding, uint16_t ms) {
        tap(trigger);
        idle_for(ms - 2);
        let_go(holding);
    }

    /* The quick gesture: hold `holding`, tap `trigger`, release `holding`. */
    void quick_gesture(KeymapKey &holding, KeymapKey &trigger) {
        hold(holding);
        tap(trigger);
        let_go(holding);
    }

    /* Expect that exactly `layers` are on, besides the base layer. */
    void expect_layers_on(std::initializer_list<uint8_t> layers) {
        layer_state_t expected = 0;
        for (uint8_t layer : layers) {
            expected |= (layer_state_t)1 << layer;
        }
        EXPECT_EQ(layer_state, expected);
    }

    /* Tap `key_typed` and expect that it sends `code`. */
    void expect_typed(TestDriver &driver, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key_typed);
        VERIFY_AND_CLEAR(driver);
    }
};

/*
 * Rule 10: releasing the last holding key right after the trigger press,
 * with no key typed, leaves the target layer pending. QMK switches the
 * holding layer off.
 */
TEST_F(TargetLayerQuickRelease, Rule10_QuickReleaseLeavesTheTargetLayerPending) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_PEN});

    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F1);
}

/*
 * Rule 10: the One Shot Term counts from the press of the trigger key, not
 * from the press of the holding key, which lies more than a One Shot Term
 * back here. A release just before its deadline is a quick release.
 */
TEST_F(TargetLayerQuickRelease, Rule10_ReleaseJustBeforeTheOneShotTermOfTheTriggerPressIsQuick) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    tap_and_let_go_after(key_pen, key_ent_num, TARGET_LAYER_ONE_SHOT_TERM - 1);

    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 7 and 10: a release at the deadline of the One Shot Term is no quick
 * release, and switches the target layer off.
 */
TEST_F(TargetLayerQuickRelease, Rule10_ReleaseJustAfterTheOneShotTermOfTheTriggerPressIsNotQuick) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap_and_let_go_after(key_pen, key_ent_num, TARGET_LAYER_ONE_SHOT_TERM);

    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 7 and 10: a key typed between the trigger press and the release of
 * the holding key leaves nothing pending, however quick the release.
 */
TEST_F(TargetLayerQuickRelease, Rule10_KeyTypedBeforeTheReleaseLeavesNothingPending) {
    TestDriver driver;

    hold(key_ent_num);
    tap(key_pen);
    expect_typed(driver, KC_F1);

    EXPECT_NO_REPORT(driver);
    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 10: a key typed on the holding layer before the trigger press does
 * not prevent the quick release.
 */
TEST_F(TargetLayerQuickRelease, Rule10_KeyTypedBeforeTheTriggerPressDoesNotCount) {
    TestDriver driver;

    hold(key_ent_num);
    expect_typed(driver, KC_2);

    EXPECT_NO_REPORT(driver);
    tap(key_pen);
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 6 and 10: with several holding keys held, the quick release is that
 * of the last of them.
 */
TEST_F(TargetLayerQuickRelease, Rule10_QuickReleaseIsThatOfTheLastHoldingKey) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_pen);

    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_PEN});

    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 8 and 10: a target layer that its trigger key cancelled does not
 * become pending.
 */
TEST_F(TargetLayerQuickRelease, Rule10_CancelledTargetLayerDoesNotBecomePending) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_pen);
    let_go(key_ent_num);

    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 9 and 10: the One Shot Term counts anew from the press of a trigger
 * key that replaces the target layer.
 */
TEST_F(TargetLayerQuickRelease, Rule9_Rule10_OneShotTermCountsAnewFromTheReplacingTriggerKey) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_fun);
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_FUN});

    tap_and_let_go_after(key_pen, key_ent_num, TARGET_LAYER_ONE_SHOT_TERM - 1);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 11: an ordinary modifier is no key typed. Pressed between the trigger
 * press and the release of the holding key, it leaves the release quick.
 */
TEST_F(TargetLayerQuickRelease, Rule11_OrdinaryModifierIsNoKeyTyped) {
    TestDriver driver;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_shift);
    let_go(key_ent_num);

    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 11, issue #40: a one-shot mod key, which the caller tells, is no key
 * typed either.
 */
TEST_F(TargetLayerQuickRelease, Rule11_Issue40_OneShotModKeyIsNoKeyTyped) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());

    hold(key_ent_num);
    tap(key_pen);
    tap(key_one_shot_ctrl);
    let_go(key_ent_num);

    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 12: a pending target layer applies to the next typed key only. It is
 * switched off once that key press has been processed.
 */
TEST_F(TargetLayerQuickRelease, Rule12_PendingTargetLayerAppliesToTheNextTypedKeyOnly) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);
    expect_layers_on({TEST_LAYER_PEN});

    EXPECT_REPORT(driver, (KC_F1));
    key_typed.press();
    run_one_scan_loop();
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    key_typed.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 12: a second key rolled over the key that used the pending target
 * layer does not come from the target layer.
 */
TEST_F(TargetLayerQuickRelease, Rule12_SecondKeyRolledOverTheFirstDoesNotComeFromTheTargetLayer) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_F1));
        EXPECT_REPORT(driver, (KC_F1, KC_C));
        EXPECT_REPORT(driver, (KC_C));
        EXPECT_EMPTY_REPORT(driver);
    }
    key_typed.press();
    run_one_scan_loop();
    key_typed_next.press();
    run_one_scan_loop();
    key_typed.release();
    run_one_scan_loop();
    key_typed_next.release();
    run_one_scan_loop();

    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 11 and 12: an ordinary modifier is no typed key, so it does not use
 * up the pending target layer. The key after it does.
 */
TEST_F(TargetLayerQuickRelease, Rule12_OrdinaryModifierDoesNotUseUpThePendingTargetLayer) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LSFT));
        EXPECT_REPORT(driver, (KC_LSFT, KC_F1));
        EXPECT_REPORT(driver, (KC_LSFT));
        EXPECT_EMPTY_REPORT(driver);
    }
    key_shift.press();
    run_one_scan_loop();
    expect_layers_on({TEST_LAYER_PEN});
    tap(key_typed);
    expect_layers_on({});
    let_go(key_shift);
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 11 and 12, issue #40: a one-shot mod key is no typed key. Tapped on
 * a pending target layer, it leaves the layer pending, and the next key
 * comes from the target layer with the modifier and uses up both.
 */
TEST_F(TargetLayerQuickRelease, Rule12_Issue40_OneShotModKeyDoesNotUseUpThePendingTargetLayer) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);

    EXPECT_NO_REPORT(driver);
    tap(key_one_shot_ctrl);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LCTL, KC_F1));
        EXPECT_REPORT(driver, (KC_F1)).Times(AnyNumber());
        EXPECT_EMPTY_REPORT(driver);
    }
    tap(key_typed);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 13: pressing the trigger key again while its target layer is pending
 * switches it off and sends nothing. No holding key is held then.
 */
TEST_F(TargetLayerQuickRelease, Rule13_TriggerKeyCancelsItsPendingTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    expect_layers_on({TEST_LAYER_PEN});

    key_pen.press();
    run_one_scan_loop();
    expect_layers_on({});

    key_pen.release();
    run_one_scan_loop();
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 14: holding a holding key while a target layer is pending ends the
 * pending target layer, and the holding layer of that key comes on as usual.
 * It need not be the holding key that was released.
 */
TEST_F(TargetLayerQuickRelease, Rule14_HoldingAHoldingKeyEndsThePendingTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    expect_layers_on({TEST_LAYER_PEN});

    hold(key_tab_sym);
    expect_layers_on({TEST_LAYER_SYM});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_1);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 14: a new gesture starts with the holding key that ended the pending
 * target layer, and its trigger key works.
 */
TEST_F(TargetLayerQuickRelease, Rule14_NewGestureStartsWithTheHoldingKeyThatEndedThePendingTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    hold(key_ent_num);
    expect_layers_on({TEST_LAYER_NUM});

    tap(key_pen);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_PEN});
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 14: a tapped holding key is a typed key. It sends its tap and uses up
 * the pending target layer.
 */
TEST_F(TargetLayerQuickRelease, Rule14_TappedHoldingKeyUsesUpThePendingTargetLayer) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);

    EXPECT_REPORT(driver, (KC_TAB));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 15: a pending target layer is still on just before its timeout ends,
 * counted from the release of the holding key.
 */
TEST_F(TargetLayerQuickRelease, Rule15_PendingTargetLayerIsStillOnJustBeforeItsTimeout) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 1);

    expect_layers_on({TEST_LAYER_PEN});
    expect_typed(driver, KC_F1);
}

/* Rule 15: a pending target layer is switched off when its timeout ends. */
TEST_F(TargetLayerQuickRelease, Rule15_PendingTargetLayerTimesOut) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);

    expect_layers_on({});
    expect_typed(driver, KC_B);
}

/*
 * Rule 15: the press of an ordinary modifier does not extend the timeout of
 * a pending target layer.
 */
TEST_F(TargetLayerQuickRelease, Rule15_OrdinaryModifierDoesNotExtendTheTimeout) {
    TestDriver driver;
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT / 2);
    tap(key_shift);
    expect_layers_on({TEST_LAYER_PEN});

    idle_for(TARGET_LAYER_PENDING_TIMEOUT - TARGET_LAYER_PENDING_TIMEOUT / 2 - 2);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 15 as amended by issue #41: the press of a one-shot mod key restarts
 * the timeout of a pending target layer. Here the one-shot mod key stays
 * down, and the layer is still on after the timeout would have ended,
 * counted from the release of the holding key.
 */
TEST_F(TargetLayerQuickRelease, Amendment41_Rule15_OneShotModKeyPressRestartsTheTimeout) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());

    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT / 2);
    key_one_shot_ctrl.press();

    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 500);
    expect_layers_on({TEST_LAYER_PEN});

    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({});

    let_go(key_one_shot_ctrl);
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 15 as amended again by issue #41: the release of a one-shot mod key
 * restarts the timeout of a pending target layer too. Here the one-shot mod
 * key is held for a while, as an ordinary modifier, and the layer is still on
 * just before the timeout ends, counted from the release.
 */
TEST_F(TargetLayerQuickRelease, Amendment41_Rule15_OneShotModKeyReleaseRestartsTheTimeout) {
    TestDriver driver;
    EXPECT_ANY_REPORT(driver).Times(AnyNumber());

    quick_gesture(key_ent_num, key_pen);
    hold(key_one_shot_ctrl);
    idle_for(500);
    let_go(key_one_shot_ctrl);

    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 1);
    expect_layers_on({TEST_LAYER_PEN});

    run_one_scan_loop();
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 15 as amended again by issue #41: a pending target layer does not end
 * before a one-shot mod of QMK's own `OSM()` key that was tapped slowly. QMK
 * handles the press of such a tap only at its release and starts
 * `ONESHOT_TIMEOUT` then, so a timeout counted from the press would end the
 * layer first, and the next key would be a key of BASE with the modifier.
 * Here the tap takes 150 ms, and the key comes 50 ms before the one-shot mod
 * ends.
 */
TEST_F(TargetLayerQuickRelease, Amendment41_Rule15_PendingTargetLayerDoesNotEndBeforeAOneShotModOfQmkThatWasTappedSlowly) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    key_one_shot_ctrl.press();
    idle_for(150);
    key_one_shot_ctrl.release();
    run_one_scan_loop();

    idle_for(ONESHOT_TIMEOUT - 50);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LCTL, KC_F1));
        EXPECT_REPORT(driver, (KC_F1)).Times(AnyNumber());
        EXPECT_EMPTY_REPORT(driver);
    }
    tap(key_typed);
    VERIFY_AND_CLEAR(driver);
    expect_layers_on({});
}

/*
 * Rule 15 as amended again by issue #41: at the moment at which the one-shot
 * mod of QMK's own `OSM()` key times out, the pending target layer has not
 * ended before it. The key is that of the target layer without the modifier,
 * never a key of BASE with it.
 */
TEST_F(TargetLayerQuickRelease, Amendment41_Rule15_PendingTargetLayerIsStillOnWhenAOneShotModOfQmkTimesOut) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    key_one_shot_ctrl.press();
    idle_for(150);
    key_one_shot_ctrl.release();
    run_one_scan_loop();

    /* One scan loop has passed since QMK made the one-shot mod pending. */
    idle_for(ONESHOT_TIMEOUT - 1);
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F1);
    expect_layers_on({});
}

/*
 * Rule 15 as amended by issue #41: a pending target layer does not time out
 * while the caller says that its timeout is suspended. The timeout starts
 * anew in the first scan loop that finds the suspension ended.
 */
TEST_F(TargetLayerQuickRelease, Amendment41_Rule15_PendingTargetLayerDoesNotTimeOutWhileSuspended) {
    TestDriver driver;

    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT / 2);

    test_pending_timeout_suspended = true;
    idle_for(2 * TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({TEST_LAYER_PEN});

    test_pending_timeout_suspended = false;
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({TEST_LAYER_PEN});

    run_one_scan_loop();
    expect_layers_on({});

    expect_typed(driver, KC_B);
}

/*
 * Amendment of issue #39: for an entry that says so, a quick release makes
 * the target layer a locked target layer, as if Layer Lock had been pressed.
 * It stays on for every key that follows.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_QuickReleaseLocksTheTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_tab_sym, key_lck);
    expect_layers_on({TEST_LAYER_LCK});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_LEFT);
    expect_typed(driver, KC_LEFT);
    expect_layers_on({TEST_LAYER_LCK});
}

/*
 * Amendment of issue #39, under the condition of rule 10: a release at the
 * deadline of the One Shot Term does not lock the target layer.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_ReleaseJustAfterTheOneShotTermDoesNotLock) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap_and_let_go_after(key_lck, key_tab_sym, TARGET_LAYER_ONE_SHOT_TERM);

    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);
}

/*
 * Amendment of issue #39: a release just before the deadline of the One Shot
 * Term locks the target layer.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_ReleaseJustBeforeTheOneShotTermLocks) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap_and_let_go_after(key_lck, key_tab_sym, TARGET_LAYER_ONE_SHOT_TERM - 1);

    expect_layers_on({TEST_LAYER_LCK});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 11, issue #40: a mouse key is a typed key. Pressed on the target
 * layer before the release of the holding key, it prevents the lock.
 */
TEST_F(TargetLayerQuickRelease, Rule11_Issue40_MouseKeyIsAKeyTyped) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_lck);
    tap(key_typed_next);
    let_go(key_tab_sym);

    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 1 and the amendment of issue #39: with several holding keys held, the
 * quick release does what the entry of the holding key released last says.
 * The entry of `TAB_SYM` locks LCK; that of `ENT_NUM` does nothing.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_QuickReleaseDoesWhatTheEntryOfTheLastHoldingKeySays) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_lck);
    let_go(key_ent_num);
    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_LCK});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_LCK));

    tap(key_lck);
    expect_layers_on({});

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_lck);
    let_go(key_tab_sym);
    let_go(key_ent_num);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 18 as amended by issue #39: the trigger key pressed while its target
 * layer is locked unlocks it and switches it off. No holding key is held
 * then, and nothing is sent.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_Rule18_TriggerKeyUnlocksItsLockedTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_tab_sym, key_lck);
    expect_layers_on({TEST_LAYER_LCK});

    key_lck.press();
    run_one_scan_loop();
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));

    key_lck.release();
    run_one_scan_loop();
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 18 as amended by issue #39: with a holding key held again above the
 * locked target layer, the trigger key unlocks the target layer and leaves
 * the user on the holding layer. Pressed once more, it switches the target
 * layer on again.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_Rule18_TriggerKeyUnlocksWhileAHoldingKeyIsHeld) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_tab_sym, key_lck);
    hold(key_tab_sym);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_LCK});

    tap(key_lck);
    expect_layers_on({TEST_LAYER_SYM});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_1);

    EXPECT_NO_REPORT(driver);
    tap(key_lck);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_LCK});
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Amendment of issue #39: the Layer Lock key unlocks a locked target layer,
 * and the gesture works again afterwards.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_LayerLockKeyUnlocksALockedTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_tab_sym, key_lck);
    tap(key_layer_lock);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_LCK));

    hold(key_tab_sym);
    tap(key_lck);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_LCK});
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Amendment of issue #39: nothing but its trigger key and Layer Lock leaves
 * a locked target layer. Not typed keys, the timeout of a pending target
 * layer, a holding key held, tapped or released, or the gesture of another
 * target layer, pending or not.
 */
TEST_F(TargetLayerQuickRelease, Amendment39_NothingElseLeavesALockedTargetLayer) {
    TestDriver driver;

    quick_gesture(key_tab_sym, key_lck);

    expect_typed(driver, KC_LEFT);
    idle_for(2 * TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({TEST_LAYER_LCK});

    EXPECT_NO_REPORT(driver);
    hold(key_tab_sym);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_LCK});
    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_LCK});
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_TAB));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_tab_sym);
    expect_layers_on({TEST_LAYER_LCK});
    VERIFY_AND_CLEAR(driver);

    /* The gesture of FUN, whose quick release does nothing. */
    EXPECT_NO_REPORT(driver);
    hold(key_ent_num);
    tap(key_fun);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_FUN, TEST_LAYER_LCK});
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_LCK});

    /* The gesture of PEN, left pending and then cancelled by its trigger key. */
    quick_gesture(key_ent_num, key_pen);
    expect_layers_on({TEST_LAYER_PEN, TEST_LAYER_LCK});
    tap(key_pen);
    expect_layers_on({TEST_LAYER_LCK});

    /* The gesture of PEN, left pending and timed out. */
    quick_gesture(key_ent_num, key_pen);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_layers_on({TEST_LAYER_LCK});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_LCK));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_LEFT);
}

/*
 * Rule 17: Layer Lock pressed on a target layer while the holding key is
 * held locks it. Releasing the holding key then leaves the target layer on,
 * and the holding layer goes off. The Layer Lock key unlocks it.
 */
TEST_F(TargetLayerQuickRelease, Rule17_LayerLockOnAHeldTargetLayerLocksIt) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_layer_lock);
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_PEN));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F1);
    expect_typed(driver, KC_F1);

    EXPECT_NO_REPORT(driver);
    tap(key_layer_lock);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 17: the same for an entry whose quick release does nothing, and with
 * a quick release of the holding key.
 */
TEST_F(TargetLayerQuickRelease, Rule17_LayerLockLocksATargetLayerWhoseQuickReleaseDoesNothing) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_fun);
    tap(key_layer_lock);
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_FUN});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_FUN));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F12);
    expect_typed(driver, KC_F12);
}

/*
 * Rules 10 and 17: a target layer locked by Layer Lock does not become
 * pending by a quick release of the holding key. It stays on after the next
 * typed key and after the timeout of a pending target layer.
 */
TEST_F(TargetLayerQuickRelease, Rule17_QuickReleaseLeavesATargetLayerLockedByLayerLockLocked) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_layer_lock);
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F1);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_typed(driver, KC_F1);
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_PEN));
}

/*
 * Rule 17: Layer Lock pressed again while the holding key is still held
 * unlocks the target layer, and the user is back on the holding layer.
 */
TEST_F(TargetLayerQuickRelease, Rule17_LayerLockPressedAgainUnlocksWhileTheHoldingKeyIsHeld) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_layer_lock);
    tap(key_layer_lock);
    expect_layers_on({TEST_LAYER_NUM});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_2);

    EXPECT_NO_REPORT(driver);
    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 17: Layer Lock pressed while a target layer is pending locks it and
 * ends the pending state. Neither the next typed key nor the timeout
 * switches it off.
 */
TEST_F(TargetLayerQuickRelease, Rule17_LayerLockOnAPendingTargetLayerLocksIt) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    quick_gesture(key_ent_num, key_pen);
    tap(key_layer_lock);
    expect_layers_on({TEST_LAYER_PEN});
    EXPECT_TRUE(is_layer_locked(TEST_LAYER_PEN));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_F1);
    expect_typed(driver, KC_F1);
    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_typed(driver, KC_F1);

    EXPECT_NO_REPORT(driver);
    tap(key_layer_lock);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 17 and rule 18 as amended by issue #39: a target layer that Layer
 * Lock locked while it was on through the trigger key is a locked target
 * layer, so its trigger key unlocks it.
 */
TEST_F(TargetLayerQuickRelease, Rule17_Amendment39_Rule18_TriggerKeyUnlocksATargetLayerLockedByLayerLock) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_pen);
    tap(key_layer_lock);
    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_PEN});

    tap(key_pen);
    expect_layers_on({});
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_PEN));
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Issue #40: a trigger key does not use up a pending one-shot mod of QMK's
 * own one-shot mod keys. `OSM(MOD_LCTL)`, then the trigger key, then a key:
 * that key comes from the target layer with Ctrl.
 */
TEST_F(TargetLayerQuickRelease, Issue40_TriggerKeyDoesNotUseUpAPendingOneShotModOfQmk) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_one_shot_ctrl);
    tap(key_lck);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_LCK});
    VERIFY_AND_CLEAR(driver);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
        EXPECT_REPORT(driver, (KC_LEFT)).Times(AnyNumber());
        EXPECT_EMPTY_REPORT(driver);
    }
    tap(key_typed);
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    VERIFY_AND_CLEAR(driver);
}

/*
 * Issue #40: the same with the whole gesture after the one-shot mod key, and
 * a quick release that locks the target layer. Neither the trigger key nor
 * the unlocking trigger key uses up the pending one-shot mod.
 */
TEST_F(TargetLayerQuickRelease, Issue40_TriggerGestureDoesNotUseUpAPendingOneShotModOfQmk) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    tap(key_one_shot_ctrl);
    quick_gesture(key_tab_sym, key_lck);
    expect_layers_on({TEST_LAYER_LCK});
    tap(key_lck);
    expect_layers_on({});
    quick_gesture(key_tab_sym, key_lck);
    expect_layers_on({TEST_LAYER_LCK});
    VERIFY_AND_CLEAR(driver);

    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_LCTL, KC_LEFT));
        EXPECT_REPORT(driver, (KC_LEFT)).Times(AnyNumber());
        EXPECT_EMPTY_REPORT(driver);
    }
    tap(key_typed);
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_LEFT);
}
