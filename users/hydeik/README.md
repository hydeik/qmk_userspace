# The `hydeik` keymap family

A 34-key typing design with home-row mods, built against upstream QMK ([`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware)). It is applied to the `crkbd/rev4_1/mini` and `mothwing` keyboards.

How the repository is organised, and how to build and flash, is described in the [top-level README](../../README.md).

## Features

- Home-row mods on the BASE, SYM and NUM layers, in the order Ctrl, Alt, GUI, Shift from the little finger inwards.
- Tap-hold behaviour tuned per key: [Chordal Hold](https://docs.qmk.fm/tap_hold#chordal-hold), [Flow Tap](https://docs.qmk.fm/tap_hold#flow-tap), Permissive Hold, and per-key tapping term, quick tap term and hold-on-other-key-press.
- [Caps Word](https://docs.qmk.fm/features/caps_word), turned on by pressing both Shift keys or by double-tapping Shift.
- [Layer Lock](https://docs.qmk.fm/features/layer_lock), on a key in every layer except BASE.
- [Repeat Key](https://docs.qmk.fm/features/repeat_key) and Alternate Repeat Key, on the NAV layer. The Alternate Repeat Key is context-aware: for example it types `=` after an arithmetic operator, and completes `.` to `../`.
- One-shot Shift on a thumb key, and one-shot modifiers on the home row of the NAV layer.

## Layers

There are six layers:

| Layer | Contents | Reached by |
| --- | --- | --- |
| BASE | QWERTY with home-row mods | Default |
| SYM | Symbols and brackets | Holding the Tab thumb key |
| NUM | Numbers and arithmetic operators | Holding the Backspace thumb key |
| NAV | Arrow keys, navigation, undo/cut/copy/paste, Repeat Key | Holding the Space thumb key |
| MOU | Mouse movement, wheel and buttons | Holding the key at the `G` position on NAV |
| FUN | Function keys, volume, brightness, media controls, bootloader | Holding the SYM and NUM thumb keys together |

The layers are defined in `hydeik.h`. The finger keys of the SYM, NUM and FUN layers come from the common core (`users/hydeik_common/layers.h`), which `hydeik_vial` builds its layers from too; `hydeik.h` supplies the home-row mods and the thumb keys.

## Files

| File | Contents |
| --- | --- |
| `hydeik.h` | Layer numbers, custom keycodes, mod-tap aliases and the layer definitions. |
| `hydeik.c` | Tap-hold tuning, the Alternate Repeat Key rules and their macros, and the SYM + NUM → FUN tri-layer. |
| `config.h`, `rules.mk` | The settings specific to this keymap family. Settings that do not depend on the keymap family are in the common core. |
