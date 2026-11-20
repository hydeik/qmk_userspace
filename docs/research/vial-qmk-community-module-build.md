# Building the One Shots on Steroids community module against the pinned vial-qmk

Research for issue #23. Date: 2026-11-20.

## Answer

Yes. The vial-qmk commit pinned for the `hydeik_vial` keymap family
(`dd43959ae5c08d8a28d38a1acf7b04e86b14a344`) builds One Shots on Steroids
(OSoS) as a real community module from this userspace. No pin bump is needed.

It takes four things:

1. The module sources at `modules/Kawamashi/oneshots_on_steroids/` in this
   repository.
2. A `keymap.json` containing only `{"modules": ["Kawamashi/oneshots_on_steroids"]}`
   next to the existing `keymap.c`. The `keymap.c` stays as it is.
3. `OS_STEROIDS_COUNT` in `config.h`, the trigger keycodes in the `QK_KB_0`
   enum, and the `oneshot_os[]` array in userspace code.
4. **A change to the local `Makefile`.** As it stands, `make <keyboard>:hydeik_vial`
   fails once the `keymap.json` exists, because plain `make` with
   `QMK_USERSPACE` ignores the `keymap.c` beside a userspace `keymap.json`.
   The CI build (`qmk userspace-compile`) works unchanged, apart from fetching
   the module if it is added as a git submodule.

The fallback, compiling the module sources as plain userspace code, also
builds. It costs one deleted line in a vendored copy of the module and three
hook calls, and needs no `keymap.json` and no `Makefile` change.

Only compilation and linking were tested. Nothing was flashed, so runtime
behaviour under Vial is not verified.

## Sources

| Short name | Source |
| --- | --- |
| vial-qmk | `vial-kb/vial-qmk` at `dd43959ae5c08d8a28d38a1acf7b04e86b14a344` (the pin in `firmware_sources.json`). Paths below are relative to <https://github.com/vial-kb/vial-qmk/tree/dd43959ae5c08d8a28d38a1acf7b04e86b14a344>. |
| module | `Kawamashi/qmk_modules` at `d8dcb1351910ee003428c38768ae7c026d82c9ff` (head of `main` on 2026-11-20), directory `oneshots_on_steroids/`: <https://github.com/Kawamashi/qmk_modules/tree/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids> |
| QMK docs | <https://docs.qmk.fm/features/community_modules>. The same text is in vial-qmk at `docs/features/community_modules.md`. |
| Vial docs | `vial-kb/vial-kb.github.io` at `4c996015fd64b8e8120ce66251e0634d8dc72dbd`, `docs/docs/custom_keycode.md` |
| Vial GUI | `vial-kb/vial-gui` at `aef8222a2d0429a183b2ed692d5f9efcfd383f08`, `src/main/python/keycodes/keycodes_v6.py` |

## Build experiments

All builds were of `crkbd/rev4_1/mini:hydeik_vial`, in a worktree of this
repository at `02643d8`, against the checkout that `make firmware` put in
`.firmware/hydeik_vial` (verified at the pinned commit by
`firmware_sources.py locate`). Toolchain: `arm-none-eabi-gcc` 15.2.0 from
`qmk/qmk_toolchains` release `v15.2.0-1`, QMK CLI 1.2.0, macOS arm64. The
experiment code was not committed.

Experiment setup for the module builds (A1 to A5):

- the six module files copied to `modules/Kawamashi/oneshots_on_steroids/`;
- `keyboards/crkbd/rev4_1/mini/keymaps/hydeik_vial/keymap.json` with
  `{"modules": ["Kawamashi/oneshots_on_steroids"]}`, beside the unchanged
  `keymap.c`;
- from A2 on: `#define OS_STEROIDS_COUNT 3` in `users/hydeik_vial/config.h`,
  three keycodes appended to the `QK_KB_0` enum in `users/hydeik_vial/layout.h`,
  and a three-entry `oneshot_os[]` (a Shift key, a GUI key, a `_NAV` layer key)
  in `users/hydeik_vial/hydeik_vial.c`, which included `oneshots_on_steroids.h`.

| # | Build | Result |
| --- | --- | --- |
| Baseline | `make crkbd/rev4_1/mini:hydeik_vial`, no module, clean | OK, `text` = 69652 bytes |
| A1 | Local `make`, module and `keymap.json` only | Fails as expected: `'OS_STEROIDS_COUNT' undeclared` in the module. Shows the module is found in the userspace and compiled. |
| A2 | Local `make`, full setup | **Fails**: `'keymaps' undeclared`, `'key_combos' undeclared`, `'tap_dance_actions' undeclared` in `quantum/keymap_introspection.c`. The module itself compiled. |
| A4 | Local `make` plus `MAIN_KEYMAP_PATH_1..5=<absolute keymap directory>` on the command line, clean | OK, `text` = 70520 bytes |
| A5 | `qmk compile -kb crkbd/rev4_1/mini -km hydeik_vial`, run from inside `.firmware/hydeik_vial` with `QMK_USERSPACE` set, clean | OK, `text` = 70520 bytes |
| CI | `firmware_sources.py select hydeik_vial`, `qmk userspace-doctor`, `qmk userspace-compile -e DUMP_CI_METADATA=yes` (the CI steps), clean | OK, `text` = 70520 bytes |
| B1 | Fallback: no `keymap.json`, no `modules/`, module sources vendored under `users/hydeik_vial/`, local `make` without extra arguments, clean | OK, `text` = 70520 bytes |

The 868-byte growth over the baseline shows the module code is linked in, not
discarded. In A4, A5 and CI the log shows `quantum/vial.c` and
`quantum/vialrgb.c` compiled, so Vial stayed enabled.

Not covered by the experiments:

- The CI container (`ghcr.io/qmk/qmk_cli:latest`) and its compiler. The CI
  commands were run locally with the toolchain above.
- Anything at run time: OSoS behaviour, its interaction with Vial's dynamic
  keymap, tap dance, combos and QMK settings.
- The `-p` flag of `qmk userspace-compile`. vial-qmk at the pin does not
  accept it, which the workflow already guards against.

## Question 1: build machinery, API version, module requirements

### Verified

- The pinned vial-qmk carries the complete community-module machinery, merged
  from upstream QMK:
  - `data/constants/module_hooks/` holds `0.1.0.hjson`, `1.0.0.hjson`,
    `1.1.0.hjson`, `1.1.1.hjson`. The newest file name is the API version
    (`lib/python/qmk/community_modules.py`, `module_api_list()`), and the
    generated `community_modules.h` in the A builds defines
    `COMMUNITY_MODULES_API_VERSION` as `COMMUNITY_MODULES_API_VERSION_BUILDER(1,1,1)`.
    **The module API version is 1.1.1.**
  - `builddefs/build_keyboard.mk` lines 254 to 290 generate
    `community_rules.mk`, `community_modules.h`, `community_modules.c` and the
    introspection includes with `qmk generate-community-modules-*`
    (`lib/python/qmk/cli/generate/community_modules.py`).
  - Core calls the generated hooks: `process_record_modules` before
    `process_record_kb` (`quantum/quantum.c` line 363),
    `post_process_record_modules` (line 296), `housekeeping_task_modules`
    (`quantum/keyboard.c` line 434).
  - History: `1.0.0.hjson` arrived with upstream "Community modules (#24848)"
    (`1efc82403bebe759272d1ba7a79d9dfa0d5df506`, 2025-02-26) and `1.1.1.hjson`
    with "#25187" (`f4171412a676ae3cbd1cd50e859a7deb1a554e15`, 2025-05-11).
    Both are ancestors of the pin.
- What the module requires (module source):
  - `ASSERT_COMMUNITY_MODULES_MIN_API_VERSION(1, 0, 0)` in
    `oneshots_on_steroids.c`. 1.0.0 is at or below 1.1.1, and the assertion
    passed in every A build.
  - The hooks it implements: `process_record_oneshots_on_steroids` and
    `post_process_record_oneshots_on_steroids` (API 0.1.0) and, when
    `OS_STEROIDS_TIMEOUT > 0` (default 5000), `housekeeping_task_oneshots_on_steroids`
    (API 1.0.0).
  - `qmk_module.json` declares only `module_name`, `maintainer`, `license`
    (`GPL-3.0-or-later`) and `url`. It declares **no `keycodes` and no
    `features`**.
  - `rules.mk` adds `oneshots_on_steroids_utils.c`. `introspection.h` only
    includes `oneshots_on_steroids.h`, which makes the types visible to
    `keymap.c`. There is no `introspection.c`.
  - From the user: `OS_STEROIDS_COUNT` must be defined in `config.h` (there is
    no default; A1 fails without it), and a
    `const oneshot_on_steroids_t oneshot_os[]` must be defined (declared
    `extern` in the header).
  - Stock one-shot keys must be available: the module has
    `#error` under `NO_ACTION_ONESHOT` and calls `add_oneshot_mods` /
    `del_oneshot_mods`. The `hydeik_vial` build does not define
    `NO_ACTION_ONESHOT` (the A builds passed).
  - `neutralize_flashing_modifiers` is used only if
    `DUMMY_MOD_NEUTRALIZER_KEYCODE` is defined. It exists at the pin
    (`quantum/action_util.c` line 523).
- The module's top-level `README.md` asks for QMK 0.28.0 (2025-02-27) or
  later. The pin postdates that.

### Inference

- The module is GPL-3.0-or-later while this repository's own sources are
  marked GPL-2.0+. A firmware binary that links both is therefore distributed
  under GPL-3. This matters only if the module files are vendored into the
  repository or binaries are published (the `latest` release publishes them).

## Question 2: what enabling the module needs here

### Verified

- **Module location.** `find_available_module_paths()` in
  `lib/python/qmk/community_modules.py` searches `<QMK_USERSPACE>/modules/`
  and then `<QMK_FIRMWARE>/modules/`, and a module is named by its path below
  `modules/`. The module's `README.md` expects the whole `Kawamashi/qmk_modules`
  repository at `modules/Kawamashi`, which makes the name
  `Kawamashi/oneshots_on_steroids`. In A1 the generated `community_rules.mk`
  pointed at the userspace copy, so the userspace location works with the
  pinned firmware.
- **`keymap.json` is the only switch available to a userspace.**
  `get_modules()` in `lib/python/qmk/info.py` takes modules from the
  keyboard's `info.json`/`keyboard.json` (inside the firmware tree) and from
  the `modules` key of the keymap JSON. There is no `rules.mk` variable for it.
- **`keymap.json` and `keymap.c` can coexist.** `build_keyboard.mk` lines 212
  to 218: when a `keymap.json` is found and a `keymap.c` was found too, the
  `keymap.c` is passed as `OTHER_KEYMAP_C`, and the `keymap.c` generated from
  the JSON includes it (`lib/python/qmk/keymap.py` lines 37 to 39). A
  `keymap.json` that holds only `modules` (no `keyboard`, `keymap`, `layout`
  or `layers`) was accepted in A4, A5 and CI.
- **The local `Makefile` route breaks (A2).** The repository `Makefile` runs
  `make -C <checkout> <keyboard>:<keymap> QMK_USERSPACE=<repo>`. For a
  userspace `keymap.json`, `builddefs/build_json.mk` lines 19 to 36 set
  `KEYMAP_JSON` and `KEYMAP_PATH` but not `KEYMAP_JSON_PATH`. Then in
  `build_keyboard.mk`:
  - line 155: `KEYMAP_PATH` is already set, so the search for `keymap.c` and
    the keymap's `rules.mk` (lines 156 to 208) is skipped, `KEYMAP_C` stays
    empty and `OTHER_KEYMAP_C` is never set;
  - line 220: `KEYMAP_PATH := $(KEYMAP_JSON_PATH)` empties `KEYMAP_PATH`, so
    the keymap's `rules.mk` (line 227), `config.h` (line 508) and `vial.json`
    (`builddefs/build_vial.mk`) are not looked up in the keymap directory.

  The A2 errors match: `keymaps` is undeclared because `keymap.c` was not
  included, and the combo and tap dance arrays are demanded because
  `VIAL_ENABLE = yes` from the keymap's `rules.mk` was never read.
- **The QMK CLI route works (A5, CI).** `KeyboardKeymapBuildTarget.compile_command()`
  in `lib/python/qmk/build_targets.py` passes
  `MAIN_KEYMAP_PATH_1..5=<absolute keymap directory>` when the keymap is in the
  userspace. With that, the first block of `build_json.mk` (lines 2 to 17)
  finds the JSON and sets `KEYMAP_JSON_PATH`, and the `keymap.c` search finds
  the `keymap.c`. CI uses this route (`qmk userspace-compile`), so
  `.github/workflows/build_binaries.yaml` builds the module without changes to
  its build step.
- **Passing the same variables to plain `make` works (A4).** Command-line
  variables reach the inner make, so
  `make crkbd/rev4_1/mini:hydeik_vial MAIN_KEYMAP_PATH_1=<dir> ... MAIN_KEYMAP_PATH_5=<dir>`
  through the repository `Makefile` built successfully.
- `qmk.json` needs no change. `firmware_sources.py` needs no change. The
  `users/hydeik_vial/rules.mk` and `config.h` were picked up as before in the
  successful builds.
- The CI "Checkout userspace" step uses `actions/checkout@v6` without
  `submodules`, so a module added as a git submodule would be absent in CI.

### Two ways to fix the local build (both verified to build)

1. Keep plain `make` and have the `Makefile` pass `MAIN_KEYMAP_PATH_1..5` with
   the absolute keymap directory. The `Makefile` then has to find that
   directory itself (a keymap may sit at any level of the keyboard path).
2. Have the `Makefile` call `qmk compile -kb <keyboard> -km <keymap>` from
   inside the firmware checkout with `QMK_USERSPACE` set. Observed with QMK
   CLI 1.2.0: a firmware tree containing the current directory wins over the
   configured `user.qmk_home`, but `user.qmk_home` wins over the `QMK_HOME`
   environment variable. Running from outside the checkout with only
   `QMK_HOME` set silently built in the globally configured firmware tree.
   ADR 0001 currently states that `qmk compile` with a single `qmk_home` is
   not supported; this variant does not rely on `qmk_home`.

### Inference

- For getting the sources into `modules/`: a git submodule at
  `modules/Kawamashi` follows the module's and QMK's documentation and pins
  the module to a commit, at the price of `submodules: true` on the CI
  userspace checkout and a submodule step for local clones. Vendoring the six
  files needs neither. Only the copied-files layout was built here; a
  submodule yields the same directory layout, so it should behave the same.
- Upstream QMK at the `hydeik` pin was not checked for the same plain-`make`
  limitation.

## Question 3: module keycodes, `SAFE_RANGE` and Vial

### Verified

- Keycode ranges at the pin (`quantum/keycodes.h`): `QK_COMMUNITY_MODULE` is
  0x77C0 to 0x77FF, `QK_KB` is 0x7E00 to 0x7E3F, `QK_USER` is 0x7E40 to 0x7FFF.
  `SAFE_RANGE` is `QK_USER` (`quantum/quantum_keycodes.h` line 37).
- Keycodes a module declares in `qmk_module.json` are generated into an enum
  starting at `QK_COMMUNITY_MODULE` (`_render_keycodes()` in
  `lib/python/qmk/cli/generate/community_modules.py`).
- **OSoS declares no keycodes.** In the A builds the generated enum contained
  only `LAST_COMMUNITY_MODULE_KEY`. The trigger keycodes are ordinary custom
  keycodes that the user defines and lists in `oneshot_os[]`; the module only
  compares `keycode == oneshot_os[i].trigger`, so their numeric range does not
  matter to it. The module's readme uses `SAFE_RANGE` only as an example.
- Vial expects custom keycodes from `QK_KB_0`: the Vial docs say to replace
  `SAFE_RANGE` with `QK_KB_0` and to list the keycodes, in enum order, under
  `customKeycodes` in `vial.json`. The Vial GUI maps `USER00` to `USER63` onto
  `QK_KB + 0..63` and has no entry for the community-module range.
- Putting the OSoS triggers in the existing `QK_KB_0` enum of
  `users/hydeik_vial/layout.h` compiles and links (A4, A5, CI).
- The `QK_KB` range holds 64 keycodes. The enum currently uses 11
  (`COS_MOD` to `SW_WIN`).
- Module hooks run before keyboard, VIA, Vial and user processing
  (`quantum/quantum.c` lines 363 to 370), and OSoS returns `false` for its
  trigger keys, so `process_record_user` does not see them.

### Inference

- So for OSoS there is no conflict: define the triggers in the `QK_KB_0` enum
  and add matching `customKeycodes` entries to `vial.json`, exactly as for the
  current custom one-shot keys. They then appear in the GUI's User tab and can
  be remapped like any key.
- A `SAFE_RANGE`-based keycode would still work in firmware (it is a valid
  16-bit keycode that the dynamic keymap can store) but the GUI would show it
  as a raw hex value. The same holds for a module that does declare keycodes
  in the 0x77C0 range. Neither was tested in the GUI.
- `vial.json` currently lists 10 `customKeycodes` for 11 enum entries
  (`SW_WIN` has none). New entries must keep the list in enum order, so
  `SW_WIN` needs an entry, or must move to the end, before OSoS keycodes are
  appended after it.
- The `layer` field of `oneshot_os[]` is a compile-time layer number. Vial can
  remap keys at run time but cannot change which layer an OSoS key activates.

## Question 4: fallback, compiling the module as plain userspace code

Not needed, since the module route builds. For comparison, B1 built it this
way.

### Verified

- What the module relies on from the module system:
  1. `ASSERT_COMMUNITY_MODULES_MIN_API_VERSION`. The macro comes from the
     generated `community_modules.h`, which `quantum/quantum.h` (lines 248 to
     250) includes only under `COMMUNITY_MODULES_ENABLE`. Without the module
     system that one line in `oneshots_on_steroids.c` must go.
  2. Three hooks, which nothing calls without the generated
     `community_modules.c`: `process_record_oneshots_on_steroids`,
     `post_process_record_oneshots_on_steroids`,
     `housekeeping_task_oneshots_on_steroids`. They are not declared in
     `oneshots_on_steroids.h` either.
  3. Introspection: only `introspection.h`, which includes the module header
     ahead of `keymap.c`. A plain `#include "oneshots_on_steroids.h"` replaces
     it. Nothing in the module reads the keymap through introspection.
- B1 changes, all in `users/hydeik_vial/`:
  - a copy of `oneshots_on_steroids.c` (minus the assert line),
    `oneshots_on_steroids_utils.c` and `oneshots_on_steroids.h` in a
    subdirectory;
  - `rules.mk`: that subdirectory added to `VPATH`, the two `.c` files added
    to `SRC`;
  - `hydeik_vial.c`: prototypes for the three hooks, a call to
    `process_record_oneshots_on_steroids` at the start of
    `process_record_user` (stop when it returns `false`), a
    `post_process_record_user` that calls
    `post_process_record_oneshots_on_steroids`, and a `housekeeping_task_user`
    that calls `housekeeping_task_oneshots_on_steroids`;
  - the same `config.h`, keycode and `oneshot_os[]` additions as the module
    route.
- B1 built with the unchanged repository `Makefile` and no `keymap.json`.

### Inference

- Cost of the fallback: a vendored, slightly patched copy that must be
  re-patched on every module update, and three hook functions owned by the
  userspace. In exchange the local `Makefile` and the CI workflow stay as they
  are.
- Ordering differs slightly: as a module, OSoS runs before
  `process_record_kb`; in the fallback it runs inside `process_record_user`,
  which the keyboard's `process_record_kb` normally calls first. Whether that
  changes behaviour on this keyboard was not tested.

## Question 5: earliest vial-qmk commit that works

Not applicable. The pinned commit works. For reference, the module's minimum
(API 1.0.0) entered the tree with upstream commit
`1efc82403bebe759272d1ba7a79d9dfa0d5df506` (2025-02-26), long before the pin.

## Open points

- Runtime behaviour on hardware under Vial was not tested.
- The CI container's toolchain was not used; only the CI commands were
  reproduced locally.
- Whether upstream QMK at the `hydeik` pin has the same plain-`make`
  limitation for a userspace `keymap.json` was not checked.
