/*
 * Copyright Hidekazu Ikeno <@hydeik>
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * What target_layer answers about its state: the state of a target layer and
 * whether its trigger key works now, by the resolution of issue #84, "What do
 * smart_mod and target_layer answer about their state, and how is the
 * indicator module cut?". What the layers do is the matter of the other tests
 * of this directory, so every keyboard report is allowed here.
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

/*
 * The keymap of the tests, one row of eight keys. An empty cell is a
 * transparent key.
 *
 * | Column | 0       | 1       | 2       | 3       | 4       | 5       | 6  | 7       |
 * | ------ | ------- | ------- | ------- | ------- | ------- | ------- | -- | ------- |
 * | BASE   | TAB_SYM | ENT_NUM | A       | B       | C       | QK_LLCK | D  | TG(PEN) |
 * | SYM    |         |         | MO(NAV) | MO(PEN) | MO(LCK) |         | 1  |         |
 * | NUM    |         |         | MO(NAV) | MO(PEN) | MO(LCK) |         | 2  |         |
 * | NAV    |         |         | MO(NAV) |         |         |         |    |         |
 * | PEN    |         |         |         | MO(PEN) |         |         | F1 |         |
 * | LCK    |         |         |         |         | MO(LCK) |         |    |         |
 *
 * `key_nav`, `key_pen` and `key_lck` are the positions of the trigger keys.
 * By the table of entries of test_glue.h, both holding keys are holding keys
 * of NAV, PEN and LCK, and only `ENT_NUM` is one of FUN. A quick release
 * leaves PEN pending, locks LCK if the holding key is `TAB_SYM`, and does
 * nothing to NAV.
 *
 * A scan loop handles the key events at the current time and then advances
 * the time by 1 ms. `press()` / `release()` take effect in the next scan loop.
 */
class TargetLayerStateQueries : public TestFixture {
   public:
    static constexpr uint8_t COLUMNS = 8;

    KeymapKey key_tab_sym{TEST_LAYER_BASE, 0, 0, TAB_SYM};
    KeymapKey key_ent_num{TEST_LAYER_BASE, 1, 0, ENT_NUM};
    KeymapKey key_nav{TEST_LAYER_BASE, 2, 0, KC_A};
    KeymapKey key_pen{TEST_LAYER_BASE, 3, 0, KC_B};
    KeymapKey key_lck{TEST_LAYER_BASE, 4, 0, KC_C};
    KeymapKey key_layer_lock{TEST_LAYER_BASE, 5, 0, QK_LLCK};
    KeymapKey key_typed{TEST_LAYER_BASE, 6, 0, KC_D};
    KeymapKey key_toggle_pen{TEST_LAYER_BASE, 7, 0, TG(TEST_LAYER_PEN)};

    TestDriver driver;

    TargetLayerStateQueries() {
        layer_lock_all_off();
        layer_clear();
        target_layer_reset();
        test_pending_timeout_suspended = false;

        for (const KeymapKey &key : {key_tab_sym, key_ent_num, key_nav, key_pen, key_lck, key_layer_lock, key_typed, key_toggle_pen}) {
            add_key(key);
        }
        for (const KeymapKey &key : {
                 KeymapKey{TEST_LAYER_SYM, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_SYM, 3, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_SYM, 4, 0, MO(TEST_LAYER_LCK)},
                 KeymapKey{TEST_LAYER_SYM, 6, 0, KC_1},
                 KeymapKey{TEST_LAYER_NUM, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_NUM, 3, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_NUM, 4, 0, MO(TEST_LAYER_LCK)},
                 KeymapKey{TEST_LAYER_NUM, 6, 0, KC_2},
                 KeymapKey{TEST_LAYER_NAV, 2, 0, MO(TEST_LAYER_NAV)},
                 KeymapKey{TEST_LAYER_PEN, 3, 0, MO(TEST_LAYER_PEN)},
                 KeymapKey{TEST_LAYER_PEN, 6, 0, KC_F1},
                 KeymapKey{TEST_LAYER_LCK, 4, 0, MO(TEST_LAYER_LCK)},
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
        EXPECT_CALL(driver, send_keyboard_mock(_)).Times(AnyNumber());
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

    /* The quick gesture: hold `holding`, tap `trigger`, release `holding`. */
    void quick_gesture(KeymapKey &holding, KeymapKey &trigger) {
        hold(holding);
        tap(trigger);
        let_go(holding);
    }

    /*
     * Expect that the target layers `on`, `pending` and `locked` are in that
     * state, each -1 if there is none, and that every other layer is off.
     */
    void expect_states(int on, int pending, int locked) {
        for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
            target_layer_state_t expected = TARGET_LAYER_STATE_OFF;
            if (layer == on) {
                expected = TARGET_LAYER_STATE_ON;
            } else if (layer == pending) {
                expected = TARGET_LAYER_STATE_PENDING;
            } else if (layer == locked) {
                expected = TARGET_LAYER_STATE_LOCKED;
            }
            EXPECT_EQ(target_layer_state_of(layer), expected) << "layer " << +layer;
        }
    }

    static constexpr int NONE = -1;
};

/****************************************************************************
 * The state of a target layer
 ****************************************************************************/

/*
 * A target layer is on from the press of its trigger key, while a holding
 * key keeps it on, and off again once that holding key is released.
 */
TEST_F(TargetLayerStateQueries, TargetLayerKeptOnByAHoldingKeyIsOn) {
    expect_states(NONE, NONE, NONE);

    hold(key_ent_num);
    expect_states(NONE, NONE, NONE);

    tap(key_pen);
    expect_states(TEST_LAYER_PEN, NONE, NONE);

    idle_for(TARGET_LAYER_ONE_SHOT_TERM);
    let_go(key_ent_num);
    expect_states(NONE, NONE, NONE);
}

/* The trigger key of another target layer replaces the one that is on. */
TEST_F(TargetLayerStateQueries, ReplacedTargetLayerIsOff) {
    hold(key_ent_num);
    tap(key_nav);
    expect_states(TEST_LAYER_NAV, NONE, NONE);

    tap(key_pen);
    expect_states(TEST_LAYER_PEN, NONE, NONE);
}

/*
 * A pending target layer is pending until the next typed key has used it.
 */
TEST_F(TargetLayerStateQueries, PendingTargetLayerIsPendingUntilAKeyIsTyped) {
    quick_gesture(key_ent_num, key_pen);
    expect_states(NONE, TEST_LAYER_PEN, NONE);

    tap(key_typed);
    expect_states(NONE, NONE, NONE);
}

/* A pending target layer is pending until its timeout ends, and off after. */
TEST_F(TargetLayerStateQueries, PendingTargetLayerIsOffAfterItsTimeout) {
    quick_gesture(key_ent_num, key_pen);

    idle_for(TARGET_LAYER_PENDING_TIMEOUT - 1);
    expect_states(NONE, TEST_LAYER_PEN, NONE);

    run_one_scan_loop();
    expect_states(NONE, NONE, NONE);
}

/*
 * A target layer that a quick release locked is locked until its trigger key
 * unlocks it.
 */
TEST_F(TargetLayerStateQueries, TargetLayerLockedByAQuickReleaseIsLocked) {
    quick_gesture(key_tab_sym, key_lck);
    expect_states(NONE, NONE, TEST_LAYER_LCK);
    ASSERT_TRUE(is_layer_locked(TEST_LAYER_LCK));

    tap(key_typed);
    expect_states(NONE, NONE, TEST_LAYER_LCK);

    tap(key_lck);
    expect_states(NONE, NONE, NONE);
}

/*
 * Edge: Layer Lock pressed on a target layer that a holding key keeps on
 * makes it locked, at once and after the release of the holding key. The
 * Layer Lock key unlocks it.
 */
TEST_F(TargetLayerStateQueries, LayerLockPressedOnATargetLayerThatIsOnMakesItLocked) {
    hold(key_ent_num);
    tap(key_pen);
    expect_states(TEST_LAYER_PEN, NONE, NONE);

    tap(key_layer_lock);
    expect_states(NONE, NONE, TEST_LAYER_PEN);

    let_go(key_ent_num);
    expect_states(NONE, NONE, TEST_LAYER_PEN);

    tap(key_layer_lock);
    expect_states(NONE, NONE, NONE);
}

/* Edge: Layer Lock pressed on a pending target layer makes it locked. */
TEST_F(TargetLayerStateQueries, LayerLockPressedOnAPendingTargetLayerMakesItLocked) {
    quick_gesture(key_ent_num, key_pen);
    expect_states(NONE, TEST_LAYER_PEN, NONE);

    tap(key_layer_lock);
    expect_states(NONE, NONE, TEST_LAYER_PEN);

    idle_for(TARGET_LAYER_PENDING_TIMEOUT);
    expect_states(NONE, NONE, TEST_LAYER_PEN);
}

/*
 * Edge: a target layer that is on for any other reason than this module or
 * Layer Lock is off.
 */
TEST_F(TargetLayerStateQueries, TargetLayerOnForAnotherReasonIsOff) {
    tap(key_toggle_pen);
    ASSERT_TRUE(layer_state_is(TEST_LAYER_PEN));
    expect_states(NONE, NONE, NONE);

    /* Its trigger key leaves it alone, and it stays off to the query. */
    hold(key_ent_num);
    tap(key_pen);
    ASSERT_TRUE(layer_state_is(TEST_LAYER_PEN));
    expect_states(NONE, NONE, NONE);
}

/*
 * Edge: a target layer that Layer Lock holds is locked, whichever way it
 * came on.
 */
TEST_F(TargetLayerStateQueries, TargetLayerThatLayerLockHoldsIsLockedWhicheverWayItCameOn) {
    tap(key_toggle_pen);
    expect_states(NONE, NONE, NONE);

    tap(key_layer_lock);
    ASSERT_TRUE(is_layer_locked(TEST_LAYER_PEN));
    expect_states(NONE, NONE, TEST_LAYER_PEN);
}

/*
 * Edge: a layer that is no target layer is off, while it is on and also while
 * Layer Lock holds it.
 */
TEST_F(TargetLayerStateQueries, LayerThatIsNoTargetLayerIsOffAlsoWhileLayerLockHoldsIt) {
    hold(key_tab_sym);
    ASSERT_TRUE(layer_state_is(TEST_LAYER_SYM));
    expect_states(NONE, NONE, NONE);

    tap(key_layer_lock);
    let_go(key_tab_sym);
    ASSERT_TRUE(is_layer_locked(TEST_LAYER_SYM));
    expect_states(NONE, NONE, NONE);
}

/*
 * A locked target layer stays locked while the trigger key of another target
 * layer switches that one on: each layer has its own answer.
 */
TEST_F(TargetLayerStateQueries, LockedTargetLayerAndTheOneThatIsOnAreAnsweredApart) {
    quick_gesture(key_tab_sym, key_lck);

    hold(key_ent_num);
    tap(key_pen);
    expect_states(TEST_LAYER_PEN, NONE, TEST_LAYER_LCK);

    let_go(key_ent_num);
    expect_states(NONE, TEST_LAYER_PEN, TEST_LAYER_LCK);
}

/****************************************************************************
 * Whether the trigger key of a target layer works
 ****************************************************************************/

/*
 * The trigger key of a target layer works exactly while a holding key of
 * that target layer is held: not for a target layer that the held key is no
 * holding key of, and not for a layer that is no target layer.
 */
TEST_F(TargetLayerStateQueries, TriggerKeyWorksExactlyWhileAHoldingKeyOfItsTargetLayerIsHeld) {
    for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
        EXPECT_FALSE(target_layer_is_trigger_key_working(layer)) << "layer " << +layer;
    }

    hold(key_tab_sym);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_PEN));
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_LCK));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_SYM));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_BASE));

    let_go(key_tab_sym);
    for (uint8_t layer = 0; layer < TEST_LAYER_COUNT; layer++) {
        EXPECT_FALSE(target_layer_is_trigger_key_working(layer)) << "layer " << +layer;
    }
}

/*
 * With two holding keys of one target layer held, its trigger key works
 * until the last of them is released, in either order.
 */
TEST_F(TargetLayerStateQueries, TriggerKeyWorksUntilTheLastOfTwoHoldingKeysIsReleased) {
    hold(key_tab_sym);
    hold(key_ent_num);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));

    let_go(key_tab_sym);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));

    let_go(key_ent_num);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));

    hold(key_tab_sym);
    hold(key_ent_num);
    let_go(key_ent_num);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_FUN));

    let_go(key_tab_sym);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
}

/*
 * A holding key counts as held only once QMK has made its press a hold; a
 * tapped holding key makes no trigger key work.
 */
TEST_F(TargetLayerStateQueries, TriggerKeyDoesNotWorkBeforeTheHoldingKeyIsAHold) {
    key_tab_sym.press();
    run_one_scan_loop();
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));

    idle_for(TAPPING_TERM);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));

    let_go(key_tab_sym);
    tap(key_tab_sym);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_NAV));
}

/*
 * Whether the trigger key works is apart from the state of its target layer:
 * it works while the target layer is on, and not while the target layer is
 * pending or locked with no holding key held.
 */
TEST_F(TargetLayerStateQueries, TriggerKeyWorkingIsApartFromTheStateOfItsTargetLayer) {
    hold(key_ent_num);
    tap(key_pen);
    EXPECT_TRUE(target_layer_is_trigger_key_working(TEST_LAYER_PEN));

    let_go(key_ent_num);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_PEN), TARGET_LAYER_STATE_PENDING);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_PEN));

    quick_gesture(key_tab_sym, key_lck);
    ASSERT_EQ(target_layer_state_of(TEST_LAYER_LCK), TARGET_LAYER_STATE_LOCKED);
    EXPECT_FALSE(target_layer_is_trigger_key_working(TEST_LAYER_LCK));
}
