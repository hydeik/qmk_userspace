# One Shots on Steroids: behaviour from source, compared with the current one-shot mods

Research for issue #24. It describes what the One Shots on Steroids (OSoS) community module does according to its source, and compares that with the current implementation of the `hydeik_vial` keymap family. It does not recommend an adoption approach.

## Sources

| Tag | Source | Revision |
| --- | --- | --- |
| `c` | [`oneshots_on_steroids.c`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids.c) | `Kawamashi/qmk_modules` `main` at `d8dcb1351910ee003428c38768ae7c026d82c9ff` (2026-11-06) |
| `h` | [`oneshots_on_steroids.h`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids.h) | same |
| `u` | [`oneshots_on_steroids_utils.c`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/oneshots_on_steroids_utils.c) | same |
| `r` | [`readme.md`](https://github.com/Kawamashi/qmk_modules/blob/d8dcb1351910ee003428c38768ae7c026d82c9ff/oneshots_on_steroids/readme.md) | same |
| `vq:<file>` | [`vial-kb/vial-qmk`](https://github.com/vial-kb/vial-qmk/tree/dd43959ae5c08d8a28d38a1acf7b04e86b14a344), files under `quantum/` | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344`, the pin in `firmware_sources.json` |
| `cur` | `users/hydeik_vial/hydeik_vial.c` | this repository, `02643d8` |

A citation such as `c:261` means line 261 of that file at that revision.

How to read the confidence labels:

- **Verified** means read directly in the cited lines.
- **Trace** means derived by following several cited code paths by hand. Nothing in this document was compiled, run, or tried on hardware.
- **Assumption** means it depends on something outside the firmware (the host).

One host-side assumption is used throughout: the host applies to a click of an external mouse whatever modifier state the keyboard last sent in a keyboard report. The firmware never sees that click.

## 1. Short answer

- An OSoS key is a five-state machine. The modifier is registered on press. On a release inside the One Shot Term with no other key in between, Shift and Ctrl are unregistered and turned into pending QMK one-shot mods, while Alt and GUI stay registered. The one-shot ends on the next non-ignored key press, a second tap, a cancel key, or a timeout (default 5 s).
- A mouse click after a tapped Shift or Ctrl is therefore plain; after a tapped Alt or GUI it is still modified until the timeout, unless `should_mod_be_held_after_oneshot_release` is overridden.
- Out of the box, a modifier tapped while a layer key is held applies to one key only. The "modifiers persist while the layer key is held" part of the smart mod layer is reproduced only by split trigger/hold (with the layer key as the holding key, one holding key per entry) or, for modifiers that were active before the layer key was pressed, by absorb mods (which needs the layer key to be an OSoS layer key).
- Nothing in OSoS gives one key a hold function and a different tap function, and a second tap always cancels. "Hold `SMT_MOD` = MOD layer, tap = Shift, double tap = Caps Word" on one key has no built-in counterpart; only the generic custom-behaviour callback could host it.

## 2. The state machine of an OSoS key

### 2.1 Data

Each entry of `oneshot_os[]` has a `trigger` keycode, a `suppressor` keycode (the "holding key"), a `modifier` mask and a `layer` (`h:38-43`). Without `OS_STEROIDS_SPLIT_TRIGGER_HOLD` the `OS()` wrapper sets the suppressor equal to the trigger (`h:59-63`). Verified.

Per entry: `oneshot_state` and `oneshot_tap_time` (`c:26-27`). Global: `active_osl_index`, the single OSoS layer entry currently active (`c:28`); with absorb mods, `oneshot_pressed_mods` and `oneshot_absorbed_mods` (`c:40-41`); with a timeout, one `idle_timer` and one `timed_oneshot_index` for the whole module (`c:129-130`). Verified.

States (`h:50-56`): `os_idle`, `os_down_unused`, `os_down_used`, `os_up_queued`, `os_finished`.

### 2.2 Where it runs

`process_record_oneshots_on_steroids` is the module's `process_record` hook. Modules run before `process_record_kb` (and so before `process_record_user`), and before Caps Word and Tap Dance (`vq:quantum.c:363-364, 396, 402`). Verified.

The hook returns `false` for every press and release of a trigger keycode (`c:391-394, 401, 406`), so a trigger keycode never reaches `process_record_user`, Caps Word, or QMK's action handling. Verified.

The module's `post_process_record` hook deactivates entries in `os_finished` (`c:440-452`). QMK calls post-processing only when the whole `process_record` chain returned `true` (`vq:action.c:289-299`). Verified. Trace: if a later handler returns `false` for the key that used the one-shot, the entry stays in `os_finished` until the next event that passes the whole chain.

### 2.3 Transitions

"Regular key" means a key press that is neither a cancel key nor ignored (section 4).

| From | Event | To | What happens | Lines |
| --- | --- | --- | --- | --- |
| idle | trigger pressed | down_unused | `register_mods(modifier)`, `layer_on(layer)`, tap time recorded | `c:259-308` |
| down_unused | suppressor released, elapsed < One Shot Term | up_queued | Shift/Ctrl: `unregister_mods` then `add_oneshot_mods`. Alt/GUI: left registered. Idle timer armed. | `c:318-332` |
| down_unused | suppressor released, elapsed >= term | idle | modifier unregistered, layer off | `c:333-338`, `c:194-196` |
| down_unused | regular key pressed | down_used | nothing else | `c:358-363` |
| down_unused | regular key pressed, automatic release on | finished, or idle at once if that key is an OSoS key | see section 5.3 | `c:358-373` |
| down_used | suppressor released | idle | modifier unregistered, layer off | `c:333-338`, `c:197-203` |
| up_queued | regular key pressed | finished, or idle at once if that key is an OSoS key | | `c:364-373` |
| finished | end of that key event (post-process) | idle | Alt/GUI: `unregister_mods`. Shift/Ctrl: nothing; the pending one-shot mod was sent with the key's report. Layer off. | `c:446-450`, `c:211-217` |
| any non-idle | trigger pressed again | idle | cancel; the later release of the trigger is swallowed | `c:309-313`, `c:399-403` |
| any non-idle | cancel key pressed | idle | cancel; the cancel key itself is processed normally afterwards | `c:343-347` |
| any non-idle | idle timer expires | idle (all entries) | `clear_oneshots_on_steroids()` | `c:135-139`, `u:69-73` |
| any non-idle | ignored key pressed | unchanged | idle timer re-armed if it is running | `c:349-354` |

All rows verified, except that "the pending one-shot mod was sent with the key's report" relies on QMK (section 3).

Details that the table hides:

- **The One Shot Term** defaults to `TAPPING_TERM` (`h:65-67`) and is per key through `get_oneshot_on_steroids_term` (`u:206-212`). It is measured from the trigger press to the suppressor release (`c:307`, `c:318`). Verified.
- **Re-tap** cancels from every non-idle state, not only `os_up_queued` (`c:309-313`). In `os_up_queued` it deletes the pending one-shot mod for Shift/Ctrl and unregisters Alt/GUI (`c:204-210`). Verified.
- **Which modifiers are held after release** is decided by `should_mod_be_held_after_oneshot_release`. The default returns `false` when the entry's mask contains any Shift or Ctrl bit and `true` otherwise (`u:198-204`). The whole mask of the entry is passed in one call (`c:324`), so an entry for Ctrl+GUI is treated entirely as "not held". Verified.
- **Timeout.** `OS_STEROIDS_TIMEOUT` defaults to 5000 ms (`h:69-71`); 0 compiles the timer out (`c:128`). The timer is armed only when an entry enters `os_up_queued` (`c:329-332`) and re-armed only by ignored key presses (`c:350-352`). There is one timer; a second queued key overwrites it. Expiry deactivates every non-idle entry, including keys that are physically held (`c:137`, `u:69-73`). Verified. Trace: because deactivating the entry that owns the timer also stops the timer (`c:248-253`), tapping A, tapping B, then re-tapping B leaves A queued with no timeout.
- **`called_by_user`** is `true` for the API functions and the timeout, `false` for key-driven paths. It selects whether the flashing-modifier neutraliser is used in `os_down_used` and whether a pending one-shot mod is deleted in `os_finished` (`c:197-217`). Verified.

## 3. Registered modifiers, pending one-shot mods, and the mouse

QMK facts the module relies on (all verified at the vial-qmk pin):

- `register_mods` / `unregister_mods` change the real modifiers and send a keyboard report at once (`vq:action.c:1058-1074`).
- `add_oneshot_mods` only sets a variable and a timestamp; it sends nothing (`vq:action_util.c:419-425`).
- Every keyboard report includes the pending one-shot mods, and they are cleared only when that report contains a key (`vq:action_util.c:243-258`).
- QMK's own one-shot timeout clears pending one-shot mods on the next key event or report (`vq:action.c:111-118`, `vq:action_util.c:77-80, 247-253`). In vial-qmk it is the runtime setting `QS_oneshot_timeout`, default `ONESHOT_TIMEOUT` (`vq:qmk_settings.h:95-96, 185, 220`). This repository sets `ONESHOT_TIMEOUT 2000` in `users/hydeik_common/common_config.h`.

State by state, with the default `should_mod_be_held_after_oneshot_release`:

| State | Shift / Ctrl | Alt / GUI | External mouse click |
| --- | --- | --- | --- |
| idle | nothing | nothing | plain |
| down_unused, down_used (key or holding key physically held) | registered | registered | modified |
| up_queued | not registered; pending QMK one-shot mod | registered | Shift/Ctrl: plain, and the one-shot stays pending for the next key. Alt/GUI: modified. |
| finished (within one key event) | sent with that key's report | registered until post-process | not applicable |

The first two columns are verified (`c:261`, `c:322-327`, `c:211-217`). The mouse column is the host assumption applied to them.

Consequences by trace:

- A click does not consume or cancel anything. After a tapped Alt or GUI, clicks stay modified until the next key press, a re-tap, or the timeout. Overriding `should_mod_be_held_after_oneshot_release` to return `false` for those modifiers would make them pending as well (`c:324-327`).
- A pending Shift/Ctrl becomes visible to the host as soon as any keyboard report without a key is sent, for example when a second modifier is registered or unregistered: `get_mods_for_report` ORs the pending mods into that report and does not clear them (`vq:action_util.c:254-257`). So "tap OSoS Shift, then tap or hold OSoS GUI, then click" gives a Shift+GUI click.
- Two timeouts apply to a tapped Shift/Ctrl: OSoS's (5000 ms default) and QMK's one-shot timeout (2000 ms with this repository's setting). Between the two, the OSoS entry is still `os_up_queued` but QMK has already dropped the modifier, so the next key is unmodified.
- If the key that ends an `os_up_queued` Shift/Ctrl sends no keyboard report containing a key (a custom keycode, a tap dance that resolves later, a media key), the OSoS entry goes idle while QMK's one-shot mod is still pending and applies to the next real key (`c:214-216` deletes it only when `called_by_user`).

## 4. Layer keys, mod-taps, combos, tap dance, Caps Word

### 4.1 The default `should_oneshot_on_steroids_ignore_key`

Verified at `u:141-195`. It classifies the pressed key:

- **Mod key:** `KC_LCTL ... KC_RGUI`, `KC_HYPR`, `KC_MEH`, any `OSM()`, a mod-tap that is held (`tap.count == 0`), and a modifier-only OSoS key (`u:143, 148-155`).
- **Layer key:** a layer-tap that is held, `TT`, `MO`, `LM`, `OSL`, `TO`, `TG`, the Tri Layer keys, and an OSoS key that has a layer (`u:144, 158-168`).
- Anything else is a regular key and is never ignored (`u:171`). That includes a tapped mod-tap or layer-tap (`u:149, 159`), tap dance keycodes, `QK_LLCK`, and custom keycodes.

Then, by the kind of the active OSoS entry:

| Active OSoS entry | Pressed key | Result | Lines |
| --- | --- | --- | --- |
| has a layer | layer key | not ignored: ends or uses the OSoS layer | `u:174-176` |
| has a layer | mod key | ignored | `u:183` |
| has a layer | modifier-only OSoS key, with `OS_MOD_SHOULD_LEAVE_OS_LAYER` | not ignored | `u:178-181` |
| modifier-only | mod key or layer key | ignored: the one-shot survives | `u:191-193` |
| modifier-only | OSoS layer key that absorbs mods, with `OS_STEROIDS_ABSORB_MODS` | not ignored: the modifier is handed to the layer (section 5.1) | `u:186-190` |

Only presses are classified; releases of other keys do not go through this function (`c:417-419`).

### 4.2 Per key type

- **`MO`, held `LT`, `LM`, `TO`, `TG`, `TT`** (verified from 4.1): ignored by a modifier-only OSoS key, so a queued modifier waits for the first regular key on the new layer and then applies to that one key. They are regular keys for an OSoS layer key.
- **Tapped `LT` / mod-tap**: a regular key; it consumes the one-shot. The module sees these keys after tap-hold resolution, because it reads `record->tap.count` in `process_record` (`u:149, 159`). Verified.
- **Held mod-tap, plain modifiers, `OSM()`**: ignored, so they stack with a queued OSoS modifier. Verified.
- **Combos** (trace): combos are resolved in `pre_process_record_quantum` (`vq:quantum.c:285-290`), before `process_record`, so the module sees only the keycode a combo emits and classifies it like any other keycode. Not checked: `OS_STEROIDS_FREE_LAYER_STACK` reads the source layer of `record->event.key` (`c:268`), which for a combo is not a matrix position.
- **Tap dance** (trace): the module runs before `process_tap_dance` (`vq:quantum.c:363, 402`) and a tap dance keycode is a regular key, so the one-shot is consumed at the press of the tap dance key, not when the dance resolves. A queued Alt/GUI is unregistered in the post-process of that press (`c:211-213`), which is before a dance that resolves on timeout (`vq:process_tap_dance.c:186-195`) sends anything. A queued Shift/Ctrl stays pending in QMK and still reaches the dance's output if it arrives inside QMK's one-shot timeout.
- **Caps Word** (trace): trigger keycodes never reach `process_caps_word` (section 2.2). Therefore `DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD`, which watches only `KC_LSFT` and `OSM(MOD_LSFT)` (`vq:process_caps_word.c:130`), cannot be triggered by an OSoS Shift, and `CAPS_WORD_INVERT_ON_SHIFT`, which tracks the same keycodes (`vq:process_caps_word.c:29-71`), does not see it either. While Caps Word is on, the next key press with any modifier other than Shift or AltGr active (registered or pending) turns Caps Word off (`vq:process_caps_word.c:88, 155, 252`); an OSoS Shift does not.

## 5. The optional behaviours

### 5.1 Absorb mods (`OS_STEROIDS_ABSORB_MODS`)

Verified unless marked.

- It acts only at the moment an OSoS **layer** key is pressed from idle, and only if `should_oneshot_on_steroids_absorb_mods(trigger)` returns true (default: always, `u:225-231`) (`c:279-302`).
- At that moment:
  - Registered modifiers other than the entry's own are recorded in `oneshot_pressed_mods` (`c:291-294`). That covers a physically held modifier and a tapped Alt/GUI that is still registered.
  - Pending one-shot mods other than the entry's own are removed from QMK's one-shot state, registered as real modifiers, and recorded in `oneshot_absorbed_mods` (`c:295-300`).
  - If the entry that owns the idle timer has one of those modifiers, the timer is stopped (`c:281-289`).
- A modifier-only OSoS key that was queued is then deactivated without unregistering its modifier (`u:186-190`, `c:365-369`, `c:160-167`, `c:43-56`). Trace.
- A modifier recorded in `oneshot_pressed_mods` moves to `oneshot_absorbed_mods` when its key is released while the layer entry is `os_down_unused` or `os_up_queued`; the release is swallowed so QMK does not unregister it. If the layer entry is already `os_down_used`, the release is processed normally and the modifier ends there (`c:43-56`, `c:58-82`, `c:423-428`). This matches the readme ("considered used and is deactivated when released", `r:296`).
- Absorbed modifiers are unregistered when the absorbing layer entry is deactivated (`c:238-244`): at the release of a held layer key, after the next key for a tapped one, or on cancel/timeout.
- A modifier pressed **after** the OSoS layer key is not absorbed: it is not in `oneshot_pressed_mods`, so `should_unregister_mod` returns true for it (`c:43-56`). Trace: with the layer key held, a modifier-only OSoS key tapped on that layer becomes an ordinary one-shot for one key.
- Absorption does not happen for `MO`, `LT` or `LM` layer keys; only entries of `oneshot_os[]` with a layer run this code (`c:263`).
- `get_absorbed_mods` / `set_absorbed_mods` expose the absorbed set to keymap code (`c:84-90`).

### 5.2 Split trigger/hold (`OS_STEROIDS_SPLIT_TRIGGER_HOLD`)

Verified unless marked.

- The flag only changes the `OS()` wrapper to take a separate holding key (`h:59-63`). The processing is the same code: activation on the trigger press (`c:391-394`), and the release logic on the **suppressor** release (`c:404`).
- The release of the trigger itself does nothing and is swallowed (`c:404-406`).
- The suppressor may be any keycode, for example a layer-tap; its release is matched without looking at `tap.count` and is not swallowed, so its own action still runs (`c:404`).
- Trace of an entry `OS(X, LT_KEY, mod, 0)` with `LT_KEY` held:
  - Press X: modifier registered, `os_down_unused`.
  - Type keys: `os_down_used`; the modifier stays registered across all of them.
  - Release `LT_KEY`: modifier unregistered.
  - Instead, release `LT_KEY` within the One Shot Term counted from the press of X, with no key in between: `os_up_queued`, a one-shot for the next key.
  - Instead, release `LT_KEY` later with no key in between: cancelled.
- Each entry has exactly one suppressor (`h:38-43`). Trace: two entries with the same trigger and different suppressors both activate on the trigger press (`c:387-395`) and are deactivated independently, so the modifier is unregistered when the first suppressor is released while the other entry stays non-idle; the next press of the trigger then cancels instead of activating.
- Trace: if the trigger is pressed while its suppressor is not held (for example from a layer reached with `TO`), the entry stays in `os_down_unused` / `os_down_used` with the modifier registered until the suppressor is next released, the trigger is pressed again, or a cancel key is pressed. The idle timer is not armed in those states (`c:329-332`).

### 5.3 Automatic release (`OS_STEROIDS_AUTOMATIC_RELEASE`)

- Verified: with the flag, a regular key pressed while the OSoS key is still held is treated like the key after a tap when `should_automatically_release_oneshot_on_steroids(key, trigger, record)` returns true (default: always, `u:245-251`): the entry goes to `os_finished` and is deactivated after that key press (`c:169-175`, `c:358-373`). The later physical release of the trigger is swallowed (`c:399-403`).
- Trace, an apparent defect: for an entry whose modifier is not "held after release" (Shift/Ctrl by default), the modifier is still really registered at that point, but the `os_finished` branch only unregisters "held" modifiers and otherwise only deletes a pending one-shot mod when `called_by_user` (`c:211-217`). On this path nothing unregisters the Shift or Ctrl, so it stays registered until the same OSoS key is used again. The readme describes the modifier as deactivated (`r:218`).

### 5.4 `OS_MOD_SHOULD_LEAVE_OS_LAYER`

Verified at `u:174-183`. It changes one decision in the default ignore function: when the active entry is an OSoS layer key and the pressed key is a modifier-only OSoS key, the press is not ignored.

- If the layer entry is `os_up_queued` (tapped), it is deactivated immediately, during the modifier key's press event (`c:364-369`). The modifier key itself was already activated in the first pass (`c:387-395`). The layer is gone and the modifier is queued for the next key on the layers underneath. This is the readme's example (`r:380-386`).
- If the layer key is still held (`os_down_unused`), it only becomes `os_down_used`; the layer stays until the key is released (`c:358-363`).
- It applies only to modifier-only OSoS keys (`u:180`); plain modifiers, mod-taps and `OSM()` are still ignored.
- It has no effect on a layer entered with `MO`, `LT`, `LM`, `TO` or `TG`, because those are not OSoS entries.

### 5.5 Free layer stack (`OS_STEROIDS_FREE_LAYER_STACK`)

Not asked for by the ticket; for completeness. Verified: when an OSoS layer key is pressed on a layer higher than its target, that source layer is switched off until the OSoS layer ends (`c:264-277`, `c:229-237`). `_MOD` is the highest layer of the keymap family (`users/hydeik_vial/layout.h:13-20`).

## 6. Where source and readme disagree

1. The readme links the default ignore function as `oneshots_on_steroids.c#L551` (`r:396`). That file has 453 lines; the function is in `oneshots_on_steroids_utils.c` (`u:141-195`).
2. The readme's absorb callback is named `should_osl_on_steroids_absorb_mods` (`r:308`). The module declares and calls `should_oneshot_on_steroids_absorb_mods` (`h:157`, `c:280`, `u:188, 225`). A function with the readme's name would never be called.
3. Automatic release: the readme says the key is "automatically deactivated after the next keypress" (`r:218`); for Shift/Ctrl the registered modifier is not unregistered on that path (section 5.3, trace).
4. Timeout: the readme says OSoS keys are deactivated "after a period of keyboard inactivity" (`r:177`). In the source the timer is armed only by a tap, exists once for the whole module, is extended only by ignored key presses, and its expiry clears every OSoS key, held ones included (section 2.3).
5. `OS_STEROIDS_TIMEOUT 0`: the readme says the one-shot then stays "indefinitely until another key is pressed" (`r:194`). A pending Shift/Ctrl is a QMK one-shot mod and still falls under QMK's one-shot timeout, which is on by default in vial-qmk (section 3).
6. The header says each OSoS key "must also be declared in `custom_keycodes`" (`h:45-46`). The code compares keycodes for equality only (`c:391`); this is why a layer-tap can be a suppressor. No behaviour depends on the trigger being a custom keycode.
7. Minor: the readme's first example lists `OS_NAV` in the array but not in the enum (`r:54-68`), and the split example lacks a comma (`r:352`).

## 7. The current implementation, as far as the comparison needs it

All from `cur`; verified unless marked.

- A `COS_*` key registers its modifier on press (`cur:154-156`). On release it is unregistered only if it was marked used (`cur:160-163`); otherwise it stays registered.
- The release of any non-ignored key marks held `COS_*` keys as used and unregisters every other registered modifier ("queued"), unless a smart mod layer key is held, in which case the queued modifiers are marked used instead and stay (`cur:114-125`, `cur:208-211`). The trigger is a key **release**, and it acts on all of `get_mods()`, not only on modifiers from `COS_*` keys.
- Holding a smart mod layer key (`LT`/`MO`/`LM` targeting the MOD layer) turns the layer on and marks the currently registered modifiers as used; releasing it unregisters all used modifiers, unless a `COS_*` key is still held (`cur:127-148`).
- Holding any other layer key marks the registered modifiers as used on press and unregisters them on release (`cur:103-112`, `cur:171-195`).
- When the last held `COS_*` key is released, the MOD layer is switched off even if the smart mod layer key is still held (`cur:165-169`). Trace: to stack two modifiers during one hold, the second must be pressed before the first is released.
- A tap of `SMT_MOD` clears all modifiers and the bookkeeping on press, and registers Left Shift on release (`cur:227-241`); `KC_KP_0` is never sent (`cur:242`). A second tap inside the tapping term turns Caps Word on (`cur:232-236`) and, on its release, registers Shift again.
- `COS_CLR` clears all modifiers on press (`cur:204-207`).
- `clear_mods()` only clears the variable and sends no report (`vq:action_util.c:346-348`), and a custom keycode maps to no action (`vq:keymap_common.c:196-198`). Trace: after `COS_CLR` alone the host is not told that the modifiers were released until the next keyboard report, so a mouse click directly after `COS_CLR` would still be modified. After a tap of `SMT_MOD` a report is sent (by `register_mods`), with Shift down. This was not confirmed on hardware.
- `COS_MOD` is declared and listed as ignored (`cur:39`) but has no behaviour. `ENT_MOD` is defined in `layout.h` but not placed on a layer there.

## 8. Smart mod layer elements and their OSoS counterparts

"Counterpart" means a built-in mechanism of the module. Where only a callback could host the behaviour, that is said explicitly.

| # | Element of the current behaviour | OSoS mechanism | Gap |
| --- | --- | --- | --- |
| 1 | Holding `SMT_MOD` / `SPC_MOD` turns the MOD layer on (`LT` hold) | None needed: a held `LT` is ignored by modifier-only OSoS keys (4.1). Alternatively an OSoS layer key (`layer` field). | An OSoS layer key has no separate tap function (see 6 and 7 below), so it cannot also send Space. |
| 2 | A modifier tapped on the MOD layer persists across several keys while the layer key is held, and is released with the layer key | Split trigger/hold with the layer key as holding key (5.2). Or physically holding the modifier key (`os_down_used`). | One holding key per entry, while two keys hold the MOD layer today. A trigger pressed without its holding key held stays registered (5.2); the same `COS_*` keycodes also sit on NAV and FUN. Without split, the tapped modifier applies to one key only. |
| 3 | A modifier that was already queued when the layer key is pressed persists while the layer is held | Absorb mods (5.1) | Only if the layer key is an OSoS layer key, not `LT`. |
| 4 | The same as 3 for any other held layer key (SYM, NUM) | Absorb mods | Same condition. With `LT` keys the queued modifier applies to the first key on that layer and ends. |
| 5 | A modifier tapped on the MOD layer survives the release of the layer key and applies to the next key, with no time limit | Default behaviour without split (`os_up_queued`; layer keys are ignored) | Bounded by the timeouts (section 3). With split, the holding key must be released within the One Shot Term of the trigger press, otherwise the one-shot is cancelled (5.2). |
| 6 | Tap `SMT_MOD` = Shift for the next key | A modifier-only OSoS Shift key gives a pending one-shot Shift | Not on the same key as the layer hold: a trigger is matched by keycode without regard to tap or hold (`c:391`), and the module swallows the event, so an `LT` used as a trigger never performs its layer action. No built-in counterpart; `is_oneshot_on_steroids_custom_behavior` (`c:382`, `u:120-131`) is the only hook. |
| 7 | Double tap `SMT_MOD` = Caps Word | None | A second tap of an OSoS key always cancels (`c:309-313`), and QMK's double-tap-Shift detection never sees OSoS keys (4.2). The readme shows Caps Word being switched on from the custom-behaviour callback (`r:409-421`). |
| 8 | Tap `SMT_MOD` also clears all one-shot mods; `COS_CLR` clears them | Cancel keys (`is_oneshot_on_steroids_cancel_key`, default: none, `u:133-139`), re-tap, timeout, and the functions `clear_oneshots_on_steroids` etc. (`u:64-117`) | A cancel key is still processed as a normal key afterwards (`c:343-347`). |
| 9 | The MOD layer switches off when the last `COS_*` key is released | `OS_MOD_SHOULD_LEAVE_OS_LAYER` (5.4) | Only for a MOD layer entered by a tapped OSoS layer key; a held layer key keeps the layer. |
| 10 | A one-shot mod stays until the first other key is released, so rolled keys all get it | None | OSoS ends the one-shot at the first key press. |

## 9. Scenarios side by side

OSoS column: default callbacks and no optional flags, unless a flag is named. "Shift/Ctrl" and "Alt/GUI" refer to the default `should_mod_be_held_after_oneshot_release`. Rows are traces of the code cited in sections 2 to 7.

| Scenario | Current implementation | OSoS |
| --- | --- | --- |
| Press the modifier key | Registered at once | Registered at once |
| Hold it, type keys, release it | Modifier on all keys; unregistered on release (marked used at the first key release) | Same (`os_down_used`) |
| Release it quickly, no other key (tap) | Stays registered, with no time limit | Shift/Ctrl: unregistered, pending one-shot. Alt/GUI: stays registered. Timeout armed. |
| Hold it longer than the term, no other key, release | Same as a tap: stays registered | Cancelled: unregistered, no one-shot |
| Next key after a tap | Modifier stays until the first non-ignored key is **released**; a second key rolled before that also gets it | Applies to the next key **press** only; a rolled second key is unmodified |
| Tap the same modifier again | Nothing changes; it stays registered | Cancelled |
| No key for a while after a tap | Nothing; no timeout | Cleared after 5 s (default). A pending Shift/Ctrl is already dropped by QMK after its own one-shot timeout (2 s in this repository). |
| Cancel key | `COS_CLR` or a tap of `SMT_MOD` (which then leaves Shift registered) | None by default; keys chosen in `is_oneshot_on_steroids_cancel_key` |
| Mouse click while the modifier key is held | Modified | Modified |
| Mouse click after a tap | Modified, and it stays modified for later clicks | Shift/Ctrl: plain; the one-shot still waits for the next key. Alt/GUI: modified until timeout, next key or re-tap. |
| Mouse click after tapping Shift and then another modifier | Modified by both | Modified by both: the pending Shift is included in the report sent for the second modifier (section 3) |
| Tap modifier A, then modifier B | Both stay registered and apply together | Both queued and apply together to the next key |
| Tap a modifier, then hold a plain layer key (`LT` hold, `MO`) and type several keys | Modifier on all keys until the layer key is released; lost if the layer key is released without typing | Layer key ignored; modifier on the first key only |
| Tap a modifier, then tap a layer-tap key (for example Space) | The tap is an ordinary key and uses up the modifier | Same |
| Hold `SMT_MOD` / `SPC_MOD`, tap a modifier, type several keys, release the layer key | Modifier on all keys; unregistered when the layer key is released | First key only. With split trigger/hold and the layer key as holding key: all keys, unregistered when the layer key is released. |
| Hold the layer key, tap a modifier, release the layer key, then type a key | Modifier on that key, no time limit | Modifier on that key if inside the timeouts. With split: only if the layer key is released within the One Shot Term of the modifier's press. |
| Modifier queued first, then hold the MOD layer key and type several keys | Modifier on all keys until the layer key is released | With an `LT`: first key only. With an OSoS layer key and absorb mods: all keys until the layer key is released. |
| Tap `SMT_MOD` | Clears all modifiers, registers Shift until the next key is released | No counterpart on that key (section 8, row 6) |
| Double tap `SMT_MOD` | Caps Word on, and Shift registered until the next key is released | No counterpart; a second tap of an OSoS key cancels it |
| Held mod-tap after a tapped modifier | Not treated specially: its release counts as another key and ends the one-shot | Ignored; both modifiers apply to the next key |
| Combo | Seen as the keycode the combo emits | Same |
| Tap dance key after a tapped modifier | The release of the tap dance key ends the one-shot, which can be before the dance resolves | Consumed at the press. Alt/GUI is unregistered before a dance that resolves later; a pending Shift/Ctrl still reaches its output. |
| Caps Word active, then a modifier key | A `COS_*` key reaches Caps Word as an unknown keycode and turns it off (`users/hydeik_common/common.c:38-40`) | The OSoS key is not seen by Caps Word; the next key with Ctrl/Alt/GUI active turns it off, Shift does not |

## 10. Not verified

- Nothing here was compiled or run. Every "trace" should be confirmed on hardware or in a test before a decision rests on it, in particular the automatic-release finding (5.3), the pending-Shift-in-a-later-report finding (section 3), and the `COS_CLR` report finding (section 7).
- Whether the module builds against the pinned vial-qmk was not examined. The pin does contain the module hooks in `quantum.c` (`vq:quantum.c:186, 198, 363`); the build-system side is a separate question.
- Core QMK behaviour was read at the vial-qmk pin only. The module is written for upstream QMK, where the one-shot timeout is a compile-time option rather than a Vial setting.
- The host assumption about mouse clicks and keyboard modifier state was not tested on the owner's operating systems.
- Interaction with Repeat Key, Layer Lock, Vial's dynamic combos and tap dances, and the flashing-modifier neutraliser (`c:153-158`) was not traced beyond what is written above.
- `gh issue list` on the upstream repository returned no issues, open or closed, on 2026-11-20, so the findings could not be cross-checked against reports from other users.
