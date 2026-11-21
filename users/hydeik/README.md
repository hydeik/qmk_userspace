# The `hydeik` keymap family

A 34-key typing design with home-row mods, built against upstream QMK ([`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware)). It is applied to the `crkbd/rev4_1/mini` and `mothwing` keyboards.

How the repository is organised, and how to build and flash, is described in the [top-level README](../../README.md).

## Features

- Home-row mods on the BASE, SYM and NUM layers, in the order Ctrl, Alt, GUI, Shift from the little finger inwards.
- Tap-hold behaviour tuned per key: [Chordal Hold](https://docs.qmk.fm/tap_hold#chordal-hold), [Flow Tap](https://docs.qmk.fm/tap_hold#flow-tap), Permissive Hold, and per-key tapping term, quick tap term and hold-on-other-key-press.
- [Caps Word](https://docs.qmk.fm/features/caps_word), turned on by pressing both Shift keys or by double-tapping Shift.
- [Layer Lock](https://docs.qmk.fm/features/layer_lock), on a key in every layer except BASE.
- [Repeat Key](https://docs.qmk.fm/features/repeat_key) and Alternate Repeat Key, on the NAV layer. The Alternate Repeat Key is context-aware: for example it types `=` after an arithmetic operator, and completes `.` to `../`.
- A one-shot mod for Shift on a thumb key, and one-shot mods on the home row of the NAV layer.

## Layers

There are six layers:

| Layer | Contents | Reached by |
| --- | --- | --- |
| BASE | QWERTY with home-row mods | Default |
| SYM | Symbols and brackets | Holding the Tab thumb key |
| NUM | Numbers and arithmetic operators | Holding the Backspace thumb key |
| NAV | Arrow keys, navigation, undo/cut/copy/paste, Repeat Key | Holding the Space thumb key |
| MOU | Mouse movement, wheel and buttons | Holding the Space thumb key and pressing the trigger key at the `G` position |
| FUN | Function keys, volume, brightness, media controls, bootloader | Holding the Backspace thumb key and pressing the trigger key at the bottom-right corner |

### Reaching MOU and FUN

MOU and FUN are target layers, switched by the `target_layer` state machine of the common core (`users/hydeik_common/features/target_layer.h`). A target layer is switched on by its trigger key, pressed while its holding key is held:

| Holding key | Trigger key | Target layer | Quick release of the holding key |
| --- | --- | --- | --- |
| Space thumb key (`NAV_SPC`) | `MO(_MOU)` at the `G` position | MOU | MOU becomes a locked target layer |
| Backspace thumb key (`NUM_ENT`) | `MO(_FUN)` at the bottom-right corner | FUN | FUN becomes a pending target layer |

- The target layer stays on until the holding key is released; the trigger key itself may be released at once. Pressing the trigger key again switches the target layer off.
- A quick release is a release of the holding key within the One Shot Term (`TARGET_LAYER_ONE_SHOT_TERM`, 200 ms) after the press of the trigger key, with no key typed in between.
- After a quick release, MOU is locked and both hands are free. Leave it with the trigger key at the same position, with Layer Lock, or by the idle timeout of Layer Lock (60 s). While MOU is locked, a tap of the left thumb keys still sends Tab or Space.
- After a quick release, FUN stays on for the next typed key only. A one-shot mod does not use it up, so the one-shot mod for Shift followed by `F5` sends Shift+`F5`. A pending FUN ends by itself after `TARGET_LAYER_PENDING_TIMEOUT` (2 s).
- A trigger key is the ordinary keycode `MO()` of its target layer, but it is no momentary layer key: without its holding key held it does nothing.
- The SYM and NUM thumb keys held together no longer give FUN; they give NUM, the higher layer.

The layers are defined in `hydeik.h`. The finger keys of the SYM, NUM and FUN layers come from the common core (`users/hydeik_common/layers.h`), which `hydeik_vial` builds its layers from too; `hydeik.h` supplies the home-row mods and the thumb keys.

## Files

| File | Contents |
| --- | --- |
| `hydeik.h` | Layer numbers, custom keycodes, mod-tap aliases and the layer definitions. |
| `hydeik.c` | Tap-hold tuning, the Alternate Repeat Key rules and their macros, and the wiring of `target_layer`: its table of entries and the three hooks forwarded to it. |
| `config.h`, `rules.mk` | The settings specific to this keymap family. Settings that do not depend on the keymap family are in the common core. |
