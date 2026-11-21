---
status: accepted
---

# Layer switching is one state machine in the common core, triggered by `MO()` keys

Both keymap families switch to a target layer by tapping a trigger key while a thumb holding key is held, and a quick release of the holding key leaves the target layer pending or locked. We implement this as one state machine of our own in the common core (`users/hydeik_common/features/target_layer.{h,c}`), used unchanged by both families. A trigger key is the ordinary QMK keycode `MO(target layer)`: the state machine intercepts `MO()` for the layers that a family lists as target layers, and that key no longer acts as a momentary layer key.

The state machine knows no keymap and no keymap family. Each family passes in its table of holding key, target layer and what a quick release does, tells it which key presses are one-shot mod keys and when the timeout of a pending target layer is suspended, and forwards the three QMK hooks it needs. This follows the rules of ADR 0001; it is the first code in the common core that holds state.

## Considered Options

- **Custom keycodes as trigger keys.** Rejected. `hydeik` would take them from `QK_USER` and `hydeik_vial` from `QK_KB_0 + 9` onward, with matching `customKeycodes` entries in every `vial.json` and numbers that must never move, because keymaps stored by Vial hold them. `smart_mod`, Caps Word and Repeat Key would each have to be taught that these keys only switch layers. `MO()` needs none of this: Vial shows it as a built-in key that can be placed anywhere, and the three features already treat it as a layer-switching key.
- **One state machine per keymap family.** Rejected. The behaviour is specified once for both families, the code it relies on (tap-hold, layers, Layer Lock) is the same at both firmware pins, and a probe passes on the unit-test frameworks of both. Two copies would drift, which is what ADR 0001 exists to prevent. Flash is no reason either: `mothwing:hydeik` has more than 10 KB free.
- **The One Shots on Steroids community module.** Rejected for the reasons of ADR 0002, which do not depend on the keymap family. Its split trigger/hold option also never checks that the holding key is held and allows one holding key per trigger key.

## Consequences

- `MO()` of a target layer does nothing unless one of its holding keys is held. A keymap that wants a plain momentary key for such a layer cannot have one; layer-tap keys are unaffected.
- A target layer has exactly one kind of trigger key.
- The tri-layer FUN of `hydeik` and every `TO()` key of `hydeik_vial` go.
- What counts as "a key typed" is defined once, in the common core, and `smart_mod` uses that definition too.
- In `hydeik_vial` the two state machines see every key event, `target_layer` first; when their timeouts end together, the pending one-shot mods end first.
- `hydeik` gains `post_process_record_user` and `housekeeping_task_user` hooks.
- The unit tests of `target_layer` live in the common core and run on the test frameworks of both firmware sources, so `make test` and CI no longer cover `hydeik_vial` alone.

The behaviour itself is specified in the tickets of issue #35.
