# RGB Matrix indicator hooks and state sync on a split keyboard

Research for issue #78 (map: #77). It answers where the RGB Matrix indicator
hooks run on `crkbd/rev4_1/mini`, which state QMK sends to the other half by
itself, and how the userspace can send state of its own.

## Sources

Everything below was read from the two firmware sources at the commits pinned
in `firmware_sources.json`:

| Firmware source | Commit | Checkout |
| --- | --- | --- |
| `qmk` (`qmk/qmk_firmware`, `master`) | `80973aa0b5413e9a5bbbf9293507f39e870f9aa3` | `.firmware/qmk` |
| `vial-qmk` (`vial-kb/vial-qmk`, `vial`) | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344` | `.firmware/vial-qmk` |

Paths are relative to the root of a firmware source. A citation without a
prefix holds for `qmk`; where the line numbers of `vial-qmk` differ they are
given as `vial-qmk: ...`. Nothing was built, flashed or measured: every
statement is a reading of the source. Statements that are a consequence drawn
from the source, and not a line of it, are marked **(derived)**.

## Summary

- Both hooks run on **both halves**, each in its own main loop. Each half can
  set only the LEDs wired to it: global indices 0-19 on the left half, 20-39 on
  the right half.
- Out of the box `crkbd/rev4_1/mini` syncs the matrix of the master half
  (`SPLIT_TRANSPORT_MIRROR`), the RGB Matrix settings with the suspend state
  (through `RGB_MATRIX_SPLIT`), the sync timer and the watchdog. It does
  **not** sync the layer state or the modifiers.
- `SPLIT_LAYER_STATE_ENABLE` and `SPLIT_MODS_ENABLE` add the layer state and
  the modifiers. Neither carries what tells a pending one-shot mod, a latched
  one-shot mod, a pending target layer or a locked target layer apart: that
  state is private to `smart_mod` and `target_layer`, and Layer Lock's
  `locked_layers` is not synced by any core transaction.
- State of the userspace's own goes through a custom split transaction
  (`SPLIT_TRANSACTION_IDS_USER`, `transaction_register_rpc`,
  `transaction_rpc_send`): 32 bytes each way by default, no throttle and no
  retry built in.
- The split transport is the same in both firmware sources. One difference
  matters: `rgb_matrix_set_color()` of `vial-qmk` does **not** drop an index
  that belongs to the other half.

## 1. Where the hooks run, and which LEDs each half can set

### The call path

`keyboard_task()` calls `rgb_matrix_task()` with no check for the master half
(`quantum/keyboard.c:710-732`; `vial-qmk: quantum/keyboard.c:733-754`).
`quantum_task()` right above it returns early on the half that is not the
master (`quantum/keyboard.c:648-652`; `vial-qmk: 657-661`), but
`rgb_matrix_task()` is called after it, outside that function. So the RGB
Matrix task, and both indicator hooks with it, run on each half.

Inside the task (`quantum/rgb_matrix/rgb_matrix.c:411-436`;
`vial-qmk: 373-406`):

```c
case RENDERING:
    rgb_task_render(effect);
    if (effect) {
        if (rgb_task_state == FLUSHING) { // ensure we only draw basic indicators once rendering is finished
            rgb_matrix_indicators();
        }
        rgb_matrix_indicators_advanced(&rgb_effect_params);
    }
    break;
```

- `rgb_matrix_indicators()` calls `rgb_matrix_indicators_kb()`, whose weak
  default calls `rgb_matrix_indicators_user()`
  (`rgb_matrix.c:442-453`; `vial-qmk: 412-423`). It runs **once per frame**,
  after the last chunk of the effect has been rendered.
- `rgb_matrix_indicators_advanced()` calls
  `rgb_matrix_indicators_advanced_kb(min, max)`, whose weak default calls
  `rgb_matrix_indicators_advanced_user(led_min, led_max)`
  (`rgb_matrix.c:487-504`; `vial-qmk: 457-474`). It runs **once per render
  chunk**, with the LED range of that chunk.
- `keyboards/crkbd/crkbd.c` defines neither `_kb` hook, so the weak defaults
  apply and the `_user` hooks are reached.
- Neither hook is called when `effect` is 0. The effect is 0 when RGB Matrix
  is switched off (`rgb_matrix_config.enable == 0`), suspended, or timed out
  (`rgb_matrix.c:317-324`; `vial-qmk: 378-384`).
- A new frame starts at most every `RGB_MATRIX_LED_FLUSH_LIMIT` = 16 ms
  (`quantum/rgb_matrix/rgb_matrix.h:85-87`, `rgb_matrix.c:299-303`;
  `vial-qmk: rgb_matrix.c:272-276`).

### The LED range of each half

`crkbd/rev4_1/mini` declares `"split_count": [20, 20]`
(`keyboards/crkbd/rev4_1/mini/keyboard.json:6-7`), which becomes
`RGB_MATRIX_SPLIT {20, 20}` (`data/mappings/info_config.hjson:163`). The 40
entries of the LED layout are indices 0-19 for the left half and 20-39 for the
right half (`keyboard.json:9-49`), all with `"flags": 4`.

`rgb_matrix_get_limits()` clips the range of every chunk to the half it runs
on (`rgb_matrix.c:455-481`; `vial-qmk: 425-451`):

```c
if (is_keyboard_left() && (limits.led_max_index > k_rgb_matrix_split[0])) limits.led_max_index = k_rgb_matrix_split[0];
if (!(is_keyboard_left()) && (limits.led_min_index < k_rgb_matrix_split[0])) limits.led_min_index = k_rgb_matrix_split[0];
```

With the default `RGB_MATRIX_LED_PROCESS_LIMIT` of `(40 + 4) / 5` = 8
(`rgb_matrix.h:89-91`), the advanced hook is called with these ranges
**(derived)**: left half `[0,8)`, `[8,16)`, `[16,20)`; right half ranges that
start no lower than 20, among them empty ones where `led_min > led_max`.

`RGB_MATRIX_INDICATOR_SET_COLOR(i, r, g, b)` sets the colour only if
`led_min <= i < led_max` (`rgb_matrix.h:109-112`), so through it a half can
set its own LEDs only, in both firmware sources.

A direct `rgb_matrix_set_color(index, ...)` behaves differently in the two
firmware sources: see section 4.

### Which half is which

- Left or right is read from a pin: `split.handedness.pin` = `GP21`
  (`keyboards/crkbd/rev4_1/info.json:49-51`,
  `quantum/split_common/split_util.c:139-148`).
- The master half is the one that detects USB
  (`split_util.c:181-189`; `USB_VBUS_PIN GP13` in
  `keyboards/crkbd/rev4_1/config.h:7`). Either half can be the master, so "the
  other half" is not always the right one.

### What runs on the half that is not the master

- Key events are processed on the master half only: `should_process_keypress()`
  returns `is_keyboard_master()` (`quantum/keyboard.c:396-398, 615-632`;
  `vial-qmk: 404-406, 624`). `process_record_user` and
  `post_process_record_user` therefore never run on the other half, and
  `smart_mod` and `target_layer` see no key event there.
- `housekeeping_task()` and with it `housekeeping_task_user()` run on both
  halves (`quantum/main.c:80`, `quantum/keyboard.c:425-429`;
  `vial-qmk: quantum/main.c:75`, `keyboard.c:433`).
- `keyboard_post_init_user()` runs on both halves
  (`quantum/keyboard.c:345-347`; `vial-qmk: 349`).

So on the half that is not the master the state of `smart_mod` and of
`target_layer` stays empty for ever **(derived)**: their `static struct state`
(`users/hydeik/features/smart_mod.c:32`, `users/hydeik/features/target_layer.c:60`)
is changed only by key events and by timeouts of what key events started.

## 2. What QMK syncs by itself

The master half runs `transactions_master()` in every matrix scan
(`quantum/matrix_common.c:91-96`,
`quantum/split_common/transactions.c:954-975`; `vial-qmk: 957-978`). Most
transactions are sent when the data changed, and again at least every
`FORCED_SYNC_THROTTLE_MS` = 100 ms (`transactions.c:70-72, 159-173`).

| State | Switched on by | What is sent | On `crkbd/rev4_1/mini` |
| --- | --- | --- | --- |
| Matrix of the other half | always | `GET_SLAVE_MATRIX_*` | on |
| Matrix of the master half | `SPLIT_TRANSPORT_MIRROR` (`split.transport.sync.matrix_state`) | `PUT_MASTER_MATRIX` (`transactions.c:209-224`) | **on** (`rev4_1/info.json:73-76`) |
| Sync timer | on unless `DISABLE_SYNC_TIMER` | `PUT_SYNC_TIMER` (`transactions.c:294-320`) | on |
| Layer state | `SPLIT_LAYER_STATE_ENABLE` (`split.transport.sync.layer_state`) | `layer_state` and `default_layer_state` (`transactions.c:333-357`) | **off** |
| Host LEDs | `SPLIT_LED_STATE_ENABLE` (`split.transport.sync.indicators`) | `host_keyboard_leds()` (`transactions.c:370-385`) | off |
| Modifiers | `SPLIT_MODS_ENABLE` (`split.transport.sync.modifiers`) | real mods, weak mods, QMK's one-shot mods and locked one-shot mods (`transactions.c:398-452`, `transport.h:99-108`) | **off** |
| RGB Matrix settings | `RGB_MATRIX_ENABLE` with `RGB_MATRIX_SPLIT` (`rgb_matrix.split_count`) | `rgb_matrix_config` (enable, mode, HSV, speed, flags) and the suspend state (`transactions.c:572-593`, `transport.h:90-97`) | **on** whenever RGB Matrix is built in |
| Watchdog | `SPLIT_WATCHDOG_ENABLE` (`split.transport.watchdog`) | `PUT_WATCHDOG` | on (`rev4_1/info.json:77`) |
| Encoders | `ENCODER_ENABLE` | `GET_ENCODERS_*` | on when encoders are built in |
| Activity timestamps | `SPLIT_ACTIVITY_ENABLE` (`split.transport.sync.activity`) | last matrix, encoder and pointing activity | off |

The `info.json` keys are mapped in `data/mappings/info_config.hjson:163,
207-218`. Each of the settings can equally be a `#define` in a `config.h`
(`docs/features/split_keyboard.md:257-283`). Nothing in `quantum/`,
`platforms/`, `builddefs/` or `keyboards/crkbd/` of either firmware source
defines `SPLIT_LAYER_STATE_ENABLE` or `SPLIT_MODS_ENABLE` by implication, and
neither `users/hydeik/` nor the two keymaps define any `SPLIT_*` setting
today.

The same table holds for `vial-qmk`; its `transactions.c` is the same code
three lines further down (for example `layer_state_handlers_master` at 338,
`mods_handlers_master` at 403, `rgb_matrix_handlers_master` at 577).

### What the built-in sync does and does not carry

- **Layer state.** With `SPLIT_LAYER_STATE_ENABLE` the other half gets a copy
  of `layer_state`, written by plain assignment (`transactions.c:346-349`): no
  `layer_state_set_user()` is called there. It tells that a target layer or
  the smart mod layer is on. It does not tell whether a target layer is held,
  pending or locked: that is `state.phase` and `state.locked_target_layers`
  of `target_layer.c`, and `locked_layers` of Layer Lock
  (`quantum/layer_lock.c:20`), and no transaction in `transactions.c` sends
  either.
- **Modifiers.** With `SPLIT_MODS_ENABLE` the other half gets the real mods,
  the weak mods and QMK's own one-shot mods. `smart_mod` does not use QMK's
  one-shot mods (ADR-0002), so `oneshot_mods` stays 0. A pending one-shot mod
  is not registered until a key press consumes it
  (`users/hydeik/features/smart_mod.c:168-177`), so it is in none of the synced
  fields. A latched one-shot mod is registered with `register_mods()`
  (`smart_mod.c:195`), so it shows up in the real mods, where it cannot be
  told from a modifier that is merely held **(derived)**.
- **RGB Matrix settings.** Both halves render the same effect with the same
  settings, and switching RGB Matrix off or suspending it on the master half
  reaches the other half too. Colours set with `rgb_matrix_set_color()` are
  **not** sent: each half fills its own LED buffer.

The docs say the same in general terms: "RGB indicators on split keyboards
will require state information synced to the slave half"
(`docs/features/rgb_matrix.md:763-765`).

## 3. Sending state of the userspace's own

### The mechanism

QMK has a custom data sync, modelled on a remote procedure call
(`docs/features/split_keyboard.md:325-400`):

1. Define the transaction IDs in a `config.h`:
   `#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_A, ...`
   (`quantum/split_common/transaction_id_define.h:118-121`;
   `vial-qmk: 114-117`). Defining it also builds in the four RPC transactions
   `PUT_RPC_INFO`, `PUT_RPC_REQ_DATA`, `EXECUTE_RPC`, `GET_RPC_RESP_DATA`
   (`transaction_id_define.h:21-23, 106-111`; `vial-qmk: 102-107`).
2. Register a handler for the receiving half, on both halves, for example in
   `keyboard_post_init_user()`:
   `void transaction_register_rpc(int8_t transaction_id, slave_callback_t callback);`
   (`quantum/split_common/transactions.h:26, 48`,
   `transactions.c:1001-1009`; `vial-qmk: 1004`). The handler has the
   signature
   `void (*)(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data)`.
3. Call from the master half, which the docs recommend to do in the
   housekeeping task (`docs/features/split_keyboard.md:381-383`):
   - `bool transaction_rpc_exec(int8_t id, uint8_t m2s_size, const void *m2s, uint8_t s2m_size, void *s2m);`
   - `transaction_rpc_send(id, size, buffer)`: master to the other half only.
   - `transaction_rpc_recv(id, size, buffer)`: the other half to master only.
   (`transactions.h:50-53`, `transactions.c:1011-1048`; `vial-qmk: 1014-1051`).

The transport is always initiated by the master half; the other half cannot
start a transaction.

No other mechanism for arbitrary data was found in `quantum/split_common/`.
The alternatives are the built-in transactions of section 2, and replacing
the transport altogether (`SPLIT_TRANSPORT = custom`,
`docs/features/split_keyboard.md:84`).

### Limits on size

- Payload: `RPC_M2S_BUFFER_SIZE` and `RPC_S2M_BUFFER_SIZE`, **32 bytes each**
  by default, overridable in `config.h`
  (`quantum/split_common/transport.h:27-33`; `vial-qmk: 26-32`). A larger
  request is refused: `transaction_rpc_exec()` returns false
  (`transactions.c:1019-1020`; `vial-qmk: 1022-1023`). The size parameters are
  `uint8_t`, so 255 bytes is the upper bound of any override.
- Number of transactions: `NUM_TOTAL_TRANSACTIONS <= 32`, checked at compile
  time (`transaction_id_define.h:135-136`; `vial-qmk: 126-127`). The built-in
  ones, the four RPC ones and the user's own share that budget.
- For comparison, the state in question is small: the sets of pending and of
  latched one-shot mods are one byte each (`smart_mod.c:32-98`), and the
  target layer state is a phase, a layer number and a `layer_state_t` mask of
  locked target layers (`target_layer.c:60-98`).

### Limits on rate

- There is **no throttle and no change detection** in `transaction_rpc_exec()`:
  every call goes on the wire. The helpers that send on change and every
  `FORCED_SYNC_THROTTLE_MS` (`send_if_condition`, `send_if_data_mismatch`) are
  `static` in `transactions.c:159-173` and not reachable from the userspace.
  The docs ask the caller to throttle, and show a call every 500 ms as the
  example (`docs/features/split_keyboard.md:360-378`).
- There is **no retry**: a failed call returns false and it is up to the
  caller to call again. It returns false at once while the transport is
  flagged as disconnected (`transactions.c:1012-1015`), which happens after
  `SPLIT_MAX_CONNECTION_ERRORS` = 10 failures
  (`quantum/split_common/split_util.c:46-55, 230-232`).
- One call is **four serial transactions**, blocking the main loop of the
  master half: `PUT_RPC_INFO`, `PUT_RPC_REQ_DATA`, `EXECUTE_RPC`,
  `GET_RPC_RESP_DATA` (`transactions.c:1035-1046`). Each transaction is one ID
  byte, one handshake byte back, then the payload
  (`platforms/chibios/drivers/serial_protocol.c:113-158`).
- The link is the RP2040 vendor serial driver, half duplex on one pin
  (`split.serial.driver` = `vendor`, `rev4_1/info.json:70-72`;
  `SERIAL_USART_TX_PIN GP12`, `rev4_1/config.h:6`), at the default
  `SERIAL_USART_SPEED` of 230400 baud with a `SERIAL_USART_TIMEOUT` of 20 ms
  per operation (`platforms/chibios/drivers/serial_usart.h:21-45`). A call
  with a payload of a few bytes is in the order of 15 to 20 bytes on the wire,
  that is somewhat under 1 ms at 230400 baud, before turnaround delays
  **(derived, not measured)**.

### Where the handler runs

On the receiving half the handler is called from the transport thread, not
from the main loop: `react_to_transaction()` runs in `SlaveThread`, a ChibiOS
thread of priority `HIGHPRIO`, and calls `slave_callback` while it holds the
lock of the split shared memory
(`platforms/chibios/drivers/serial_protocol.c:17-39, 51-94`). The indicator
hooks run in the main loop. Data that the handler stores is therefore written
in one thread and read in another **(derived)**.

## 4. Differences between `qmk` and `vial-qmk`

Compared: `quantum/rgb_matrix/`, `quantum/split_common/`,
`quantum/keyboard.c`, `quantum/matrix_common.c`,
`platforms/chibios/drivers/serial_protocol.c`,
`platforms/chibios/drivers/vendor/RP/RP2040/`, `keyboards/crkbd/` and the two
docs pages.

**Same in both:**

- `keyboards/crkbd/` is identical, apart from a `keymaps/vial` directory that
  only `vial-qmk` has. The keyboard settings of section 2 are the same.
- The call path of the indicator hooks, the clipping of the LED range, the
  frame limit and the process limit.
- The split transactions, their settings and defaults, the custom data sync
  API with its 32 byte buffers and its limit of 32 transactions.
  `serial_protocol.c`, `serial_vendor.c` and `transport.c` do not differ.

**Different:**

1. **`rgb_matrix_set_color()` with an index of the other half.**
   `qmk` returns -1 from `rgb_matrix_led_index()` for an index that does not
   belong to this half, and `rgb_matrix_set_color()` then does nothing
   (`quantum/rgb_matrix/rgb_matrix.c:159-186`):

   ```c
   const int led_index = rgb_matrix_led_index(index);
   if (led_index < 0) {
       return;
   }
   rgb_matrix_driver.set_color(led_index, red, green, blue);
   ```

   `vial-qmk` has the older code with no such guard
   (`vial-qmk: quantum/rgb_matrix/rgb_matrix.c:148-159`):

   ```c
   __attribute__((weak)) int rgb_matrix_led_index(int index) {
   #if defined(RGB_MATRIX_SPLIT)
       if (!is_keyboard_left() && index >= k_rgb_matrix_split[0]) {
           return index - k_rgb_matrix_split[0];
       }
   #endif
       return index;
   }

   void rgb_matrix_set_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
       rgb_matrix_driver.set_color(rgb_matrix_led_index(index), red, green, blue);
   }
   ```

   The driver writes `ws2812_leds[index]` unchecked
   (`platforms/chibios/drivers/vendor/RP/RP2040/ws2812_vendor.c:269-278`, the
   same in both). In `vial-qmk` this means **(derived)**:
   - on the right half, a global index 0-19 (an LED of the left half) is
     passed on unchanged and lights the right half's own LED of that local
     number, which is the wrong LED;
   - on the left half, a global index 20-39 writes an entry of the buffer of
     40 that no LED of that half shows.

   `RGB_MATRIX_INDICATOR_SET_COLOR` inside the advanced hook is not affected,
   because it checks `led_min` and `led_max` first. `rgb_matrix_indicators_user`
   gets no range, so a direct `rgb_matrix_set_color()` there relies on the
   guard that only `qmk` has.

2. **When the effect to render is chosen.** `qmk` decides once per frame, in
   `rgb_task_start()`, whether RGB Matrix is off, suspended or timed out
   (`rgb_matrix.c:305-328`); `vial-qmk` decides in every call of
   `rgb_matrix_task()` (`vial-qmk: rgb_matrix.c:373-384`). In both, the
   indicator hooks are skipped while the effect is 0.

3. **Naming of the RPC switch.** `qmk` defines `SPLIT_TRANSACTION_RPC` when
   `SPLIT_TRANSACTION_IDS_KB` or `SPLIT_TRANSACTION_IDS_USER` is defined
   (`transaction_id_define.h:21-23`); `vial-qmk` tests the two settings
   directly. The effect is the same.

4. **Only in `qmk`:** transaction IDs for community modules
   (`transaction_id_define.h:123-126`), the RGB Matrix flag steps and mode
   names (`rgb_matrix.c:75-80, 789-864`). Only in `vial-qmk`: the effect
   `vialrgb_direct_anim.h`. None of them changes the answers above.

## Not verified

- Nothing was run on the keyboard. In particular the behaviour of
  `rgb_matrix_set_color()` in `vial-qmk` with an index of the other half, and
  the time one custom transaction takes, are read from the source only.
- Whether VialRGB adds a path of its own that draws over the indicators was
  not looked at here; it belongs to the research on VialRGB.
