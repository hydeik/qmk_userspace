# One Shots on Steroids: split trigger/hold for layers, from source

Research for issue #36 (map: issue #35). It states how the One Shots on Steroids (OSoS) community module behaves when `OS_STEROIDS_SPLIT_TRIGGER_HOLD` is defined and an entry of `oneshot_os[]` has a **layer**. It builds on the research for issue #24 (`docs/research/oneshots-on-steroids-behaviour.md` on the branch `research/oneshots-on-steroids-behaviour`), which covers the modifier side and the module in general; that part is not repeated here.

It describes the reference. It does not decide what this repository's own implementation should do.

## Sources

| Tag | Source | Revision |
| --- | --- | --- |
| `c` | [`oneshots_on_steroids.c`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids.c) | `Kawamashi/qmk_modules` `main` at `d8dcb1351910ee003428c38768ae7c026d82c9ff` (2026-11-06), the same commit as the research for issue #24 |
| `h` | [`oneshots_on_steroids.h`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids.h) | same |
| `u` | [`oneshots_on_steroids_utils.c`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids_utils.c) | same |
| `r` | [`readme.md`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/readme.md) | same |
| `vq:<file>` | [`vial-kb/vial-qmk`](https://github.com/vial-kb/vial-qmk/tree/dd43959ae5c08d8a28d38a1acf7b04e86b14a344), files under `quantum/` | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344`, the pin of `hydeik_vial` in `firmware_sources.json` |
| `repo` | this repository | `9251386` |

A citation such as `c:307` means line 307 of that file at that revision.

Confidence labels:

- **Read** means the statement is what the cited lines say.
- **Trace** means it was derived by following several cited code paths by hand.
- **Nothing was compiled, run, or tried on hardware.** There is no "run" evidence in this document. The module was cloned and read only. Core QMK was read at the vial-qmk pin only, not at the upstream QMK pin of the `hydeik` family.

## Notation

One entry, in the readme's form `{OS(T, H, 0, L)}` (`r:345-356`):

- `T`: the trigger key, a keycode.
- `H`: the holding key, a keycode. The source calls it the `suppressor` (`h:28, 40`). In the readme's example it is a layer-tap key, `LT(A, tap)`.
- `A`: the layer `H` holds. The module does not know `A`; it is not part of the entry.
- `L`: the target layer, the entry's `layer`.
- "Regular key": a key press that is neither a cancel key nor ignored (rule 6).
- Default callbacks and no other optional flag, unless one is named.

## 1. Short answer

- The flag changes only the `OS()` macro (`h:59-63`). There is no separate code for split entries: the trigger **press** activates the entry, the holding key **release** runs the logic that the release of an ordinary OSoS key runs, and the trigger release does nothing.
- The module never looks at whether the holding key is held. It never sees its press; it only matches its release, by keycode, without regard to tap or hold. "The holding key must be held" is true only because of where the trigger sits in the keymap.
- The One Shot Term runs from the moment the trigger press is processed to the moment the holding key release is processed. Releasing the holding key inside it, with no regular key pressed since the trigger, leaves the target layer on for the next key press. Any other release of the holding key switches the target layer off.
- A trigger pressed while its holding key is not held switches the target layer on and leaves it on, with no timeout, until the trigger is pressed again, a cancel key is pressed, or the holding key is next released.
- The module keeps no layer stack. It calls `layer_on(L)` and `layer_off(L)` on QMK's layer bitmask; the layer of the holding key goes off through the holding key's own QMK action.
- One entry has one holding key. Two entries that share a trigger do not give "one trigger, two holding keys"; they desynchronise (rule 9).

## 2. Rules

### Rule 1. What the flag changes

- Read: without the flag, `OS(key, mods, layer)` expands to `key, key, mods, layer`; with it, `OS(key1, key2, mods, layer)` expands to `key1, key2, mods, layer` (`h:59-63`). Nothing else in the three source files tests the flag.
- Read: a press is compared with `trigger` only (`c:391`), a release with `suppressor` and with `trigger` (`c:404, 406`). For an ordinary entry both are the same keycode; that is the whole difference.

### Rule 2. The trigger press activates, whatever the holding key is doing

- Read: on a press of `T` with the entry idle, the module calls `layer_on(L)`, records `oneshot_tap_time = timer_read()`, and sets the state to `os_down_unused` (`c:259, 263, 304-308`). No condition involves the holding key.
- Read: the press is swallowed (`c:394`): it reaches no later handler and performs no QMK action. The release of `T` is swallowed too and changes nothing (`c:399-402`, `c:406`).
- Trace: because the release of `T` has no effect, it does not matter whether `T` is tapped or kept down. Keeping `T` down does not keep the layer.

### Rule 3. What counts as the holding key being held

- Read: the module has no state for "holding key down". It reacts to the holding key in exactly one place, a release event whose keycode equals `suppressor`, and only while the entry is not idle (`c:397-404`). The comparison does not look at `record->tap.count`.
- Trace, consequences:
  - A **hold** of `LT(A, tap)` is what makes `T` reachable at all when `T` sits on layer `A`. This gate is the keymap, not the module.
  - The **tap** release of the same layer-tap key also counts as the release of the holding key. It matters when the entry is active without the holding key held (rule 7): tapping `H` then ends it. The press of that tap is a regular key (`u:158-159, 171`), so the entry is already `os_down_used` and the release deactivates it (`c:333-338`).
  - The release of `H` is matched by keycode. QMK resolves the keycode of a release from the layer the key was pressed on (`vq:quantum.c:266-282`, `vq:action_layer.c:330-344`), so a holding key pressed before the trigger is still recognised after the target layer has covered its position.
- Read: the release of `H` is not swallowed (the first loop sets `should_continue_processing = false` only for triggers, `c:394, 401, 406`), so QMK then runs the layer-tap's own release and switches `A` off (`vq:action.c:721-723`).
- Trace: the trigger is seen by the module only when QMK processes it, which is after the holding key's tap-hold decision. Whether a quick "holding key down, trigger down" counts as a hold is decided by QMK's tap-hold settings, not by the module. The module runs in `process_record` (`vq:quantum.c:363`), and an event buffered behind an undecided tap-hold key gets its keycode from the layer state at the time it is finally processed (`vq:action_tapping.c:131-160`, `vq:quantum.c:301`, `vq:quantum.c:266-282`).

### Rule 4. The One Shot Term

- Read: the start is `timer_read()` when the trigger press is processed (`c:307`), not `record->event.time` and not the press of the holding key. The end is `timer_elapsed()` when the holding key release is processed (`c:318`).
- Read: the one-shot is queued only if the state is still `os_down_unused` and the elapsed time is below the term (`c:318`). Otherwise the entry is deactivated (`c:333-338`).
- Read: the term is `get_oneshot_on_steroids_term(keycode, record)` where `keycode` is the released key, that is, the **holding key**, and `record` is its release (`c:318`, `c:404`). The default returns `OS_STEROIDS_TERM`, which defaults to `TAPPING_TERM` (`u:206-212`, `h:65-67`).
- Trace:
  - How long the holding key was held before the trigger is irrelevant. Hold `H` for ten seconds, tap `T`, release `H` at once: the one-shot is queued.
  - If the trigger press was buffered behind the holding key's tap-hold decision, the clock starts when the decision is made, not at the physical press.
  - A per-key term has to be keyed on the holding key in split mode. A `case` on the trigger, as in the readme's example (`r:124-134`), is never reached for a split entry.
  - Both timer values are 16 bit, so the elapsed time wraps after about 65.5 s. A holding key released that long after the trigger can be read as "inside the term". This is true of every OSoS key, not only split ones.

### Rule 5. Releasing the holding key

- Read, inside the term and unused: state `os_up_queued`; nothing is done to `L`, so it stays on (`c:322-328`). The idle timer is armed with the timeout of the **trigger** (`c:329-332`), counted from the event time of the holding key release (`c:142-149`). Then QMK switches `A` off (rule 3).
- Read, after the term or after a regular key: `deactivate_oneshot_on_steroids` calls `layer_off(L)` (`c:224-225`, `c:333-338`). Then QMK switches `A` off.
- Trace: in the second case two `layer_state_set` calls happen during one key event, first without `L`, then without `A` (`vq:action_layer.c:136-149, 191-201`). A `layer_state_set_user` hook sees the intermediate state.

### Rule 6. What ends the one-shot (state `os_up_queued`)

- **Next regular key press.** Read: the state becomes `os_finished` during that key's `process_record` and the entry is deactivated in the module's `post_process_record` hook, which calls `layer_off(L)` (`c:364-373`, `c:440-452`). Trace: the key itself is resolved with `L` still on; its release uses the layer it was pressed on (`vq:action_layer.c:330-344`); a second key rolled over it is resolved without `L`.
- **Which presses are regular for a layer entry.** Read (`u:141-184`): everything except modifier keys. In particular the layer keys are **not** ignored: a held layer-tap, `MO`, `LM`, `TT`, `OSL`, `TO`, `TG`, the Tri Layer keys, and any other OSoS layer trigger (`u:144, 158-168, 174-176`). A tapped layer-tap or mod-tap is regular (`u:149, 159`). Custom keycodes are regular (`u:171`).
- **Ignored presses.** Read: `KC_LCTL` to `KC_RGUI`, `KC_HYPR`, `KC_MEH`, `OSM()`, a held mod-tap, and a modifier-only OSoS key (`u:143, 148-155, 183`). They leave the one-shot queued and restart the idle timer if it is running (`c:349-354`). With `OS_MOD_SHOULD_LEAVE_OS_LAYER` a modifier-only OSoS key is not ignored (`u:178-181`).
- **Another OSoS trigger that is not ignored.** Read: the queued entry is deactivated at once, inside that press, instead of going through `os_finished` (`c:365-369`). The other entry was already activated by the first loop (`c:387-395`).
- **The trigger again.** Read: a press of `T` while the entry is not idle deactivates it (`c:309-313`); the following release of `T` is swallowed (`c:399-402`).
- **Cancel key.** Read: `is_oneshot_on_steroids_cancel_key` (default: no key, `u:133-139`) deactivates the entry on press (`c:343-347`). The cancel key is not swallowed. Trace: its keycode was resolved before the module ran, so it is the key at that position with `L` on.
- **Timeout.** Read: the timer is armed only on entering `os_up_queued` (`c:329-332`), there is one for the whole module (`c:129-130`), and its expiry deactivates every entry (`c:135-139`, `u:69-73`). Default 5000 ms (`h:69-71`).
- **Holding key pressed again.** Trace: as a hold it is a layer key, hence regular; when QMK resolves it as a hold, `A` goes on and `L` goes off in the same event, so the one-shot is spent on it and the next key comes from `A`. As a tap it is an ordinary regular key: its tap keycode is sent and `L` goes off.
- **A key that a later handler stops.** Trace: QMK calls the post-process hooks only when the whole `process_record` chain returned `true` (`vq:action.c:289-299`). If the key that uses the one-shot is handled by something after the module that returns `false` (a custom keycode in `process_record_user`, for example), the entry stays `os_finished` with `L` on until the next event that passes the whole chain. A press in `os_finished` changes nothing (`c:357-376`), so that next key is still resolved on `L`.

### Rule 7. The trigger pressed without its holding key

- Read: activation is unconditional (rule 2), so `L` goes on and the state is `os_down_unused`.
- Trace:
  - The release of `T` does nothing. No timer is armed, because that happens only on entering `os_up_queued`.
  - Regular keys only move the state to `os_down_used` (`c:358-363`). `L` stays on for any number of keys.
  - It ends on: a second press of `T`; a cancel key; the next release of `H`, tap or hold, whenever that happens; the expiry of a timer armed by another entry; one of the API functions (`u:64-79`).
  - In effect the trigger alone is a layer toggle that the holding key's next release switches off.
- How the trigger becomes reachable without the holding key: layer `A` is on by other means (`TO`, `TG`, Layer Lock, a second key that holds `A`), or the trigger also sits on a layer that is on. One case is built into the gesture itself: the one-shot state of another entry (rule 9, chained triggers).

### Rule 8. The same key at the same position on the target layer

- Read: cancelling is rule 6, "the trigger again": any press of the keycode `T` while the entry is not idle, in `os_down_unused`, `os_down_used` and `os_up_queued` alike (`c:309-313`). The module does not look at the position or the layer of the key.
- Trace, what the keymap must provide:
  - While `H` is held, `A` is still on underneath `L`. A transparent key at that position on `L` falls through to `T` on `A` (`vq:action_layer.c:354-372`), so it cancels as well.
  - In the one-shot state `A` is off. Then only a real `T` on `L` cancels; a transparent key falls through to the default layer and is a regular key that uses the one-shot.
  - Any other key at that position on `L` is a regular key.
- Trace, after the cancel: with `H` still held, `L` is off and `A` is on, so the user is back on the holding key's layer; this is the readme's statement (`r:369`). The entry is idle, so a third press of `T` activates it again, and the later release of `H` does nothing in the module (`c:399-403`) and switches `A` off in QMK. From the one-shot state the cancel leaves only the default layer.

### Rule 9. Several triggers, several holding keys

- **Several triggers, one holding key** (entries sharing `suppressor`). Read: each entry has its own state and tap time (`c:26-27`), and a release of the holding key is applied to every non-idle entry that names it (`c:387-408`). Trace: hold `H`, tap `T1` (layer `L1` on), tap `T2` (layer `L2` on; entry 1 becomes `os_down_used` because an OSoS layer trigger is a regular key for it). Release `H`: `L1` goes off; `L2` is queued as a one-shot if the release is inside the term counted from `T2`, otherwise it goes off too.
- **One trigger, several holding keys.** Not supported: an entry has one `suppressor` (`h:38-43`). Trace of the obvious workaround, two entries `{T, H1, 0, L}` (lower index) and `{T, H2, 0, L}`:
  - A press of `T` activates both (`c:387-395`). Releasing `H1` settles only the first; the second stays `os_down_unused` or `os_down_used` with no timer.
  - The next press of `T` then activates the first entry (`layer_on(L)`) and cancels the second (`layer_off(L)`), in index order, so `L` ends up off: the gesture fails every other time for `H1`, until `H2` is released for any reason.
  - The source does not detect or warn about this. A working configuration needs a distinct trigger keycode per holding key.
- **Chained triggers.** Trace: if the one-shot layer `L` carries the trigger `T2` of another split entry, pressing it ends the first one-shot at once (rule 6) and activates the second entry with its holding key not held, which is rule 7: the second target stays on until its trigger is pressed again or its holding key is next released.
- **Two entries with the same target layer.** Trace: the layer is one bit (rule 10), so the first of them to be deactivated switches it off for both.
- Read: `active_osl_index` holds one index, the layer entry activated last (`c:28, 305`). Without `OS_STEROIDS_ABSORB_MODS` and `OS_STEROIDS_FREE_LAYER_STACK` it affects only the API functions `get_oneshot_layer_on_steroids` and `reset_oneshot_layer_on_steroids` (`u:36-40, 75-79`); the release branch it guards is empty (`c:420-435`).

### Rule 10. The layer stack

- Read: QMK's layer state is a bitmask. `layer_on` sets a bit, `layer_off` clears it, and the highest set layer with a non-transparent key wins (`vq:action_layer.c:136-149, 191-201, 354-372`). There is no stack and no count of who switched a layer on.
- Read: the module touches only bit `L` (`c:304`, `c:225`), apart from the optional flag below.
- Trace:
  - While `H` is held, `A` and `L` are both on. `L` is visible only if it is the higher of the two, or where `A` is transparent. In the readme's use case the target is a "tertiary" layer above the secondary one (`r:339`).
  - After a late release of `H`, or one after a regular key: `A` and `L` off. Whatever else was on stays on.
  - After a release inside the term: `A` off, `L` on until the event that uses the one-shot has been processed, then off.
  - Deactivation clears bit `L` unconditionally. A target layer that was already on for another reason (`TG`, `TO`, Layer Lock, another entry) is switched off as well.
  - After a timeout while the holding key is still down (possible when another entry armed the timer), `L` is off and `A` stays on until `H` is released.
- `OS_STEROIDS_FREE_LAYER_STACK` (read, `c:264-277`, `c:94-124`, `c:229-237`; trace for the combination with split): if the trigger is pressed from a layer higher than `L` that is not the default layer, that layer is switched off while the entry is active, which lifts the ordering condition. When `H` is released inside the term, the module swallows the layer-tap release (`c:106-124`, `c:430-433`) because `A` is already off, and `A` is not restored after the one-shot. When the entry is deactivated while `H` is still held, `A` is switched back on (`c:233-236`).
- `OS_STEROIDS_AUTOMATIC_RELEASE` (trace, `c:358-373`): the first regular key pressed while the holding key is held ends the entry, so `L` goes off after one key although `H` is still down.

## 3. Transition table

For one layer-only split entry `{OS(T, H, 0, L)}`, default callbacks, no other optional flag. "Swallowed" means the event goes no further than the module.

| # | From | Event | To | Effect on layers and on the event | Lines |
| --- | --- | --- | --- | --- | --- |
| 1 | idle | `T` pressed, `H` held or not | down_unused | `layer_on(L)`; the term clock starts; swallowed | `c:259-308`, `c:391-394` |
| 2 | idle | `T` released | idle | swallowed | `c:399-402` |
| 3 | idle | `H` pressed or released, any other key | idle | the module does nothing | `c:399-403`, `c:413` |
| 4 | down_unused, down_used, up_queued | `T` released | unchanged | swallowed | `c:404-406` |
| 5 | down_unused | ignored key (modifier) pressed | down_unused | none | `c:349-354` |
| 6 | down_unused | regular key pressed | down_used | the key is processed normally, resolved with `L` on | `c:358-363` |
| 7 | down_unused | `H` released, elapsed < term | up_queued | `L` stays on; idle timer armed; then QMK switches `A` off | `c:318-332` |
| 8 | down_unused | `H` released, elapsed >= term | idle | `layer_off(L)`; then QMK switches `A` off | `c:333-338`, `c:224-225` |
| 9 | down_used | `H` released | idle | as row 8 | `c:333-338` |
| 10 | down_unused, down_used | `T` pressed | idle | `layer_off(L)`; swallowed; `A` is untouched | `c:309-313` |
| 11 | up_queued | ignored key (modifier) pressed | up_queued | idle timer restarted | `c:349-354` |
| 12 | up_queued | regular key pressed, not an OSoS trigger | finished | the key is resolved with `L` on | `c:364-373` |
| 13 | finished | end of that event, if the whole chain returned `true` | idle | `layer_off(L)` | `c:440-452`, `vq:action.c:289-299` |
| 14 | finished | a later handler returned `false` | finished | `L` stays on until the next fully processed event | same |
| 15 | up_queued | trigger of another OSoS layer entry pressed | idle | `layer_off(L)` inside that press; the other entry is activated | `c:365-369`, `u:174-176` |
| 16 | up_queued | `T` pressed | idle | `layer_off(L)`; swallowed | `c:309-313` |
| 17 | up_queued | idle timer expires | idle, all entries | `layer_off(L)` | `c:135-139`, `u:69-73` |
| 18 | any non-idle | cancel key pressed | idle | `layer_off(L)`; the cancel key is processed normally | `c:343-347` |
| 19 | any non-idle | `clear_oneshots_on_steroids`, `cancel_oneshot_on_steroids(T)`, `reset_oneshot_layer_on_steroids` | idle | `layer_off(L)` | `u:64-79` |

Rows 1 to 13 and 15 to 19 are read; row 14 is a trace. Row 6 includes layer keys and tapped tap-hold keys (rule 6). In row 12 a held `H` is such a key.

## 4. Scenarios

`H = LT(A, tap)`; `T` sits on `A`; `L` is above `A`; `x`, `y` are regular keys. "Term" is the One Shot Term. Each row is a trace of the rules named; none was run.

| # | Sequence | Result | Rules |
| --- | --- | --- | --- |
| S1 | Hold `H`, tap `T`, type `x` `y`, release `H` | `x` and `y` come from `L`. Afterwards `A` and `L` are off. | 2, 5 |
| S2 | Hold `H`, tap `T`, release `H` inside the term, type `x` `y` | After the release `A` is off and `L` is on. `x` comes from `L`; `L` goes off when the press of `x` has been processed; `y` does not come from `L`, even if rolled over `x`. | 4, 5, 6 |
| S3 | Hold `H`, tap `T`, wait longer than the term, release `H` | `A` and `L` off; no one-shot. | 4, 5 |
| S4 | Hold `H`, tap `T`, type `x`, release `H` inside the term | `x` from `L`; `A` and `L` off; no one-shot. | 4, 5 |
| S5 | Hold `H` for a long time, then tap `T` and release `H` inside the term | One-shot as in S2: the term counts from `T`, not from `H`. | 4 |
| S6 | Hold `H`, press and keep `T` down, release `H` after the term | `L` goes off although `T` is still down. | 2, 5 |
| S7 | Hold `H`, press `T`, release `H` inside the term, then release `T` | One-shot as in S2; the release order of `T` and `H` does not matter. | 2, 5 |
| S8 | Hold `H`, tap `T`, tap `T` again (a real `T` or a transparent key at that position on `L`) | `L` off, `A` still on. A third tap switches `L` on again. Releasing `H` then switches `A` off (and `L`, by rows 7 to 9). | 8 |
| S9 | S2 up to the one-shot state, then tap `T` (a real `T` on `L`) | `L` off; nothing is sent. | 6, 8 |
| S10 | As S9, but the position holds a transparent key on `L` | The default layer's key at that position is sent and uses the one-shot. | 8 |
| S11 | One-shot state, no key for the timeout (5 s by default) | `L` off. | 6 |
| S12 | One-shot state, press a cancel key | `L` off; the key at that position on `L` is still sent. | 6 |
| S13 | One-shot state, press a modifier (plain, held mod-tap, `OSM`, modifier-only OSoS key), then `x` | The modifier does not use the one-shot and restarts the timeout; `x` comes from `L` with the modifier. | 6 |
| S14 | One-shot state, hold `H` again, type `x` | The one-shot is spent on `H`; `x` comes from `A`. | 6 |
| S15 | One-shot state, tap `H` | The tap keycode of `H` is sent; `L` off. | 6 |
| S16 | One-shot state, press `MO`, `TO`, `TG` or another held layer-tap | The one-shot is spent on that key; `L` goes off after it. | 6 |
| S17 | `T` pressed while `H` is not held (layer `A` on by other means) | `L` on and stays on over any number of keys, without timeout. | 7 |
| S18 | S17, then tap `T` again, or press a cancel key, or tap or hold-and-release `H` | `L` off. | 7 |
| S19 | Hold `H`, tap `T1` (layer `L1`), tap `T2` (layer `L2`, same holding key), release `H` inside the term counted from `T2` | `L1` off; `L2` one-shot. | 9 |
| S20 | Entries `{T, H1, 0, L}` and `{T, H2, 0, L}`; hold `H1`, tap `T`, release `H1` late; hold `H1`, tap `T` again | The second time `L` does not come on. | 9 |
| S21 | One-shot state of entry 1, press the trigger of a second split entry that sits on `L` | `L` off at once; the second target on, and it stays on as in S17. | 6, 9 |
| S22 | `L` already on through `TG(L)`; hold `H`, tap `T`, release `H` late | `L` is off afterwards. | 10 |
| S23 | One-shot state; the next key is a custom keycode whose handler returns `false` | `L` stays on; the key after it still comes from `L`. | 6 |
| S24 | `H` down, `T` down and up quickly, before QMK has decided that `H` is a hold | Decided by QMK's tap-hold settings. If `H` becomes a hold, the trigger is processed then and the term starts then; if `H` becomes a tap, the key at the trigger's position on the layers below is sent and the module sees no trigger. | 3, 4 |

## 5. Where source and readme disagree

The readme's section on split keys is `r:336-369`. Points 1 to 5 are specific to split entries or to layers; points 6 to 8 repeat findings of the research for issue #24 where they bear on layers.

1. **The holding key need not be held.** The readme describes pressing the holding key first (`r:364`) and says nothing about the trigger alone. In the source the trigger alone switches the target layer on and leaves it on without a timeout (rule 7). This is silence in the readme, not a contradiction, but it is the largest gap between the described gesture and the code.
2. **Where the term starts.** "I release `LT_NAV` within the One Shot Term" (`r:366`) does not say from when. The source counts from the processing of the trigger press (`c:307`), and requires that no regular key was pressed since (`c:318`).
3. **Per-key term.** The readme's callback example switches on OSoS keycodes (`r:124-134`). For a split entry the callback receives the holding key's keycode (`c:318, 404`), while the timeout callback receives the trigger (`c:330`).
4. **Timeout.** "OSoS keys are automatically deactivated after a period of keyboard inactivity" (`r:177`). For a layer this holds only in the one-shot state. A target layer that is on because its holding key is held, or because the trigger was pressed alone, is not subject to the timer; and when the single timer does expire, it switches off every OSoS layer, including one whose holding key is still down (rule 6, rule 10).
5. **What ends a one-shot layer.** The readme says the layer "remains active until the next non-modifier key press" (`r:380`). In the source, layer keys end it too, including the holding key held again (rule 6). The readme's description of the default ignore function lists layer keys as "handled" without saying in which direction (`r:396`).
6. **"Same position on both layers"** (`r:369`). The source cancels on the trigger keycode wherever it is; a transparent key is enough while the holding key is held, and a real trigger is needed only for the one-shot state (rule 8). The readme is correct but narrower than the code.
7. **Naming and syntax.** The readme says "holding key", the source `suppressor` (`h:28, 40`). The split example lacks a comma (`r:352`). The header says each OSoS key "must also be declared in `custom_keycodes`" (`h:45-46`), while the code only compares keycodes; that is why a layer-tap can be a holding key.
8. **One trigger with several holding keys** is neither offered by the readme nor rejected by the source; two entries with the same trigger compile and misbehave (rule 9).

Places where the readme and the source agree, for completeness: the trigger may be released at any time (`r:365`, rule 2); releasing the holding key after the term or after use switches both layers off (`r:367`, rule 5); cancelling with the holding key held returns to the holding key's layer (`r:369`, rule 8).

## 6. Observations about this repository

These are facts read in `repo`, set next to the rules above. They are not recommendations.

- `hydeik_vial`: `TAB_SYM = LT(_SYM, KC_TAB)` and `ENT_NUM = LT(_NUM, KC_ENT)` (`users/hydeik_vial/layout.h:63-64`). The layer order is `_BASE, _SYM, _NUM, _NAV, _FUN, _MOD` (`layout.h:13-20`), so NAV and FUN are above both holding layers, as rule 10 needs.
- The same corner key carries `TO(_NAV)` on both the SYM and the NUM layer (`layout.h:113, 131`). Under the reference's model that is one trigger with two holding keys, which rule 9 says does not work with one keycode.
- The same corner key on NAV carries `TO(_FUN)` (`layout.h:151`). In the reference that position on the target layer is where the trigger sits again to cancel (rule 8); a different trigger there is the chained case of rule 9 (S21), which leaves the second target on.
- The NAV and FUN layers carry the family's own one-shot mod keys (`layout.h:150`). They are custom keycodes, so the reference's default would treat them as regular keys that use a one-shot layer (rule 6), unlike its own modifier-only keys, which it ignores.
- `QK_LLCK` sits on the SYM, NUM and NAV layers (`users/hydeik_common/layers.h:51, 68`, `layout.h:151`). Its interaction with an OSoS layer was not traced (section 7).
- `hydeik`: `NAV_SPC = LT(_NAV, KC_SPC)`, layer order `_BASE, _SYM, _NUM, _NAV, _MOU, _FUN` (`users/hydeik/hydeik.h:11-16, 36`), so MOU is above NAV. The three thumb layer-tap keys use hold-on-other-key-press (`users/hydeik/hydeik.c:21-29`), which settles S24 as a hold in that family. `hydeik_vial` sets only `TAPPING_TERM 200` in `config.h`; its other tap-hold settings were not looked up.
- `hydeik` has `update_tri_layer_state(state, _SYM, _NUM, _FUN)` in `layer_state_set_user` (`users/hydeik/hydeik.c:273`); rule 5 notes that such a hook sees an intermediate layer state.

## 7. Not verified

- Nothing was compiled or run. Every trace should be confirmed in a unit test or on hardware before a decision rests on it, in particular rule 7 (trigger alone), rule 9 (shared trigger), row 14 / S23 (`os_finished` kept by a later handler), and S24.
- Core QMK was read at the vial-qmk pin only. The `hydeik` family builds against upstream QMK at another pin.
- Not traced: Layer Lock on or under an OSoS layer; combos whose output is a trigger or a holding key; Repeat Key; Tap Dance as the key that uses a one-shot layer; `OSL` and the module in one keymap; entries that carry both a modifier and a layer in split mode (the modifier follows the research for issue #24, the layer follows this document, but their combination was not walked through); `OS_STEROIDS_ABSORB_MODS` together with split.
- The 16-bit wrap of the term clock (rule 4) was derived from the types only.
- The upstream repository had no issues to cross-check against when the research for issue #24 looked on 2026-11-20; this was not repeated.
