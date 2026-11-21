/* Throwaway probe for issue #37. See split_probe.h. */

#include "keyboard_report_util.hpp"
#include "keycode.h"
#include "test_common.hpp"
#include "test_fixture.hpp"
#include "test_keymap_key.hpp"

extern "C" {
#include "probe_glue.h"
#include "split_probe.h"
}

using testing::_;
using testing::AnyNumber;

/*
 *          col 0     1        2     3     9
 *   base   TAB_SYM   a        b     c     ENT_NUM
 *   sym    .         TRG_NAV  1     LLCK  .
 *   num    .         TRG_FUN  2     LLCK  .
 *   nav    .         .        Left  LLCK  .
 *   fun    .         .        F1    LLCK  .
 *   tri    .         .        F2    LLCK  .      (PROBE_TRI_LAYER only)
 *
 * For Chordal Hold, columns 1-4 are the left hand, 5-8 the right hand, and
 * the holding keys in columns 0 and 9 are thumb keys of neither hand.
 */
class SplitProbe : public TestFixture {
  public:
    KeymapKey tab_sym{PROBE_LAYER_BASE, 0, 0, TAB_SYM};
    KeymapKey key_a{PROBE_LAYER_BASE, 1, 0, KC_A};
    KeymapKey key_b{PROBE_LAYER_BASE, 2, 0, KC_B};
    KeymapKey key_c{PROBE_LAYER_BASE, 3, 0, KC_C};
    KeymapKey ent_num{PROBE_LAYER_BASE, 9, 0, ENT_NUM};

    SplitProbe() {
        split_probe_reset();
        for (KeymapKey key : {tab_sym, key_a, key_b, key_c, ent_num}) {
            add_key(key);
        }
        const uint16_t col1[] = {TRG_NAV, TRG_FUN, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT};
        const uint16_t col2[] = {KC_1, KC_2, KC_LEFT, KC_F1, KC_F2};
        for (uint8_t i = 0; i < 5; i++) {
            const uint8_t layer = PROBE_LAYER_SYM + i;
            add_key(KeymapKey(layer, 0, 0, KC_TRANSPARENT));
            add_key(KeymapKey(layer, 1, 0, col1[i]));
            add_key(KeymapKey(layer, 2, 0, col2[i]));
            add_key(KeymapKey(layer, 3, 0, QK_LAYER_LOCK));
            add_key(KeymapKey(layer, 9, 0, KC_TRANSPARENT));
        }
    }

    void tap(KeymapKey& key) {
        key.press();
        run_one_scan_loop();
        key.release();
        run_one_scan_loop();
    }

    void hold(TestDriver& driver, KeymapKey& key) {
        EXPECT_NO_REPORT(driver);
        key.press();
        idle_for(TAPPING_TERM + 1);
        VERIFY_AND_CLEAR(driver);
    }

    void silent_tap(TestDriver& driver, KeymapKey& key) {
        EXPECT_NO_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }

    void silent_release(TestDriver& driver, KeymapKey& key) {
        EXPECT_NO_REPORT(driver);
        key.release();
        run_one_scan_loop();
        VERIFY_AND_CLEAR(driver);
    }

    void expect_tap(TestDriver& driver, KeymapKey& key, uint8_t code) {
        EXPECT_REPORT(driver, (code));
        EXPECT_EMPTY_REPORT(driver);
        tap(key);
        VERIFY_AND_CLEAR(driver);
    }
};

/* Hold the holding key, tap the trigger key: the target layer is on while held. */
TEST_F(SplitProbe, HeldThenTriggerSwitchesToTheTargetLayer) {
    TestDriver driver;

    hold(driver, tab_sym);
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_SYM));
    silent_tap(driver, key_a); /* TRG_NAV on the SYM layer */
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));

    expect_tap(driver, key_b, KC_LEFT);

    silent_release(driver, tab_sym);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_SYM));
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
    expect_tap(driver, key_b, KC_B);
}

/* Released soon after the trigger key, the target layer stays for one key. */
TEST_F(SplitProbe, QuickReleaseLeavesTheTargetLayerForOneKey) {
    TestDriver driver;

    hold(driver, tab_sym);
    silent_tap(driver, key_a);
    silent_release(driver, tab_sym);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_SYM));
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));

    expect_tap(driver, key_b, KC_LEFT);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
    expect_tap(driver, key_b, KC_B);
}

/* The trigger key again cancels, back to the layer of the holding key. */
TEST_F(SplitProbe, TriggerAgainCancels) {
    TestDriver driver;

    hold(driver, tab_sym);
    silent_tap(driver, key_a);
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));
    silent_tap(driver, key_a); /* transparent on NAV: TRG_NAV of SYM again */
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_SYM));
    expect_tap(driver, key_b, KC_1);
    silent_release(driver, tab_sym);
}

/* Layer Lock on the target layer keeps it on after the holding key. */
TEST_F(SplitProbe, LayerLockKeepsTheTargetLayer) {
    TestDriver driver;

    hold(driver, tab_sym);
    silent_tap(driver, key_a);
    silent_tap(driver, key_c); /* QK_LLCK locks the highest layer, NAV */
    EXPECT_TRUE(is_layer_locked(PROBE_LAYER_NAV));
    silent_release(driver, tab_sym);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_SYM));
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));

    expect_tap(driver, key_b, KC_LEFT);
    expect_tap(driver, key_b, KC_LEFT);
    silent_tap(driver, key_c);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
    expect_tap(driver, key_b, KC_B);
}

/*
 * The fast roll: holding key down, trigger key down and up, holding key up,
 * all within the tapping term.
 */
TEST_F(SplitProbe, FastRollWithinTheTappingTerm) {
    TestDriver driver;

#ifdef PROBE_ROLL_SETTLES_HOLD
    EXPECT_NO_REPORT(driver);
#else
    /* The tap-hold settings make it a tap: Enter and `a` are typed. */
    EXPECT_CALL(driver, send_keyboard_mock(_)).Times(AnyNumber());
    EXPECT_REPORT(driver, (KC_ENT, KC_A));
#endif
    ent_num.press();
    idle_for(10);
    key_a.press();
    idle_for(10);
    key_a.release();
    idle_for(10);
    ent_num.release();
    idle_for(10);
    VERIFY_AND_CLEAR(driver);

#ifdef PROBE_ROLL_SETTLES_HOLD
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_FUN));
    expect_tap(driver, key_b, KC_F1);
#endif
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_FUN));
    expect_tap(driver, key_b, KC_B);
}

#ifdef CHORDAL_HOLD
/*
 * A thumb holding key ('*' in chordal_hold_layout) and a trigger key of the
 * left hand, within the tapping term: Chordal Hold does not make it a tap.
 */
TEST_F(SplitProbe, ChordalHoldLeavesAThumbHoldingKeyAlone) {
    TestDriver driver;

    EXPECT_NO_REPORT(driver);
    tab_sym.press();
    idle_for(10);
    key_a.press();
    idle_for(10);
    key_a.release();
    idle_for(10);
    tab_sym.release();
    idle_for(10);
    VERIFY_AND_CLEAR(driver);
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));
    expect_tap(driver, key_b, KC_LEFT);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
}
#endif

#ifdef PROBE_TRI_LAYER
/* update_tri_layer_state in layer_state_set_user goes on working. */
TEST_F(SplitProbe, TriLayerCoexists) {
    TestDriver driver;

    hold(driver, tab_sym);
    hold(driver, ent_num);
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_TRI));
    expect_tap(driver, key_b, KC_F2);
    silent_release(driver, ent_num);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_TRI));

    silent_tap(driver, key_a);
    EXPECT_TRUE(layer_state_is(PROBE_LAYER_NAV));
    expect_tap(driver, key_b, KC_LEFT);
    silent_release(driver, tab_sym);
    EXPECT_FALSE(layer_state_is(PROBE_LAYER_NAV));
}
#endif
