# vial-qmk vs upstream QMK: constraints on sharing between the two keymap families

Research for [#2](https://github.com/hydeik/qmk_userspace/issues/2), a child of
the map [#1](https://github.com/hydeik/qmk_userspace/issues/1).

## Sources

Everything below was read from shallow clones of the two firmware sources taken
on 2026-11-18, plus the GitHub commit API for history. No firmware was compiled
for this research; see [Not verified](#not-verified).

| Short name | Firmware source | Commit read |
| --- | --- | --- |
| **vial** | `vial-kb/vial-qmk`, branch `vial` | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344` (2026-07-26) |
| **qmk** | `qmk/qmk_firmware`, branch `master` | `80973aa0b5413e9a5bbbf9293507f39e870f9aa3` (2026-11-18) |

Paths are cited as `<short name>:<path>:<line>` at those commits.

## How far vial-qmk lags

- The last upstream merge on the `vial` branch is merge commit
  `7b247bc8005ed8a2b38525a64d1873841939342f` ("Merge remote-tracking branch
  'qmk/master' into merge-2025-06-21", 2025-06-21), which pulled in upstream
  commit `664c0bc4c32d3f2c287f02ef4f786c32c0f51cb9` (2025-06-21). It reached
  `vial` through `967967e122427050da4e23939130a11cc273e4fc` on 2025-07-12. No
  later merge commit exists in the 100 most recent commits of `vial`.
- The newest breaking-changes changelog in vial is `docs/ChangeLog/20250525.md`
  (the QMK 0.29 cycle). qmk additionally has `20250831.md`, `20251130.md`,
  `20260222.md`, `20260531.md`, `20260830.md`; its newest tag is `0.34.8`.
- So vial is upstream `master` as of 2025-06-21 plus Vial's own changes: about
  17 months and five breaking-change cycles behind.

Concrete things upstream has that vial does not, found by diffing the trees:

- `SPECULATIVE_HOLD`, `get_speculative_hold()`, `SPECULATIVE_HOLD_FLOW_TERM`,
  and `within_flow_tap_term()` (`qmk:quantum/action_tapping.h:49-76,200-214`;
  added in `qmk:docs/ChangeLog/20251130.md:5`). Absent from
  `vial:quantum/action_tapping.h`.
- `quantum/process_keycode/process_oneshot.[ch]`, `process_quantum.[ch]`,
  `process_plover_hid.[ch]`, and `docs/features/battery.md` exist only in qmk.
- `builddefs/locate_keymap.mk` exists only in qmk; vial still locates the
  keymap inline in `build_keyboard.mk` (same search order, see below).

## 1. Tap-hold and key features in vial-qmk

All four features exist in vial. They arrived in two steps: the core code came
with the 2025-06-21 upstream merge, and Vial's own integration (default-on
build flags, GUI-editable settings) came in separate Vial commits.

| Feature | In upstream since | In vial core since | Vial integration |
| --- | --- | --- | --- |
| Repeat Key / Alt Repeat Key | 2023-05-28 changelog, PR #19700 (`vial:docs/ChangeLog/20230528.md:107`) | present well before the 2025 merge | `f5eb9a8e92` 2025-06-29 "Add Repeat Key and Alt Repeat Key to vial-qmk. (#906)" |
| Layer Lock | 2024-11-24 changelog, PR #23430 (`vial:docs/ChangeLog/20241124.md:42`) | at latest the 2025-06-21 merge | `15a3bb1ab9` 2025-06-28 "PDF(layer), Caps Word, and Layer Lock for vial-qmk. (#902)" |
| Chordal Hold | 2025-02-23 changelog, PR #24560 (`vial:docs/ChangeLog/20250223.md:19`) | at latest the 2025-06-21 merge | `3af5b0b0f7` 2025-06-22 "qmk_settings: add chordal hold" |
| Flow Tap | 2025-05-25 changelog, PR #25125 (`vial:docs/ChangeLog/20250525.md:5`) | the 2025-06-21 merge | `c94a57d613` 2025-06-22 "qmk_settings: add flow tap" |

Evidence that the code is there:

- `CHORDAL_HOLD`, `get_chordal_hold()`, `get_chordal_hold_default()`,
  `chordal_hold_handedness()` and `chordal_hold_layout`:
  `vial:quantum/action_tapping.c:75,752,769` and
  `vial:quantum/action_tapping.h:109-111`.
- `chordal_hold_layout` is generated as a weak array from the keyboard's JSON
  by `vial:lib/python/qmk/cli/generate/keyboard_c.py:207-239`, the same
  generator as upstream. Neither `keyboards/mothwing/keyboard.json` nor
  `keyboards/crkbd/rev4_1/mini/keyboard.json` carries `"hand"` fields in
  either firmware source, so both rely on the generator's symmetric-layout
  guess (`keyboard_c.py:215-218`). A keymap can still define its own
  `chordal_hold_layout` in both sources because the generated one is weak.
- `FLOW_TAP_TERM`, `is_flow_tap_key()`, `get_flow_tap_term()`:
  `vial:quantum/action_tapping.c:890,906`.
- Layer Lock: `vial:quantum/process_keycode/process_layer_lock.c`,
  `vial:docs/features/layer_lock.md`.
- Repeat Key: `vial:quantum/process_keycode/process_repeat_key.c`,
  `vial:docs/features/repeat_key.md`.

### The real constraint: Vial owns the callbacks

Having the features is not the limit. The limit is that a Vial build takes
over the configuration macros and the per-key callbacks that the `hydeik`
keymap family uses to tune them. `vial:builddefs/build_vial.mk` (included from
`vial:builddefs/common_features.mk:647-648` when `VIAL_ENABLE = yes`):

```make
QMK_SETTINGS ?= yes
TAP_DANCE_ENABLE ?= yes
ifeq ($(strip $(TAP_DANCE_ENABLE)), yes)
    OPT_DEFS += -DTAPPING_TERM_PER_KEY
endif
CAPS_WORD_ENABLE ?= yes
COMBO_ENABLE ?= yes
KEY_OVERRIDE_ENABLE ?= yes
LAYER_LOCK_ENABLE ?= yes
REPEAT_KEY_ENABLE ?= yes
...
ifeq ($(strip $(QMK_SETTINGS)), yes)
    AUTO_SHIFT_ENABLE := yes
    SRC += $(QUANTUM_DIR)/qmk_settings.c
    OPT_DEFS += -DQMK_SETTINGS \
        -DAUTO_SHIFT_NO_SETUP -DAUTO_SHIFT_REPEAT_PER_KEY -DAUTO_SHIFT_NO_AUTO_REPEAT_PER_KEY \
        -DPERMISSIVE_HOLD_PER_KEY -DHOLD_ON_OTHER_KEY_PRESS_PER_KEY -DQUICK_TAP_TERM_PER_KEY -DRETRO_TAPPING_PER_KEY \
        -DCOMBO_TERM_PER_COMBO -DCHORDAL_HOLD -DFLOW_TAP_TERM=321
endif
```

With `QMK_SETTINGS` on (the default), `vial:quantum/qmk_settings.c:292-347`
defines these as ordinary, non-weak functions that return values stored in
EEPROM and edited from the Vial GUI:

- `get_permissive_hold`, `get_hold_on_other_key_press`, `get_quick_tap_term`,
  `get_retro_tapping`, `get_combo_term`
- `get_auto_shifted_key`, `get_auto_shift_repeat`,
  `get_auto_shift_no_auto_repeat`
- `get_chordal_hold` (returns `get_chordal_hold_default()` only when the GUI
  bit is set, otherwise `true`)
- `get_flow_tap_term` (returns the GUI value `QS.flow_tap_term`; the compile
  time `FLOW_TAP_TERM=321` is only a switch that compiles the feature in)

Factory defaults are Chordal Hold off and Flow Tap term 0
(`vial:quantum/qmk_settings.c:211,216`).

Independently of `QMK_SETTINGS`, `vial:quantum/vial.c` defines, also non-weak:

- `get_tapping_term` whenever `TAPPING_TERM_PER_KEY` is defined
  (`vial.c:528-542`), which `build_vial.mk` forces when Tap Dance is on.
- `get_alt_repeat_key_keycode_user` whenever `REPEAT_KEY_ENABLE` is on and
  `NO_ALT_REPEAT_KEY` is not defined (`vial.c:731`, guard at
  `vial:quantum/vial.h:188-189`).
- The `tap_dance_actions`, `key_combos` and key override tables
  (`vial.c:515,556,610`), which are filled from EEPROM.

`users/hydeik/hydeik.c` at `06a85c3` defines `get_tapping_term`,
`get_hold_on_other_key_press`, `get_quick_tap_term`, `get_flow_tap_term`,
`get_chordal_hold` and `get_alt_repeat_key_keycode_user` (lines 22, 34, 49,
69, 92, 197), and `users/hydeik/config.h` defines `FLOW_TAP_TERM 125`,
`CHORDAL_HOLD`, `TAPPING_TERM_PER_KEY`, `HOLD_ON_OTHER_KEY_PRESS_PER_KEY` and
`QUICK_TAP_TERM_PER_KEY`. Every one of those collides with a symbol or macro
that a default Vial build already supplies.

Consequences for sharing:

- Tap-hold tuning written as C callbacks cannot be compiled into a default
  Vial build. In a Vial build that behaviour is runtime data in EEPROM, not
  source.
- The escape hatches visible in the source are `QMK_SETTINGS = no` (a `?=`
  default, so a keymap `rules.mk` can override it), `TAP_DANCE_ENABLE = no`
  (stops the forced `TAPPING_TERM_PER_KEY`), and `#define NO_ALT_REPEAT_KEY`.
  Each one removes the matching page from the Vial GUI.
- Code that does not touch those hooks is shareable as far as the firmware API
  is concerned: layer and keycode macros, `process_record_user` logic, Caps
  Word and Layer Lock configuration macros, custom keycodes.
- Code must avoid anything upstream added after 2025-06-21 (list above), or
  guard it.
- Vial pins keycode values with static asserts
  (`vial:quantum/vial_ensure_keycode.h:817-823`, e.g. `QK_REPEAT_KEY ==
  0x7C79`, `QK_LAYER_LOCK == 0x7C7B`), so those keycodes mean the same thing in
  both sources today.

## 2. Pointing a keymap at a differently named `users/` directory

Supported identically in both firmware sources.

- qmk: `qmk:builddefs/build_keyboard.mk:411-414`
- vial: `vial:builddefs/build_keyboard.mk:463-466`

```make
ifeq ("$(USER_NAME)","")
    USER_NAME := $(KEYMAP)
endif
USER_PATH := users/$(USER_NAME)
```

The keymap's `rules.mk` is included before this block in both
(`qmk:builddefs/build_keyboard.mk:145-146` via `locate_keymap.mk`;
`vial:builddefs/build_keyboard.mk:155-200`, including the External Userspace
keymap paths at 157-177), so `USER_NAME := <dir>` in a keymap `rules.mk` takes
effect. Both then prefer `$(QMK_USERSPACE)/users/$(USER_NAME)` when it exists
(`qmk:...:422-426`, `vial:...:474-476`) and pull in that directory's
`rules.mk`, `config.h` and `post_config.h` (`qmk:...:429-435`,
`vial:...:479-485`).

The official docs describe the override in the same words in both trees:
`docs/feature_userspace.md`, section "Override default userspace"
(`qmk:docs/feature_userspace.md:63-69`; the two files differ only in trailing
whitespace).

What `USER_NAME` does not change: `QMK_KEYMAP` and the auto-included header
name `QMK_KEYMAP_H` stay `$(KEYMAP)` / `$(KEYMAP).h`
(`qmk:builddefs/build_keyboard.mk:534`, `vial:...:584`). A shared directory
therefore cannot rely on a header named after the keymap being found; sources
must include the shared header by its real name.

So `hydeik` and `hydeik_vial` keymaps can both set `USER_NAME` to one common
directory. The build system allows it; section 1 decides how much of the
directory's content both can actually compile.

## 3. Including a file from another `users/<name>/` directory

The mechanics are the same in both firmware sources.

- The External Userspace root is on the search path:
  `VPATH += $(QMK_USERSPACE)` (`qmk:builddefs/build_keyboard.mk:417-419`,
  `vial:...:469-471`).
- Later, `VPATH += $(KEYMAP_PATH)` and `VPATH += $(USER_PATH)`
  (`qmk:...:488-489`, `vial:...:538-539`).
- Every `VPATH` entry becomes a `-I` include directory:
  `$(INTERMEDIATE_OUTPUT)_INC := $(VPATH) $(EXTRAINCDIRS) $(KEYBOARD_PATHS)`
  (`qmk:...:536`, `vial:...:586`) and
  `$1_INCFLAGS := $$(patsubst %,-I%,$$($1_INC))`
  (`qmk:builddefs/common_rules.mk:280`, `vial:builddefs/common_rules.mk:276`).
- Every `VPATH` entry is also searched for sources:
  `vpath %.c $(VPATH_SRC)` and friends (`common_rules.mk:6-12` in both).

What follows from that, in both sources:

- Header: `#include "users/<other>/file.h"` resolves against the userspace
  root. No extra configuration.
- Source: `SRC += users/<other>/file.c` in any `rules.mk` resolves through
  `vpath`. `SRC += $(QMK_USERSPACE)/users/<other>/file.c` also works since
  `QMK_USERSPACE` is an absolute path.
- Bare names: adding `VPATH += $(QMK_USERSPACE)/users/<other>` in a
  `rules.mk` makes `#include "file.h"` and `SRC += file.c` work without the
  prefix.
- Make fragments: `include $(QMK_USERSPACE)/users/<other>/rules.mk` works from
  a keymap or user `rules.mk`; `QMK_USERSPACE` is set by the top-level
  `Makefile` in both (`qmk:Makefile:47`, `vial:Makefile:42-43`).
- `config.h` is the exception. Only `$(USER_PATH)/config.h` and
  `$(KEYMAP_PATH)/config.h` are force-included (`qmk:...:430-431,458-459`,
  `vial:...:480-481,508-509`). A shared `config.h` fragment has to be
  `#include`d from one of those.

Both sources also ship Community Modules support (`modules/qmk/`,
`data/schemas/community_module.jsonschema`, the `generate-community-modules-*`
rules in `build_keyboard.mk`), but vial's is the mid-2025 version: it lacks the
`split_transaction_id_community_modules.inc` and `community_post_config.h`
generation that qmk has (`qmk:builddefs/build_keyboard.mk:225-233`), and
upstream has since moved to Community Modules `1.1.3`
(`qmk:docs/ChangeLog/20260830.md`). Modules are a possible sharing mechanism,
but one that is itself version-skewed.

## 4. `qmk userspace-compile` and `qmk userspace-doctor`

The subcommands live in each firmware source's `lib/python/qmk/cli/userspace/`,
so their behaviour follows the firmware source the build runs against.

| Aspect | vial | qmk |
| --- | --- | --- |
| `qmk.json` schema versions accepted | `1.1`, then `1.0` (`lib/python/qmk/userspace.py`, `data/schemas/user_repo_v1_1.jsonschema`) | identical; the `v1_1` schema file is byte-identical |
| `userspace-compile` flags | `-e`, `-n`, `-c`, `-j`, `-t` (`vial:lib/python/qmk/cli/userspace/compile.py:18-23`, `vial:docs/cli_commands.md:634`) | the same plus `-p` / `--print-failures` (`qmk:lib/python/qmk/cli/userspace/compile.py:23`, `qmk:docs/cli_commands.md:634-639`) |
| Failure reporting | Prints one `[ERRORS]` line per target; compiler output only in `.build/failed.log.<pid>.<kb>.<km>` | With `-p`, the generated makefile also cats every failed log to stdout (`qmk:lib/python/qmk/cli/mass_compile.py:41-63`) |
| Exit status of `userspace-compile` | `False` (non-zero) when any `failed.log.*` exists (`vial:lib/python/qmk/cli/mass_compile.py:91-93`) | same (`qmk:lib/python/qmk/cli/mass_compile.py:108-110`) |
| Build directory | always `<qmk_home>/.build` | `BUILD_DIR` from `-e` if given, else `<qmk_home>/.build` (`qmk:lib/python/qmk/cli/mass_compile.py:27`) |
| `userspace-doctor` exit status | always success: the function returns nothing (`vial:lib/python/qmk/cli/userspace/doctor.py`) | `0` if a userspace is detected, else `1` (`qmk:lib/python/qmk/cli/userspace/doctor.py:11-13`) |
| `userspace-doctor` on an invalid `qmk.json` | logs a warning, does not fail | logs a warning, does not fail by itself (`qmk:lib/python/qmk/cli/doctor/main.py`, `userspace_tests`) |
| `DUMP_CI_METADATA` | supported (`vial:builddefs/build_keyboard.mk:32`) | supported |
| `util/ci/generate_failure_markdown.sh` | present | present |

Upstream history for the two differences: `-p` was added by
`f6f627d07f` (2025-09-09, PR #25518) and the doctor return code by
`8ec3de0f92` (2025-11-11, PR #25775). Both are after vial's 2025-06-21 merge.

One `qmk.json` (`userspace_version` `1.1`) is valid for both. The existing CI
workaround in `.github/workflows/build_binaries.yaml` (probe `--help` for
`print-failures`, and dump `failed.log.*` in a separate step) matches what the
sources show and is still needed for vial. In vial, `userspace-doctor` is not
a gate: it cannot fail the job.

## 5. The `mothwing` keyboard definition

Present in both firmware sources at the same path, with identical content.

- `keyboards/mothwing/keyboard.json`, `keyboards/mothwing/readme.md`,
  `keyboards/mothwing/keymaps/default/keymap.c` in both;
  `diff -r vial/keyboards/mothwing qmk/keyboards/mothwing` reports no
  difference.
- Upstream history: added by `fa0b4511dd` (2023-07-14, PR #19342); last
  touched by `960c4969a5` (2025-04-22, PR #25181), which predates vial's
  merge. That is why the two copies match.
- It is an ATmega32U4 split (`matrix_pins` on `F4`.. / `D4`.., serial on
  `D2`).
- vial ships no `vial` keymap and no `vial.json` for `mothwing`; only
  `default` exists. A Vial build of `mothwing` therefore depends entirely on
  the `vial.json` and `config.h` kept in this repo under
  `keyboards/mothwing/keymaps/hydeik_vial/`. (`build_vial.mk` reads
  `$(KEYMAP_PATH)/vial.json`.) By contrast `crkbd` has an in-tree
  `vial:keyboards/crkbd/keymaps/vial`.

The same holds for `keyboards/crkbd/rev4_1/mini/keyboard.json`: present in
both sources, with no keymaps directory under `mini` in either.

So neither build target needs a keyboard definition in this repo, and the
`keyboards/<kb>/keymaps/<keymap>/` overlay paths are the same for both
firmware sources.

## Not verified

- Nothing was compiled. The statement that userspace definitions of
  `get_chordal_hold`, `get_flow_tap_term`, `get_tapping_term`,
  `get_alt_repeat_key_keycode_user` etc. fail to build under Vial is inferred
  from reading non-weak definitions in `qmk_settings.c` and `vial.c`; the
  exact failure (duplicate symbol at link time, or a macro redefinition
  diagnostic for `FLOW_TAP_TERM`) was not observed.
- That `QMK_SETTINGS = no`, `TAP_DANCE_ENABLE = no` and `NO_ALT_REPEAT_KEY`
  are sufficient to let the upstream-style callbacks compile in a Vial build
  was not tested, nor was the resulting firmware size on the ATmega32U4
  `mothwing` (which had 128 bytes of headroom at commit `06a85c3`).
- The cross-directory include and `SRC` forms in section 3 follow from the
  make rules cited; they were not exercised in a build.
- The exact upstream version tag corresponding to vial's merge point
  (`664c0bc4c3`) was not resolved; "0.29 cycle" is inferred from the newest
  changelog file present. The dates in the "in vial core since" column are
  upper bounds taken from the last merge, not from walking vial's full merge
  history.
- The Vial project's own documentation site was not consulted; vial-qmk
  carries no Vial-specific docs in its tree, so every Vial claim here comes
  from its source.
- How the installed `qmk` CLI wrapper dispatches to a firmware source's
  `lib/python` was not re-read; it is assumed from the subcommands living in
  each firmware tree.
