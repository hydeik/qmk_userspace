---
status: accepted
---

# One keymap family, built with and without Vial

ADR 0001 kept two keymap families side by side and put what both used in a common core. The home-row-mod keymap family is deleted (issue #53): only the family of one-shot mods and the smart mod layer is maintained, and nothing of the deleted family is carried over except its mouse layer. With one keymap family the split has lost its reason, and this ADR supersedes ADR 0001. The remaining family is named `hydeik` and lives in one directory, `users/hydeik/`, with no common core. It is built both with Vial, on vial-qmk, and without Vial, on upstream QMK, and the keymap of a build target selects which.

1. **One directory.** `users/hydeik/` holds the settings, the layers, the two state machines `smart_mod` and `target_layer`, the layout adapters and the unit tests. Sources include each other relative to that directory.
2. **What exists only with Vial is chosen in `rules.mk`.** `users/hydeik/rules.mk` adds the firmware defaults of the Vial settings (`features/firmware_default.c`) and the QMK hooks that forward to them (`vial_hooks.c`), and switches combos and tap dances on, only when `VIAL_ENABLE` is yes. No C source branches on Vial: there is no `#ifdef VIAL_ENABLE`, and every other source is the same with and without Vial.
3. **The keymap selects the firmware source.** A keymap is the keymap family applied to one keyboard, either with or without Vial: `hydeik` is the keymap without Vial and `hydeik_vial` the keymap with Vial. Both use `users/hydeik/`; `hydeik_vial` reaches it with `USER_NAME := hydeik` in its `rules.mk`. `firmware_sources.json` declares the firmware sources by name, `qmk` and `vial-qmk`, each pinned to a commit and listing the keymaps built against it. Nothing is inferred from the form of a keymap name.

## Considered Options

- **Keep the common core beside one keymap family.** Rejected: its rules existed to keep two families from drifting apart and from depending on each other. With one family they only split one design over two directories and force includes by full path.
- **An `#ifdef VIAL_ENABLE` around the hooks that exist only with Vial.** Rejected: it puts the condition into every build of that source, the unit tests included, where separate source files keep it in one place in `rules.mk`.
- **Keep `firmware_sources.json` keyed by keymap name.** Rejected: the checkout of upstream QMK would be named after a keymap, and a unit test would name a firmware source by a keymap.

## Consequences

- ADR 0002 and ADR 0003 stay accepted and are not rewritten. Where they say `users/hydeik_vial/`, the common core or `users/hydeik_common/`, the code is now in `users/hydeik/`: `smart_mod` and `target_layer` in `features/`, and `key_typed`, the one definition of a key typed, beside them. Where they say the keymap family `hydeik_vial`, it is the keymap family `hydeik`; what they say of the home-row-mod family `hydeik` no longer applies.
- The callbacks that `smart_mod` and `target_layer` take from their caller stay. They are the interfaces of the two modules and the seam of their unit tests, not an indirection for two families.
- The layers are spelled out once, with plain keycodes, in `users/hydeik/layout.h`. The layer grids with their hooks for home-row mods are gone.
- A unit test names the firmware sources it runs on. All run on both, except the test of `firmware_default`, which needs vial-qmk.
- A keymap without Vial has no combos, no tap dances and no stored keymap.
- What ADR 0001 says of pinned firmware sources, of the build targets in `qmk.json` being the complete list, and of local builds going through `make` still holds.

The detailed decisions behind this are recorded in the tickets of issue #53.
