# Can one split trigger/hold state machine in the common core serve both firmware sources?

Research for [#37](https://github.com/hydeik/qmk_userspace/issues/37), a child
of the map [#35](https://github.com/hydeik/qmk_userspace/issues/35). It builds
on [`vial-qmk-constraints.md`](https://github.com/hydeik/qmk_userspace/blob/research/vial-qmk-constraints/docs/research/vial-qmk-constraints.md)
(issue #2) and does not repeat it.

"Holding key" and "trigger key" are the working terms of the map: a thumb
layer-tap key that is held, and a key tapped with another finger that moves to
the target layer.

## Answer

**Yes.** One implementation in `users/hydeik_common/` that never branches on
the firmware source is feasible. Everything such a state machine relies on is
either the same code in both firmware sources or differs only in ways the
state machine never sees:

- The file that decides tap versus hold (`quantum/action_tapping.c`) differs
  only by upstream's Speculative Hold, which is opt-in, applies to mod-tap keys
  only, and is not used here. A layer-tap key reaches `process_record_user`
  with the same `tap.count` in both.
- The layer API, Layer Lock and `update_tri_layer_state` are identical.
- The hooks a state machine needs (`process_record_user`,
  `post_process_record_user`, `housekeeping_task_user`) are weak in both and
  are called at the same points of the same processing order.
- A throwaway probe of the state machine, with no `#ifdef` on the firmware
  source, was compiled and run on the unit-test framework of **both** pinned
  refs under three tap-hold configurations: all tests pass on both. The
  existing `smart_mod` tests (89 tests) also pass unchanged on upstream QMK.

What differs is all on the keymap family's side of the seam, so it is passed in
or lives in the family (ADR 0001, rule 2):

1. **The number of the trigger keycode.** `hydeik` takes it from `QK_USER`;
   `hydeik_vial` must take it from `QK_KB_0 ..` to be placeable and named in
   the Vial GUI. The common core must receive the keycodes, never define them.
2. **When a holding key counts as held.** In `hydeik` this is compiled in
   (Hold On Other Key Press for the thumb layer-tap keys). In `hydeik_vial` it
   is Vial's QMK Settings in EEPROM, all off by factory default, and reset to
   that default by every newly built firmware. The state machine is unaffected;
   how the fast roll feels is not.
3. **Tri-layer** exists in `hydeik` only, and its third layer cannot be a
   target layer (see [Tri-layer](#3-tri-layer)).
4. **Vial-only event sources** (tap dance outputs at a magic key position,
   GUI-defined combos) call for two defensive rules in the state machine, which
   are harmless on upstream QMK (see [Combos and tap dance](#5-combos-and-tap-dance)).

What each family would pass in is listed under
[What each keymap family passes in](#what-each-keymap-family-passes-in).

## Sources and method

| Short name | Firmware source | Commit (from `firmware_sources.json`) |
| --- | --- | --- |
| **qmk** | `qmk/qmk_firmware`, branch `master` | `80973aa0b5413e9a5bbbf9293507f39e870f9aa3` |
| **vial** | `vial-kb/vial-qmk`, branch `vial` | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344` |
| **vial-gui** | `vial-kb/vial-gui`, branch `main` | `aef8222a2d0429a183b2ed692d5f9efcfd383f08` (2026-05-25, head at the time of reading; not pinned by this repository) |

Paths are cited as `<short name>:<path>:<line>` at those commits. Where one
line number is given for both, the file is identical.

Three kinds of evidence are used, and each claim below says which:

- **Read**: the source at the pinned commit was read, and the two firmware
  sources were compared with `diff`. vial was read from the checkout that
  `make firmware` keeps in `.firmware/hydeik_vial` (verified to be at the
  pin); qmk from a shallow fetch of exactly the pinned commit.
- **Hand-traced**: a conclusion followed through the code by reading, not
  executed.
- **Run**: compiled with Apple clang 21 on macOS arm64 and executed on the
  native unit-test framework of each firmware source
  (`builddefs/build_test.mk`, called the way this repository's `Makefile`
  does). The vial run used a copy of the checkout, so the shared one was not
  touched.

Nothing was compiled for a keyboard: no `avr-gcc` or `arm-none-eabi-gcc` was
available. No hardware and no Vial build were exercised. See
[Not verified](#not-verified).

### What was run

| What | qmk | vial |
| --- | --- | --- |
| `users/hydeik_vial/tests/smart_mod` at `9251386`, unchanged | 89 of 89 pass | 89 of 89 pass |
| Probe, tap-hold options all off (`split_default`, trigger keycodes from `QK_KB_0`) | 5 of 5 pass | 5 of 5 pass |
| Probe, Hold On Other Key Press per key (`split_hold_on_other_key`, trigger keycodes from `QK_USER`) | 5 of 5 pass | 5 of 5 pass |
| Probe, the tap-hold options of `users/hydeik/config.h` plus tri-layer (`split_hydeik_like`) | 7 of 7 pass | 7 of 7 pass |

The `smart_mod` suite is relevant beyond itself: it is an own state machine
that already depends on layer-tap tap counts, `layer_on` / `layer_off`, Layer
Lock, combos, tap dance and `housekeeping_task_user`, and it has only ever been
run on vial. It passes on qmk without a change to source or tests.

The probe is in
[`split-trigger-hold-cross-firmware-probe/`](split-trigger-hold-cross-firmware-probe/)
on this branch: about 100 lines of C (`src/split_probe.c`) taking a table of
`{holding key, trigger key, target layer}` from its caller, plus a glue file
standing in for a keymap family. It is not the behaviour spec and not
production code; it does only enough to exercise the firmware APIs. Its tests
cover: target layer on while the holding key is held; target layer kept for one
key after a quick release; the trigger key again cancels; Layer Lock on the
target layer survives the release of the holding key; the fast roll within the
tapping term; a thumb holding key under Chordal Hold; tri-layer in
`layer_state_set_user` alongside.

To run one configuration again, from a firmware checkout at its pin:

```sh
make -r -R -f builddefs/build_test.mk \
    TEST=split_default TEST_OUTPUT=probe_split_default \
    TEST_PATH=<this repo>/docs/research/split-trigger-hold-cross-firmware-probe/split_default \
    FULL_TESTS=split_default ALLOW_WARNINGS=yes EXTRAFLAGS=-Wno-c++11-narrowing
.build/test/probe_split_default.elf
```

## 1. How a held or tapped layer-tap key reaches `process_record_user`

### The decision code is the same

`diff` of `quantum/action_tapping.c` between the two refs shows only
(**read**):

- Speculative Hold, added upstream: `SPECULATIVE_HOLD`,
  `speculative_key_press`, `speculative_key_settled`
  (`qmk:quantum/action_tapping.c:56-69,151-156,743-892`), with a new bit
  `tap.speculated` where vial has `reserved2` (`qmk:quantum/action.h:41`,
  `vial:quantum/action.h:41`). It is compiled in only with
  `#define SPECULATIVE_HOLD`, which `users/hydeik/config.h` does not set, and
  it only ever acts on mod-tap keys (`qmk:quantum/action_tapping.c:782-785`).
- `within_flow_tap_term()`, a new helper (`qmk:quantum/action_tapping.c:1074`).
- Re-indentation of one block and whitespace.

`process_tapping()` itself (`qmk:quantum/action_tapping.c:255`,
`vial:quantum/action_tapping.c:223`) is otherwise line for line the same. The
ways a layer-tap key gets settled, with the line in each source:

| Settled as | When | qmk | vial |
| --- | --- | --- | --- |
| tap (`tap.count == 1`) | released within the tapping term | `:315` | `:283` |
| hold (`tap.count == 0`) | tapping term ends | `:549` | `:517` |
| hold | another key pressed, Hold On Other Key Press | `:472` | `:440` |
| hold | another key pressed and released, Permissive Hold | `:374` | `:342` |
| tap | same-hand chord, Chordal Hold | `:353`, `:463` | `:321`, `:431` |

In every case the layer-tap key reaches `process_record()` first, and the keys
pressed after it, which waited in the waiting buffer, follow in their original
order with their original `event.time`.

The order of `process_record_quantum()` around `process_record_user` is also
the same (**read**; `qmk:quantum/quantum.c:296-448`,
`vial:quantum/quantum.c:300-457`): tap dance pre-processing, Repeat Key,
community modules, `process_record_kb` / `_user`, then Caps Word, Key
Override, Tap Dance, Tri Layer, Layer Lock. vial inserts one call,
`process_record_vial`, right after `process_record_kb`
(`vial:quantum/quantum.c:369`); it only finishes Vial tap dances early
(`vial:quantum/vial.c:582-606`) and always returns true.

So in both sources (**hand-traced**, and confirmed by the probe **run**):

- A held holding key arrives as a press with `tap.count == 0`. Returning true
  lets QMK switch the layer of the layer-tap key on and, at the release, off
  (`qmk:quantum/action.c:637,717-718,730-731`,
  `vial:quantum/action.c:629,709-710,722-723`).
- A tapped holding key arrives as a press and a release with `tap.count == 1`.
- The trigger key is looked up when it is *processed*, not when it is pressed:
  `process_record_quantum()` calls `get_record_keycode(record, true)`, which
  recomputes the layer for a press (`qmk:quantum/quantum.c:243-274,297`,
  `vial:quantum/quantum.c:251-282,301`). A trigger key pressed before the
  holding key is settled is therefore still found on the holding key's layer
  once the hold settles. The probe test `FastRollWithinTheTappingTerm` shows
  it.
- The release of a tap-hold key does not always carry the tap count of its
  press (the Chordal Hold / Flow Tap bookkeeping at
  `qmk:quantum/action_tapping.c:258-270`, `vial:...:226-238`), which
  `smart_mod` already works around by remembering the key position. The same
  applies to a holding key, identically in both sources.
- `post_process_record_user` is not called for a key whose
  `process_record_user` returned false (`qmk:quantum/action.c:293-303`,
  `vial:quantum/action.c:289-299`), again identically.

### What differs: who decides the tap-hold options

This is the difference that matters to the user, though not to the code of the
state machine.

**`hydeik` (qmk): compiled in.** `users/hydeik/config.h` sets
`TAPPING_TERM 250`, `PERMISSIVE_HOLD`, `CHORDAL_HOLD`, `FLOW_TAP_TERM 125` and
per-key Hold On Other Key Press; `users/hydeik/hydeik.c` returns true from
`get_hold_on_other_key_press` for `SYM_TAB`, `NAV_SPC` and `NUM_ENT`. So a
holding key is settled as held the moment the trigger key is pressed
(**hand-traced**; the `split_hydeik_like` probe **run** shows the fast roll
ending on the target layer). Two things that could have interfered do not:

- Chordal Hold is evaluated before Hold On Other Key Press
  (`qmk:quantum/action_tapping.c:455-471`) and would turn a same-hand roll into
  a tap. But `get_chordal_hold_default()` returns true whenever the tap-hold
  key's hand is `'*'` (`qmk:quantum/action_tapping.c:940-951`), and the common
  core's layout adapters mark every thumb key `'*'`
  (`users/hydeik_common/keyboards/mothwing.h`, `crkbd_rev4_1_mini.h`). The
  probe test `ChordalHoldLeavesAThumbHoldingKeyAlone` confirms it (**run**).
  A holding key that is not a thumb key would not be exempt.
- Flow Tap: `get_flow_tap_term` in `users/hydeik/hydeik.c` returns 0 for every
  key but the home-row mod-taps, so it never applies to a thumb layer-tap key
  (**hand-traced**).

**`hydeik_vial` (vial): runtime data.** As the earlier research found, a
default Vial build defines the per-key callbacks itself and answers them from
QMK Settings in EEPROM (`vial:quantum/qmk_settings.c:288-347`). For the
holding keys this means:

- Permissive Hold, Hold On Other Key Press and Chordal Hold are each one global
  bit (`vial:quantum/qmk_settings.c:69-73`,
  `vial:quantum/qmk_settings.h:159-166`), not per key. The tapping term is one
  global value, `QS.tapping_term`, returned by Vial's `get_tapping_term`
  (`vial:quantum/vial.c:528-542`).
- The factory default is all bits off and the tapping term of `config.h`
  (`vial:quantum/qmk_settings.c:209-211`).
- With all bits off, the fast roll (holding key down, trigger key down and up,
  holding key up, all within the tapping term) is a *tap* of the holding key
  followed by the key under the trigger key on the base layer. The
  `split_default` probe **run** shows exactly that: Enter and `a` are typed and
  no layer is switched. The reference behaviour ("release the holding key
  within the One Shot Term and the target layer stays for one key") is then
  reachable only by waiting out the tapping term before the trigger key.
- Every newly built Vial firmware resets these settings (**hand-traced**, not
  observed on hardware): the build writes a random `BUILD_ID`
  (`vial:util/build_id.py:5`, `vial:builddefs/build_keyboard.mk:296`), VIA
  takes it as its EEPROM magic (`vial:quantum/via.c:86-90`), a mismatch at
  boot runs `eeconfig_init_via()` (`vial:quantum/via.c:137-153`), and
  `dynamic_keymap_reset()` ends in `qmk_settings_reset()`
  (`vial:quantum/dynamic_keymap.c:142`). The same reset puts the keymap of
  `keymap.c` back and clears the tap dances and combos.
- `QS` and `qmk_settings_set()` are public
  (`vial:quantum/qmk_settings.h:152,156`), so `hydeik_vial` code could choose
  its own defaults. That would be family code, never the common core.

The state machine must not read `TAPPING_TERM` directly; `GET_TAPPING_TERM()`
gives the right value in both (`qmk:quantum/action_tapping.h:221-225`,
`vial:quantum/action_tapping.h:179-183`), as `smart_mod` already does. Its own
terms (a One Shot Term) are its own macros.

## 2. The layer API and Layer Lock

**Read:** `quantum/action_layer.h`, `quantum/layer_lock.c`,
`quantum/layer_lock.h` and `quantum/process_keycode/process_layer_lock.c` are
byte-identical in the two sources. `quantum/action_layer.c` differs only in
two debug format strings and in vial skipping its magic key position in the
source-layer cache (`vial:quantum/action_layer.c:284-286,305-307`).

- `layer_on`, `layer_off`, `layer_state_is`, `layer_state_set` and the
  `layer_state_set_user` hook behave the same
  (`qmk:quantum/action_layer.c:132-195`, `vial:quantum/action_layer.c:136-199`).
- Layer Lock is on in both: `users/hydeik_common/common.mk` sets
  `LAYER_LOCK_ENABLE ?= yes`, and vial's `build_vial.mk` defaults to it as
  well. `QK_LAYER_LOCK` is `0x7C7B` in both (`qmk:quantum/keycodes.h:855`,
  `vial:quantum/keycodes.h:772`, pinned by
  `vial:quantum/vial_ensure_keycode.h:823`).
- `process_layer_lock()` runs *after* `process_record_user` in both
  (`qmk:quantum/quantum.c:438`, `vial:quantum/quantum.c:447`), so the state
  machine sees `QK_LLCK` and the release of a holding key before Layer Lock
  acts on them.

Behaviour the spec has to reckon with, identical in both
(`quantum/process_keycode/process_layer_lock.c`, **read**):

- `QK_LLCK` locks the *highest* layer that is on (`:44-46`). With the target
  layer on above the holding key's layer, that is the target layer.
- A locked layer that anything else switches off is silently unlocked at the
  next key event (`:40-42`). A state machine that calls `layer_off(target)` on
  the release of the holding key would undo a lock; it has to ask
  `is_layer_locked()` first (`quantum/layer_lock.c:40`). The probe does, and
  `LayerLockKeepsTheTargetLayer` passes on both (**run**).
- The release of a held layer-tap key whose layer is locked is swallowed, so
  that layer stays on (`:72-77`). This covers the holding key's own layer
  without any help from the state machine.

`is_layer_locked()` exists only when Layer Lock is compiled in, so the common
core guards it with `#ifdef LAYER_LOCK_ENABLE`: a feature switch, as
`common.c` already does for Caps Word, not a branch on the firmware source.

## 3. Tri-layer

**Read:** `update_tri_layer_state()` is the same function in both
(`qmk:quantum/action_layer.c:373-377`, `vial:quantum/action_layer.c:385-389`),
and so are `quantum/tri_layer.c` and `process_tri_layer.c`.

Only `hydeik` uses it, from `layer_state_set_user`
(`update_tri_layer_state(state, _SYM, _NUM, _FUN)` in `users/hydeik/hydeik.c`).
`hydeik_vial` defines no `layer_state_set_user`, and neither does Vial.

Two consequences (**hand-traced** from the one-line body
`(state & mask12) == mask12 ? (state | mask3) : (state & ~mask3)`):

- The hook runs on every layer change, including the state machine's own
  `layer_on(target)`, and leaves other layers alone. Tri-layer and the state
  machine coexist: the probe test `TriLayerCoexists` passes on both (**run**).
- The third layer is *forced off* whenever the first two are not both on. As
  long as `hydeik` keeps this hook, `_FUN` cannot be a target layer: a
  `layer_on(_FUN)` by the state machine is undone within the same call. Any
  change to how `hydeik` reaches FUN has to remove or change the hook first.

The common core must not define `layer_state_set_user`: that hook belongs to
the family.

## 4. Custom keycode ranges

**Read:** the ranges are the same numbers in both sources
(`quantum/keycodes.h:91-94` in both): `QK_KB` is `0x7E00 .. 0x7E3F`,
`QK_USER` is `0x7E40 .. 0x7FFF`, and `SAFE_RANGE` is `QK_USER`
(`quantum/quantum_keycodes.h:37` in both). vial pins `QK_KB` with static
asserts (`vial:quantum/vial_ensure_keycode.h:826-827`). Both ranges reach
`process_record_user` the same way; the probe ran with trigger keycodes from
`QK_KB_0` in one configuration and from `QK_USER` in the others.

The difference is in the Vial GUI, not the firmware (**read**, vial-gui):

- The GUI offers 64 user keycodes `USER00 .. USER63`, numbered `QK_KB + x`
  (`vial-gui:src/main/python/keycodes/keycodes_v6.py:568,607-608`).
- Entry `i` of `customKeycodes` in `vial.json` gives `USER<i>` its name and
  title (`vial-gui:src/main/python/keycodes/keycodes.py:833-841`). The link
  is by position only.
- A keycode the GUI does not know, such as anything in `QK_USER`, is shown and
  saved as a bare hex number (`vial-gui:.../keycodes.py:108-124`). It still
  works in the firmware, and can be typed in through the "Any" key, but it has
  no name and no place on the GUI's keycode tabs.

So for a trigger key to be placeable and stable in `hydeik_vial`:

- Its keycode comes from `QK_KB_0 ..`, appended after `COS_RGUI`
  (`QK_KB_0 + 9` onward) so that the nine existing numbers stay as ADR 0002
  requires.
- `customKeycodes` in every `vial.json` of the family, the enum in
  `users/hydeik_vial/layout.h` and the copy in the test glue get the same
  entries in the same order, as the comment in `layout.h` already demands for
  the one-shot mod keys.
- The number, once released, is never reused: keymaps saved by Vial (`.vil`
  files) hold it.
- Layer numbers are in the same position: a holding key is `LT(layer, kc)`,
  and its number contains the layer. Reordering layers changes what stored
  keymaps mean.

`hydeik` has no such constraint: its `enum custom_keycodes` starts at
`QK_USER`, nothing stores the numbers, and new entries can go anywhere.

A consequence for the interface: the common core cannot own the trigger
keycodes. Whatever enum it defined would be right for at most one family.

One alternative the spec may weigh (**hand-traced**, not probed): the trigger
key need not be a custom keycode at all. Any keycode reaches
`process_record_user` before QMK acts on it, so a family could pass an
existing layer keycode such as `MO(target)` as the trigger key and have the
state machine take it over. In Vial that key is placeable with no
`customKeycodes` entry. The cost is that the same keycode can then no longer
be used as a plain `MO` while its holding key is held.

## 5. Combos and tap dance

### Processing order

Identical in both (**read**):

- Combos run before tap-hold, in `pre_process_record_quantum()`
  (`qmk:quantum/quantum.c:277-283`, `vial:quantum/quantum.c:285-291`), on the
  keycode the key has *at the moment it is pressed*. A key that is part of a
  combo is held back for the combo term and never reaches
  `process_record_user` if the combo fires.
- A combo's result is sent as a new event carrying the keycode in
  `record->keycode`, with event type `COMBO_EVENT` and key position (0, 0)
  (`quantum/keyboard.h:87` in both; `qmk:quantum/process_keycode/process_combo.c:155-156`,
  `vial:...:178-179`). It goes through tap-hold and `process_record_user` like
  a key, so a combo whose result is a holding key or a trigger keycode works.
- Tap dance keys are swallowed by `process_tap_dance()` after
  `process_record_user` (`qmk:quantum/quantum.c:393`,
  `vial:quantum/quantum.c:402`); the state machine sees the `TD()` keycode
  itself, not what it produces.

`quantum/process_keycode/process_combo.c` differs by Vial's GUI combos (below)
and by an upstream fix for one-key combos
(`qmk:quantum/process_keycode/process_combo.c:349-351`); neither changes what
`process_record_user` sees for the keys of this design.

### What differs

- **Where the tables come from.** In `hydeik_vial` the combo and tap dance
  tables are Vial's, filled from EEPROM (`vial:quantum/vial.c:515,556`); in
  `hydeik` there are none (`COMBO_ENABLE` and `TAP_DANCE_ENABLE` are not set
  for it). The common core must define neither table.
- **What a Vial tap dance produces** (`vial:quantum/vial.c:332-356`,
  **read**). A result that is a basic or modified keycode
  (`<= QK_MODS_MAX`) is sent with `register_code16()` and never reaches
  `process_record_user`. Any other result, for example a layer-tap or a custom
  keycode, is injected as a key event at the magic position (240, 240) and
  does go through tap-hold and `process_record_user`. Upstream tap dances
  call the user's own functions instead.
- **The tap dance C API** changed upstream: the state moved out of
  `tap_dance_action_t` and is fetched with `tap_dance_get_state()`
  (`qmk:quantum/process_keycode/process_tap_dance.h:40,102` versus
  `vial:quantum/process_keycode/process_tap_dance.h:38-39`). Code that reads
  `action->state` compiles on vial only. The state machine has no reason to
  touch it; test tables written with `ACTION_TAP_DANCE_DOUBLE` compile on
  both, as the `smart_mod` run shows.

### Two rules that follow for the state machine

Both are needed for `hydeik_vial` and cost nothing in `hydeik`
(**hand-traced**):

1. **A key position does not always identify a physical key.** Combo results
   all share (0, 0), which is also a real matrix position, and Vial tap dance
   results all share (240, 240). `KEYEQ` compares row and column only
   (`quantum/keyboard.h:46`). Remembering a held holding key by position, as
   `smart_mod` does, stays correct only if the event type is compared as well,
   or if such events are not accepted as holding keys.
2. **A typed key may never be seen.** A key swallowed by a combo, and a tap
   dance result sent with `register_code16()`, do not reach
   `process_record_user`. A target layer that is on "for the next key press
   only" is not consumed by such a key. `smart_mod` has the same gap for
   pending one-shot mods and covers part of it in `housekeeping_task_user`.

### Repeat Key

Not asked, but found on the way and the same in both: `process_last_key()`
runs before `process_record_user` (`qmk:quantum/quantum.c:343`,
`vial:quantum/quantum.c:352`) and remembers every key except modifiers and the
built-in layer keys (`quantum/process_keycode/process_repeat_key.c:26-60`,
identical). A custom trigger keycode would become the last key, and Repeat Key
would replay it. `remember_last_key_user` is defined in the common core
(`users/hydeik_common/common.c`), so the state machine's "is this a trigger
key" answer has to be reachable from there.

## 6. The unit-test framework

Both refs ship the same framework with the same entry point, and this
repository's `Makefile` recipe works against either (**run**): it calls
`builddefs/build_test.mk` with `TEST_PATH` pointing at a directory outside the
firmware checkout, which includes that directory's `test.mk`
(`builddefs/build_test.mk:43,58` in both) and honours
`INTROSPECTION_KEYMAP_C` (`qmk:builddefs/build_test.mk:81-82`,
`vial:builddefs/build_test.mk:80-81`).

| Aspect | qmk | vial |
| --- | --- | --- |
| GoogleTest | 1.16.0 (`lib/googletest` at `6910c9d`) | 1.11.0 (`lib/googletest` at `e2239ee`) |
| C++ standard | `gnu++14` (`builddefs/common_rules.mk:38`) | the same |
| `TestFixture`, `KeymapKey`, `TestDriver`, `EXPECT_REPORT` and friends | present | present, same interface |
| `KeyboardReport(...)` matcher | also takes a `std::vector<uint8_t>` (`tests/test_common/keyboard_report_util.hpp:36-38`) | variadic form only |
| Fixture teardown | `NiceMock<TestDriver>` (`tests/test_common/test_fixture.cpp:68`) | plain `TestDriver` |
| Shipped tests for `layer_lock`, `tri_layer`, `tap_hold_configurations/{chordal_hold,hold_on_other_key_press,permissive_hold,flow_tap,...}` | present | identical files |
| `tap_hold_configurations/speculative_hold` | present | absent |
| Vial itself (`VIAL_ENABLE`, QMK Settings, GUI combos and tap dances) | not applicable | not built by the test framework |

Consequences:

- Test sources written against the older framework (vial) compile on the
  newer one. Writing to vial's interface keeps them portable; the vector form
  of `KeyboardReport` is the one thing found that would not go back.
- One set of test sources can therefore run against both firmware sources.
  Where the tests of a common-core state machine live, and whether `make test`
  runs them against one source or both, is open on the map; nothing in the
  frameworks forces the choice.
- The tests never contain Vial. The `hydeik_vial` tap-hold behaviour that
  depends on QMK Settings can only be imitated by compile-time options in a
  test `config.h`, as the probe's three configurations do, and Vial's tap
  dance injection at (240, 240) cannot be reproduced there at all.
- Apple clang needs `ALLOW_WARNINGS=yes EXTRAFLAGS=-Wno-c++11-narrowing` for
  both sources, as the `Makefile` already passes.

## What each keymap family passes in

The seam that worked in the probe has the shape of `smart_mod`'s: the common
core holds the state machine and declares what it must be told; each family
answers and forwards the QMK hooks. Nothing in it is conditional on the
firmware source.

**Told by the family** (a table or callbacks; the probe used a table):

| | `hydeik` (qmk) | `hydeik_vial` (vial) |
| --- | --- | --- |
| Holding keys | layer-tap keycodes of `hydeik.h`; starting hypothesis `NAV_SPC` | `TAB_SYM`, `ENT_NUM` of `layout.h` |
| Trigger keycodes | new entries of `enum custom_keycodes` (`QK_USER` range), or an existing layer keycode | new entries after `COS_RGUI` (`QK_KB_0 + 9 ..`), with matching `customKeycodes` in every `vial.json`; or an existing layer keycode |
| Target layers | `_MOU` (hypothesis) | `_NAV`, `_FUN` |
| Timing (One Shot Term and the like) | macros in the family's `config.h`, defaults in the common core | the same |

**Forwarded by the family:** `process_record_user`,
`post_process_record_user` and `housekeeping_task_user`. Each family already
owns these hooks (`hydeik.c` has `process_record_user`; `hydeik_vial.c` has
all three for `smart_mod`), so the state machine is called from them and never
defines them. The order of the calls inside `hydeik_vial`'s hooks, relative to
`smart_mod`, is that family's decision and part of the behaviour tickets.

**Staying in the family, outside the state machine:**

- `hydeik`: the tap-hold callbacks (`get_hold_on_other_key_press` and the
  rest) and `layer_state_set_user` with its tri-layer.
- `hydeik_vial`: the `customKeycodes` of `vial.json`, the stability of the
  keycode and layer numbers, any firmware-side default for Vial's tap-hold
  settings, and the `TO()` keys that the common grids take through their `BR`
  parameter.

**Rules for the common-core implementation**, all met by the probe except the
last two, which it did not need:

- Use `GET_TAPPING_TERM()`, never `TAPPING_TERM`.
- Use nothing upstream added after vial's last merge (Speculative Hold,
  `within_flow_tap_term`, `tap_dance_get_state`, `process_oneshot`).
- Guard `is_layer_locked()` with `#ifdef LAYER_LOCK_ENABLE`.
- Define no QMK hook and no keymap table.
- Compare the event type, not only the position, when remembering a held key.
- Let `remember_last_key_user` in `common.c` ask the state machine about
  trigger keys.

## Not verified

- **No keyboard firmware was built.** Whether the state machine fits is
  unknown. The earlier research recorded 128 bytes of free flash on the
  ATmega32U4 `mothwing` for `hydeik` at commit `06a85c3`; that figure was not
  re-measured, and `smart_mod` is not part of that build. A second state
  machine plus a table in the common core may not fit `mothwing:hydeik`
  without freeing space. This is the largest open risk to "both families get
  the same behaviour".
- **No Vial build and no hardware.** That a new firmware resets QMK Settings
  and the stored keymap is traced through the source, not observed. So is
  everything said about Vial tap dance results and GUI combos: the test
  framework builds neither.
- **vial-gui** was read at the head of `main` on the day, not at a commit this
  repository pins; a released Vial GUI may differ.
- The probe exercises only thumb holding keys, key events from the matrix, and
  three tap-hold configurations. Retro Tapping, Quick Tap Term on a holding
  key, Flow Tap on a holding key, encoder events and Caps Word were not
  probed.
- The probe's behaviour is a guess at the reference, good enough to drive the
  APIs. It settles nothing about the behaviour spec, and its one-shot handling
  has the unseen-key gap described in section 5.
- Whether `MO(target)` as a trigger key interacts badly with Layer Lock's own
  handling of `MO` keys (`process_layer_lock.c:52-53`, after
  `process_record_user`) was not traced further than noting that a state
  machine returning false for the key keeps Layer Lock from seeing it.
