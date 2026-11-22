---
status: accepted
---

# The master half decides every indicator state

The indicators of `crkbd/rev4_1/mini` are drawn on both halves of a split keyboard, and each half can set only its own LEDs. The master half alone decides the indicator state of every key position, of both halves, and sends the result to the other half in a custom split transaction. The other half decides nothing: it draws what it received. The built-in sync of the layer state and of the modifiers (`SPLIT_LAYER_STATE_ENABLE`, `SPLIT_MODS_ENABLE`) stays off.

1. **The view.** What is sent is the indicator state of each of the 40 key positions and one bit that says whether any indicator is shown. It fits into the 32 bytes of one transaction.
2. **Both halves draw from the view.** The master half builds the view once per frame and draws its own LEDs from it, with the same code as the other half. Which half is the master only decides where the view comes from.
3. **Sending.** The master half sends the view when it has changed, again in the next frame when sending failed, and every 500 ms even without a change. The phase of the blink is not sent: each half takes it from the synchronised timer.
4. **No view, no indicators.** Until the first view arrives the other half shows no indicator and does not dim the ordinary effect. A view that is not renewed for 2 seconds is emptied.

## Considered Options

- **Switch the built-in sync on and let each half decide for itself.** Rejected: to decide, a half needs the keymap in use, the layers that are on, and the state of `smart_mod` and `target_layer`. The stored keymap is not synchronised between the halves: an edit in Vial reaches only the master half, so the other half would look keycodes up in a stale keymap. The state of the two state machines is carried by no built-in sync either, and would need a transaction of its own all the same.
- **Send a snapshot of the state and let the other half decide from it.** Rejected for the same reason: without the keymap in use the other half cannot turn the state into key positions. Making that work means synchronising the stored keymap as well.
- **Let the master half decide directly for its own LEDs and send only the other half's part.** Rejected: two ways to draw, and with them a way for the halves to disagree.

## Consequences

- The halves cannot disagree about an indicator state, with Vial and without.
- The other half is up to one frame plus one transaction behind the master half, and up to 500 ms behind after it restarted or after a transaction was lost.
- The transaction is registered and driven from the RGB Matrix hooks of the adapter, on both halves, so either half can be the master. Nothing is sent while RGB Matrix is off or the keyboard is suspended, when no indicator is shown anyway.
- Packing the view, the decision to send now and the decision that a view has expired are plain functions of the logic and are unit-tested. The call of the transaction and its handler are checked on the real keyboard.

The detailed decisions behind this are recorded in the tickets of issue #77.
