---
status: accepted
---

# The one-shot mods of hydeik_vial are an own state machine, not a community module

The `hydeik_vial` keymap family needs one-shot mods whose pending state is invisible to the host (so a tapped modifier never lands on a mouse click) and a smart mod layer that latches them while a smart mod layer key is held. We implement both as one state machine of our own in `users/hydeik_vial/`, taking the ideas of the One Shots on Steroids community module (per-key states, a One Shot Term, re-tap cancel, a timeout) but none of its code, and without QMK's built-in one-shot mods.

## Considered Options

- **Use the One Shots on Steroids module as-is.** Rejected. It builds at the vial-qmk pin, but it keeps a pending Shift / Ctrl in QMK's built-in one-shot mods, which become host-visible again in any key-less report, and no callback changes that. Its split trigger/hold option, the only mechanism close to latching, allows one holding key per entry while there are three smart mod layer keys, and the same one-shot mod keys must not latch on the NAV and FUN layers. The self-switching-off layer, the release window, and the `SMT_MOD` tap rules have no counterpart, so they would be own code kept in sync with a module that swallows its keys before `process_record_user`. It also needs a `keymap.json` that breaks the local `make` build and makes the binary GPL-3.
- **Vendor the module sources and patch them.** Rejected for the same behavioural gaps, plus a patched copy to redo on every module update and GPL-3.0-or-later code inside a GPL-2.0+ userspace.
- **Extend the existing Exidex-style code.** Rejected: it represents a pending modifier as a really registered one, the opposite of what is required, so the additions would amount to a rewrite anyway.

## Consequences

- Pending one-shot mods live in the state machine's own state. QMK's one-shot mod API is not used; the common core's `ONESHOT_TIMEOUT` is matched by value only.
- The timeout fires on elapsed time, so `hydeik_vial` gains a `housekeeping_task_user` hook besides `process_record_user`.
- The custom keycode numbers stored by Vial stay stable: the eight one-shot mod keycodes keep their positions, and the unused first slot is kept as a reserved no-op.
- Improvements to One Shots on Steroids do not arrive by updating a module; they are reimplemented by hand if wanted.

The behaviour itself is specified in the tickets of issue #22.
