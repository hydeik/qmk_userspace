/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * The rules of the layer switch while a holding key is held. "Rule N" refers
 * to the numbered rules of the resolution of issue #38, "How should the layer
 * switch behave?"; each test carries the number of its rule in its name.
 *
 * This file must not branch on the keymap family or the firmware source.
 */

#include <initializer_list>

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "target_layer.h"
#include "test_glue.h"
}

using testing::_;
using testing::InSequence;

/*
 * The keymap of the tests, one row of eight keys. An empty cell is a
 * transparent key.
 *
 * | Column | 0       | 1       | 2       | 3    | 4      | 5       | 6       | 7       |
 * | ------ | ------- | ------- | ------- | ---- | ------ | ------- | ------- | ------- |
 * | BASE   | TAB_SYM | ENT_NUM | A       | B    | QK_REP | C       | MO(NAV) | TG(NAV) |
 * | SYM    |         |         | MO(NAV) | 1    |        | MO(FUN) |         |         |
 * | NUM    |         |         | MO(NAV) | 2    |        | MO(FUN) |         |         |
 * | NAV    |         |         | MO(NAV) | LEFT |        | MO(FUN) |         |         |
 * | FUN    |         |         |         | F1   |        | MO(FUN) |         |         |
 *
 * `key_nav` and `key_fun` are the positions of the trigger keys of NAV and
 * FUN on the holding layers. `key_nav_on_base` is a trigger key of NAV that
 * lies on the base layer, with a transparent key above it on every layer.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class TargetLayer : public TestFixture {
  public:
    static constexpr uint8_t COLUMNS = 8;

    KeymapKey key_tab_sym{TEST_LAYER_BASE, 0, 0, TAB_SYM};
    KeymapKey key_ent_num{TEST_LAYER_BASE, 1, 0, ENT_NUM};
    KeymapKey key_nav{TEST_LAYER_BASE, 2, 0, KC_A};
    KeymapKey key_typed{TEST_LAYER_BASE, 3, 0, KC_B};
    KeymapKey key_repeat{TEST_LAYER_BASE, 4, 0, QK_REP};
    KeymapKey key_fun{TEST_LAYER_BASE, 5, 0, KC_C};
    KeymapKey key_nav_on_base{TEST_LAYER_BASE, 6, 0, MO(TEST_LAYER_NAV)};
    KeymapKey key_toggle_nav{TEST_LAYER_BASE, 7, 0, TG(TEST_LAYER_NAV)};

    TargetLayer() {
        layer_lock_all_off();
        layer_clear();
        target_layer_reset();

        for (const KeymapKey& key : {key_tab_sym, key_ent_num, key_nav, key_typed, key_repeat, key_fun, key_nav_on_base, key_toggle_nav}) {
            add_key(key);
        }
        for (const KeymapKey& key : {
                 KeymapKey{TEST_LAYER_SYM, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_SYM, 3, 0, KC_1},
                 KeymapKey{TEST_LAYER_SYM, 5, 0, MO(TEST_LAYER_FUN)},
                 KeymapKey{TEST_LAYER_NUM, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_NUM, 3, 0, KC_2},
                 KeymapKey{TEST_LAYER_NUM, 5, 0, MO(TEST_LAYER_FUN)},
                 KeymapKey{TEST_LAYER_NAV, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_NAV, 3, 0, KC_LEFT},
                 KeymapKey{TEST_LAYER_NAV, 5, 0, MO(TEST_LAYER_FUN)},
                 KeymapKey{TEST_LAYER_FUN, 3, 0, KC_F1},
                 KeymapKey{TEST_LAYER_FUN, 5, 0, MO(TEST_LAYER_FUN)},
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
    void hold(KeymapKey& key) {
        key.press();
        idle_for(TAPPING_TERM + 1);
    }

    /* Release `key`. */
    void let_go(KeymapKey& key) {
        key.release();
        run_one_scan_loop();
    }

    /* Press `key`, and release it 1 ms later. */
    void tap(KeymapKey& key) {
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

    /* Tap `key_typed` and expect that it sends `code`. */
    void expect_typed(TestDriver& driver, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key_typed);
        VERIFY_AND_CLEAR(driver);
    }
};

/*
 * Rule 3: a trigger key pressed with no holding key held does nothing. It is
 * no momentary layer key either.
 */
TEST_F(TargetLayer, Rule3_TriggerKeyDoesNothingWithNoHoldingKeyHeld) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    key_nav_on_base.press();
    run_one_scan_loop();
    expect_layers_on({});

    key_nav_on_base.release();
    run_one_scan_loop();
    expect_layers_on({});

    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 5: a trigger key pressed while its holding key is held switches its
 * target layer on, and the next key comes from the target layer.
 */
TEST_F(TargetLayer, Rule5_TriggerKeySwitchesItsTargetLayerOnWhileItsHoldingKeyIsHeld) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    expect_layers_on({TEST_LAYER_SYM});

    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 8: pressing the trigger key again while its target layer is on
 * switches it off, and the user is back on the holding layer. Pressing it
 * once more switches the target layer on again.
 */
TEST_F(TargetLayer, Rule8_TriggerKeyPressedAgainCancelsItsTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_1);

    EXPECT_NO_REPORT(driver);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 8: the position of the trigger key on its target layer may hold a
 * transparent key instead of the trigger key itself.
 */
TEST_F(TargetLayer, Rule8_TriggerKeyBeneathATransparentKeyCancelsItsTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav_on_base);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    tap(key_nav_on_base);
    expect_layers_on({TEST_LAYER_SYM});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 9: the trigger key of another target layer, of which a held key is a
 * holding key, switches the first target layer off and its own on. It may
 * sit on the first target layer.
 */
TEST_F(TargetLayer, Rule9_AnotherTriggerKeyReplacesTheTargetLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_NAV});

    tap(key_fun);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_FUN});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_F1);

    EXPECT_NO_REPORT(driver);
    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 8 and 9: after a replacement, the second trigger key cancels its own
 * target layer and the user is back on the holding layer, not on the first
 * target layer.
 */
TEST_F(TargetLayer, Rule9_CancellingAfterAReplacementReturnsToTheHoldingLayer) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_ent_num);
    tap(key_nav);
    tap(key_fun);
    tap(key_fun);
    expect_layers_on({TEST_LAYER_NUM});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_2);

    EXPECT_NO_REPORT(driver);
    let_go(key_ent_num);
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 3 and 9: another trigger key does not replace the target layer if no
 * held key is one of its holding keys.
 */
TEST_F(TargetLayer, Rule9_AnotherTriggerKeyDoesNothingIfNoHeldKeyIsItsHoldingKey) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    tap(key_fun);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rules 1 and 9: after a replacement, only the held holding keys of the
 * second trigger key keep its target layer on.
 */
TEST_F(TargetLayer, Rule9_OnlyTheHoldingKeysOfTheReplacingTriggerKeyKeepItsTargetLayerOn) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_nav);
    tap(key_fun);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NUM, TEST_LAYER_FUN});

    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_SYM});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 16: a target layer that another key switched on is not switched off
 * by the release of a holding key.
 * Rule 18: and its trigger key does nothing meanwhile.
 */
TEST_F(TargetLayer, Rule16_Rule18_TargetLayerOnForAnotherReasonIsLeftAlone) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    tap(key_toggle_nav);
    expect_layers_on({TEST_LAYER_NAV});

    hold(key_tab_sym);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 16: a replacement does not switch off a target layer that another key
 * switched on.
 */
TEST_F(TargetLayer, Rule16_ReplacementLeavesATargetLayerOnForAnotherReasonOn) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    tap(key_toggle_nav);
    hold(key_ent_num);
    tap(key_fun);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_NAV, TEST_LAYER_FUN});

    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 18, as amended by the hydeik_vial transitions ticket: while its
 * target layer is locked by Layer Lock, a trigger key unlocks it and
 * switches it off, also when the layer was not on through this module when
 * it was locked. The press does not switch the target layer on again.
 */
TEST_F(TargetLayer, Rule18_Amendment39_TriggerKeyUnlocksATargetLayerLockedBehindTheModule) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    layer_lock_on(TEST_LAYER_NAV);
    hold(key_tab_sym);
    tap(key_nav);
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_NAV));
    expect_layers_on({TEST_LAYER_SYM});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 18, as amended: no holding key has to be held to leave a locked
 * target layer.
 */
TEST_F(TargetLayer, Rule18_Amendment39_TriggerKeyAloneUnlocksATargetLayerLockedBehindTheModule) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    layer_lock_on(TEST_LAYER_NAV);
    tap(key_nav);
    EXPECT_FALSE(is_layer_locked(TEST_LAYER_NAV));
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 1: a target layer may have several holding keys, and each of them
 * makes its trigger key work.
 */
TEST_F(TargetLayer, Rule1_EachHoldingKeyOfATargetLayerMakesItsTriggerKeyWork) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});
    let_go(key_tab_sym);
    expect_layers_on({});

    hold(key_ent_num);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_NAV});
    let_go(key_ent_num);
    expect_layers_on({});

    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 1: the holding keys that keep a target layer on are those held when
 * the trigger key is pressed. A holding key that is held only afterwards does
 * not keep the target layer on.
 */
TEST_F(TargetLayer, Rule1_HoldingKeyHeldAfterTheTriggerPressDoesNotKeepTheTargetLayerOn) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    hold(key_ent_num);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NUM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_NUM});

    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 3: a trigger key does nothing while its holding layer is on only
 * through Layer Lock, with no holding key held.
 */
TEST_F(TargetLayer, Rule3_TriggerKeyDoesNothingWhenItsHoldingLayerIsOnOnlyThroughLayerLock) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    layer_lock_on(TEST_LAYER_SYM);
    expect_layers_on({TEST_LAYER_SYM});

    key_nav.press();
    run_one_scan_loop();
    expect_layers_on({TEST_LAYER_SYM});

    key_nav.release();
    run_one_scan_loop();
    expect_layers_on({TEST_LAYER_SYM});

    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 3: a trigger key does nothing while the only held key is the holding
 * key of another target layer, not one of its own.
 */
TEST_F(TargetLayer, Rule3_TriggerKeyDoesNothingWhenTheHeldKeyIsNotItsHoldingKey) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    key_fun.press();
    run_one_scan_loop();
    expect_layers_on({TEST_LAYER_SYM});

    key_fun.release();
    run_one_scan_loop();
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 4: a holding key that QMK resolves as a tap does not make the trigger
 * key work. A quick roll over the trigger key's position sends the tap of
 * the holding key and the key beneath the trigger key.
 */
TEST_F(TargetLayer, Rule4_HoldingKeyResolvedAsATapSendsTheKeyBeneathTheTriggerKey) {
    TestDriver driver;
    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_TAB));
        EXPECT_REPORT(driver, (KC_TAB, KC_A));
        EXPECT_REPORT(driver, (KC_TAB));
        EXPECT_EMPTY_REPORT(driver);
    }

    key_tab_sym.press();
    run_one_scan_loop();
    tap(key_nav);
    let_go(key_tab_sym);

    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 4: a holding key that QMK resolves as a tap does not make a trigger
 * key work, however long it is kept down. Pressed again right after a tap,
 * a layer-tap key is a tap that is kept down, for the host to repeat.
 */
TEST_F(TargetLayer, Rule4_HoldingKeyKeptDownAsATapDoesNotMakeTheTriggerKeyWork) {
    TestDriver driver;
    {
        InSequence s;
        EXPECT_REPORT(driver, (KC_TAB));
        EXPECT_EMPTY_REPORT(driver);
        EXPECT_REPORT(driver, (KC_TAB));
    }

    tap(key_tab_sym);
    key_tab_sym.press();
    idle_for(TAPPING_TERM + 1);
    expect_layers_on({});

    tap(key_nav_on_base);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    EXPECT_EMPTY_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 4: a trigger key tapped within the tapping term of its holding key
 * works once QMK resolves the holding key as a hold.
 */
TEST_F(TargetLayer, Rule4_TriggerKeyWorksOnceQmkResolvesTheHoldingKeyAsAHold) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    key_tab_sym.press();
    run_one_scan_loop();
    tap(key_nav);
    expect_layers_on({});

    idle_for(TAPPING_TERM);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 5: keeping the trigger key down changes nothing. Its target layer
 * goes off with the holding key, and its release does nothing.
 */
TEST_F(TargetLayer, Rule5_KeepingTheTriggerKeyDownChangesNothing) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    key_nav.press();
    idle_for(500);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({});

    key_nav.release();
    run_one_scan_loop();
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 6: with several holding keys of the trigger key held, the target
 * layer stays on until the last of them is released.
 */
TEST_F(TargetLayer, Rule6_TargetLayerStaysOnUntilTheLastHoldingKeyIsReleased) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NUM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({TEST_LAYER_NUM, TEST_LAYER_NAV});
    VERIFY_AND_CLEAR(driver);
    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_ent_num);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/* Rule 6: the same, with the holding keys released in the other order. */
TEST_F(TargetLayer, Rule6_TargetLayerStaysOnUntilTheLastHoldingKeyIsReleasedInEitherOrder) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    hold(key_ent_num);
    tap(key_nav);

    let_go(key_ent_num);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 7: releasing the last holding key after a key was typed switches the
 * target layer off, and QMK switches the holding layer off.
 */
TEST_F(TargetLayer, Rule7_ReleasingTheHoldingKeyAfterAKeyTypedSwitchesTheTargetLayerOff) {
    TestDriver driver;

    hold(key_tab_sym);
    tap(key_nav);
    expect_typed(driver, KC_LEFT);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 7: releasing the last holding key long after the trigger press, with
 * no key typed, switches the target layer off too.
 */
TEST_F(TargetLayer, Rule7_ReleasingTheHoldingKeyLongAfterTheTriggerPressSwitchesTheTargetLayerOff) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    idle_for(1000);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);
}

/*
 * Rule 2: an entry can turn the pending target layer off. Its quick release
 * does nothing then: releasing the holding key right after the trigger key,
 * with no key typed, switches the target layer off as any other release does.
 */
TEST_F(TargetLayer, Rule2_QuickReleaseThatDoesNothingSwitchesTheTargetLayerOff) {
    TestDriver driver;
    EXPECT_NO_REPORT(driver);

    hold(key_tab_sym);
    tap(key_nav);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    expect_typed(driver, KC_B);
}

/*
 * Rule 19: Repeat Key never remembers a trigger key, nor a holding key that
 * is held: it repeats the key typed before them.
 */
TEST_F(TargetLayer, Rule19_RepeatKeyNeverRemembersATriggerKey) {
    TestDriver driver;

    expect_typed(driver, KC_B);

    hold(key_tab_sym);
    tap(key_nav);
    expect_layers_on({TEST_LAYER_SYM, TEST_LAYER_NAV});

    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_repeat);
    VERIFY_AND_CLEAR(driver);

    EXPECT_NO_REPORT(driver);
    let_go(key_tab_sym);
    expect_layers_on({});
    VERIFY_AND_CLEAR(driver);

    EXPECT_REPORT(driver, (KC_B));
    EXPECT_EMPTY_REPORT(driver);
    tap(key_repeat);
    VERIFY_AND_CLEAR(driver);
}
