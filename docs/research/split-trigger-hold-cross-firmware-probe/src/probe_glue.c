/* Throwaway probe for issue #37: what a keymap family would pass in. */

#include "probe_glue.h"

#include "quantum.h"
#include "split_probe.h"

const split_probe_entry_t split_probe_entries[] = {
    {TAB_SYM, TRG_NAV, PROBE_LAYER_NAV},
    {ENT_NUM, TRG_FUN, PROBE_LAYER_FUN},
};
const uint8_t split_probe_entry_count = ARRAY_SIZE(split_probe_entries);

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return split_probe_process_record(keycode, record);
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    split_probe_post_process_record(keycode, record);
}

#ifdef PROBE_TRI_LAYER
/* As in users/hydeik/hydeik.c, but with a third layer that is no target layer. */
layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, PROBE_LAYER_SYM, PROBE_LAYER_NUM, PROBE_LAYER_TRI);
}
#endif

#ifdef PROBE_HOLD_ON_OTHER_KEY_PRESS_PER_KEY
/* As in users/hydeik/hydeik.c: the thumb layer-tap keys hold on other key press. */
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    return keycode == TAB_SYM || keycode == ENT_NUM;
}
#endif
