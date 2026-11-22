# RGB Matrix indicator hooks, VialRGB effects, and RGB switched off

Research for issue #79 (map: #77). Facts only; no design.

## Sources

Everything below was read from the firmware sources at the commits pinned in
`firmware_sources.json`, as fetched into `.firmware/`:

| Firmware source | Commit | Path prefix used below |
| --- | --- | --- |
| `qmk` (`qmk/qmk_firmware`, `master`) | `80973aa0b5413e9a5bbbf9293507f39e870f9aa3` | `qmk:` |
| `vial-qmk` (`vial-kb/vial-qmk`, `vial`) | `dd43959ae5c08d8a28d38a1acf7b04e86b14a344` | `vial-qmk:` |

Nothing was compiled or run on a keyboard. Statements marked "not verified" say
so where they appear.

## The keyboard, as configured in the firmware sources

`keyboards/crkbd/` is identical in both firmware sources apart from the
`keymaps/` directories (`diff -rq`).

- Driver `ws2812`, `max_brightness: 50`, `sleep: true`, and 44 effects switched
  on, `cycle_left_right` among them: `keyboards/crkbd/rev4_1/info.json:313-362`.
  `keyboards/crkbd/info.json:131-134` says `max_brightness: 120`; the deeper
  `rev4_1` level is expected to win. The merged value was not verified with a
  build.
- 40 LEDs, `split_count: [20, 20]`, every LED with `flags: 4`
  (`LED_FLAG_KEYLIGHT`): `keyboards/crkbd/rev4_1/mini/keyboard.json:6-50`.
- These keys become `RGB_MATRIX_MAXIMUM_BRIGHTNESS`, `RGB_MATRIX_SLEEP`,
  `RGB_MATRIX_SPLIT`: `qmk:data/mappings/info_config.hjson:159-163`,
  `vial-qmk:data/mappings/info_config.hjson:145-149`.
- No `timeout` is set, so `RGB_MATRIX_TIMEOUT` is `0`
  (`quantum/rgb_matrix/rgb_matrix.h:28-30` in both).

## 1. Are the hooks called for every effect, after the effect has drawn?

**Yes, for every effect, the VialRGB direct mode included, as long as the
effect number is not zero.** The hooks are not tied to any effect; they are
called from the task loop.

`vial-qmk:quantum/rgb_matrix/rgb_matrix.c:384-398`:

```c
uint8_t effect = suspend_backlight || !rgb_matrix_config.enable ? 0 : rgb_matrix_config.mode;
...
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

`qmk:quantum/rgb_matrix/rgb_matrix.c:420-428` has the same block.

- An effect is rendered in several calls of the task, `RGB_MATRIX_LED_PROCESS_LIMIT`
  LEDs per call; the default is `(RGB_MATRIX_LED_COUNT + 4) / 5`, which is 8 for
  40 LEDs (`rgb_matrix.h:89-91`).
- `rgb_matrix_indicators_advanced_user(led_min, led_max)` is called after
  **each** of those calls, with the range of LEDs that call just drew
  (`rgb_matrix.c:457-466` in `vial-qmk`). `RGB_MATRIX_INDICATOR_SET_COLOR`
  writes only inside that range (`rgb_matrix.h:109-112`).
- `rgb_matrix_indicators_user()` is called **once per frame**, after the last
  render call and before the flush, and may write any LED.
- The frame is sent to the LEDs in the next task state, `FLUSHING`
  (`rgb_task_flush`), so what a hook writes is what is shown. An indicator
  overrides the effect.
- Both hooks go through `_modules` and `_kb` first; `crkbd` defines neither
  `rgb_matrix_indicators_kb` nor the advanced one (no match in
  `keyboards/crkbd/crkbd.c`), so the weak defaults forward to `_user`.

**VialRGB direct mode** is an ordinary effect in the effect list:
`vial-qmk:quantum/rgb_matrix/animations/vialrgb_direct_anim.h:1-18`, included
from `animations/rgb_matrix_effects.inc:47`. It redraws every LED of its range
from `g_direct_mode_colors` on every frame, so it goes through the same
`rgb_task_render` switch and the hooks run after it. `g_direct_mode_colors` is
filled from the raw HID command `vialrgb_direct_fastset`
(`vial-qmk:quantum/vialrgb.c:90-110`, `176-179`); it is referenced nowhere in
`quantum/split_common/`, so it is not sent to the other half (found by `grep`,
not tested).

VialRGB itself adds no render path. It only maps its own effect ids to the QMK
ones and calls the public RGB Matrix API (`vialrgb.c:59-74`, `161-189`).

### Details that matter for an indicator

- **Some effects do not redraw every LED on every frame.** `RAINDROPS`,
  `JELLYBEAN_RAINDROPS` and `PIXEL_RAIN` draw all LEDs only on the first frame
  (or never, for `PIXEL_RAIN`) and afterwards one random LED now and then
  (`animations/raindrops_anim.h:32-41`, `jellybean_raindrops_anim.h:23-32`,
  `pixel_rain_anim.h:19-24`). Under these effects a colour written by a hook
  stays on the LED after the hook stops writing it, until the effect happens to
  redraw that LED. All three are switched on for `crkbd/rev4_1`.
- **An effect that is skipped for an LED leaves it alone.** The runners skip an
  LED whose flags do not match `rgb_matrix_config.flags`
  (`RGB_MATRIX_TEST_LED_FLAGS`, `rgb_matrix.h:114-115`). All LEDs are cleared
  once when the flags change (`rgb_matrix.c:295-298` in `vial-qmk`), not on
  every frame.
- **`DIGITAL_RAIN` finishes in one render call** and draws all LEDs in it
  (`animations/digital_rain_anim.h:22-80`, `return false`). The advanced hook is
  then called once per frame with the range of the first call only: LEDs 0-7 on
  the left half, and an empty range on the right half
  (`rgb_matrix_get_limits`, `rgb_matrix.c:425-451` in `vial-qmk`). The basic
  hook is not affected.
- **A split keyboard runs the task, and the hooks, on each half**, each for its
  own LEDs. The limits are clipped to the half (`rgb_matrix_get_limits`, as
  above). `rgb_matrix_config` and the suspend state are sent from the master to
  the other half (`quantum/split_common/transactions.c`, `qmk:574-589`,
  `vial-qmk:577-592`). The state an indicator shows is a separate question (see
  the map).

## 2. RGB Matrix off, brightness zero, suspended, past the timeout

| State | Effect number | Hooks called | LEDs |
| --- | --- | --- | --- |
| RGB Matrix switched off (`rgb_matrix_config.enable == 0`) | 0 | **No** | All off |
| Suspended, with `RGB_MATRIX_SLEEP` (set for `crkbd/rev4_1`) | 0 | **No** | All off |
| Past `RGB_MATRIX_TIMEOUT`, when it is above 0 (it is 0 here) | 0 | **No** | All off |
| Brightness (`hsv.v`) zero, RGB Matrix on | the effect's | **Yes** | The effect draws black; the indicator colours show as written |

- All three "off" states collapse into `effect = 0` in the line quoted in
  section 1, and the hooks sit behind `if (effect)`. Effect 0 is
  `rgb_matrix_none`, which writes black to every LED once
  (`rgb_matrix.c:244-251` in `vial-qmk`).
- Switched off covers `RM_OFF`/`RM_TOGG`, `rgb_matrix_disable*()`, and VialRGB's
  effect "off": `VIALRGB_EFFECT_OFF` calls `rgb_matrix_disable_noeeprom()`
  (`vial-qmk:quantum/vialrgb.c:66-74`).
- While switched off, `rgb_matrix_mode*()` and `rgb_matrix_sethsv*()` return
  without doing anything (`rgb_matrix.c:550-553`, `598-601` in `vial-qmk`).
- Suspend: `suspend_power_down_quantum` calls
  `rgb_matrix_set_suspend_state(true)` (`qmk:quantum/quantum.c:516-518`,
  `vial-qmk:quantum/quantum.c:592-594`), which does something only with
  `RGB_MATRIX_SLEEP`: it renders and flushes effect 0 at once and sets the flag
  (`rgb_matrix.c:499-507` in `vial-qmk`). Wake-up clears it (`qmk:quantum.c:554-556`,
  `vial-qmk:quantum.c:626-628`). Without `RGB_MATRIX_SLEEP` the function body is
  empty and the effect and the hooks keep running.
- Brightness zero does not make the effect number zero, so the hooks run. The
  hooks pass RGB values straight to the driver (section 3), so they light.

### Known ways to show an indicator anyway

1. **The documented one: keep RGB Matrix on and make the effect black.**
   `docs/features/rgb_matrix.md`, "Indicators without RGB Matrix Effect"
   (`qmk:767-775`, `vial-qmk:526-534`): "it is not possible to disable the
   latter because toggling RGB off will disable everything", with the
   workaround `rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
   rgb_matrix_sethsv_noeeprom(HSV_OFF);`. Brightness zero under any effect that
   scales by `hsv.v` is the same idea.
2. **Keep RGB Matrix on and set the LED flags so the effect skips the LEDs**
   (`rgb_matrix_set_flags(LED_FLAG_NONE)`; in `qmk` also the keycodes `RM_FLGN`
   / `RM_FLGP`). This follows from the source (section 1, second detail); the
   docs do not present it as an indicator technique. The LEDs are not cleared
   per frame in this state, so a hook has to write black itself when an
   indicator ends.
3. **While suspended or past the timeout there is no supported way** short of
   not defining `RGB_MATRIX_SLEEP` / leaving `RGB_MATRIX_TIMEOUT` at 0. Writing
   with `rgb_matrix_set_color` outside a hook is not one: the docs say it "can
   only be run from within an effect or indicator callback"
   (`qmk:docs/features/rgb_matrix.md:829`, `vial-qmk:588`), and in the off
   states nothing flushes the buffer again after effect 0 has been flushed once
   (`rgb_matrix.c:352-358` in `vial-qmk`).

No other mechanism was found in either source.

## 3. Does the brightness limit apply to colours set from a hook?

**No.** `RGB_MATRIX_MAXIMUM_BRIGHTNESS` is applied only where an HSV value is
stored:

- `rgb_matrix_sethsv_eeprom_helper` clamps `hsv.v` (`rgb_matrix.c:604` in
  `vial-qmk`; same line of code in `qmk`).
- VialRGB clamps the direct-mode colours (`vial-qmk:quantum/vialrgb.c:107-108`).
- It is the default brightness (`rgb_matrix.h:73-75`).

`rgb_matrix_set_color` passes the RGB values to the driver unchanged
(`vial-qmk:rgb_matrix.c:157-159`, `qmk:rgb_matrix.c:179-186`), and the WS2812
driver stores them unchanged
(`platforms/chibios/drivers/vendor/RP/RP2040/ws2812_vendor.c:271-278`, the file
is identical in both). A hook that writes `(255, 255, 255)` gets full
brightness, above the limit of 50 that the effects obey. The current effect
brightness is available as `rgb_matrix_get_val()`; the docs' own layer example
scales by hand (`docs/features/rgb_matrix.md`, "Indicator Examples").

## 4. What a build target without Vial gets when RGB Matrix is switched on

For `crkbd/rev4_1/mini:hydeik` after removing `RGB_MATRIX_ENABLE = no` from its
`rules.mk` (the keyboard itself has `"rgb_matrix": true`,
`keyboards/crkbd/rev4_1/info.json:18`).

**Defaults**, written when the EEPROM is reset or the stored mode is 0
(`eeconfig_update_rgb_matrix_default`, `qmk:quantum/eeconfig.c:127-130`, and
`rgb_matrix_init`):

| Setting | Value | Where |
| --- | --- | --- |
| On/off | on | `rgb_matrix.h:52-54` |
| Effect | `RGB_MATRIX_CYCLE_LEFT_RIGHT` (it is switched on for `crkbd/rev4_1`; else `SOLID_COLOR`) | `rgb_matrix.h:56-63` |
| Hue, saturation | 0, 255 | `rgb_matrix.h:65-71` |
| Brightness | `RGB_MATRIX_MAXIMUM_BRIGHTNESS`, 50 here | `rgb_matrix.h:73-75` |
| Speed | 127 | `rgb_matrix.h:77-79` |
| Flags | `LED_FLAG_ALL` | `rgb_matrix.h:81-83` |

**Storage.** One `rgb_config_t` (on/off 2 bits, effect 6 bits, HSV, speed,
flags; 8 bytes) in the EEPROM at `EECONFIG_RGB_MATRIX`
(`quantum/rgb_matrix/rgb_matrix_types.h:78-89`,
`quantum/nvm/eeprom/nvm_eeprom_eeconfig_internal.h:28,49`,
`quantum/nvm/eeprom/nvm_eeconfig.c:147-154`). On RP2040 the EEPROM is the
wear-levelling emulation on the flash
(`builddefs/common_features.mk`, the `RP2040` branch). The slot is shared with
LED Matrix (`docs/features/rgb_matrix.md`, "EEPROM storage"). Writes are
deferred: a change marks the config dirty and the task writes it in its
`SYNCING` state (`EECONFIG_DEBOUNCE_HELPER`, `qmk:quantum/eeconfig.h:166-209`;
`rgb_task_sync`). The `_noeeprom` functions change the running value only.

**Keycodes** (`qmk:quantum/process_keycode/process_rgb_matrix.c:10-115`,
`qmk:quantum/keycodes.h:1549-1563`; they act on key release unless
`RGB_TRIGGER_ON_KEYDOWN`, and Shift reverses the direction):

| Keycode | Does |
| --- | --- |
| `RM_ON`, `RM_OFF`, `RM_TOGG` | Switch RGB Matrix on, off, toggle |
| `RM_NEXT`, `RM_PREV` | Next / previous effect |
| `RM_HUEU`, `RM_HUED` | Hue |
| `RM_SATU`, `RM_SATD` | Saturation |
| `RM_VALU`, `RM_VALD` | Brightness, capped at `RGB_MATRIX_MAXIMUM_BRIGHTNESS` |
| `RM_SPDU`, `RM_SPDD` | Speed |
| `RM_FLGN`, `RM_FLGP` | Step through the LED flags (`qmk` only, see section 5) |

All of them write to the EEPROM. The underglow keycodes (`UG_*`,
`QK_UNDERGLOW_*`) also act on RGB Matrix, in both firmware sources, unless
`RGB_MATRIX_DISABLE_SHARED_KEYCODES` is defined
(`quantum/process_keycode/process_underglow.c:13-31` and the cases after it).

The firmware keymap of the `hydeik` keymap family holds none of these keycodes
today (`grep` for `RM_`, `RGB_`, `UG_` in `users/hydeik/layout.h` finds
nothing), and a keymap without Vial has no stored keymap and no host tool. So
with RGB Matrix merely switched on, the settings stay at the defaults above.

## 5. Differences between `qmk` and `vial-qmk` at the pinned commits

The complete `diff` of `quantum/rgb_matrix/rgb_matrix.c` and `rgb_matrix.h` was
read; `rgb_matrix_types.h`, `post_config.h` and `rgb_matrix_drivers.c` are
identical.

| Topic | `qmk` | `vial-qmk` |
| --- | --- | --- |
| Hook call sites and the `if (effect)` gate | `rgb_matrix.c:420-428` | `rgb_matrix.c:390-398`, same code |
| When the effect number is decided | Once per frame, in `rgb_task_start` (`rgb_matrix.c:315-324`) | On every task call (`rgb_matrix.c:376-384`) |
| `rgb_matrix_set_color` with an LED of the other half | Ignored: `rgb_matrix_led_index` returns `-1` (`rgb_matrix.c:159-186`) | **Not ignored**: on the right half an index below 20 is passed to the driver as it is and lights the wrong LED; on the left half an index of 20 or more is written into the unused part of the driver buffer (`rgb_matrix.c:148-159`) |
| LED flag stepping | `rgb_matrix_flags_step*()`, `RM_FLGN`, `RM_FLGP`, `RGB_MATRIX_FLAG_STEPS` | Absent; `rgb_matrix_set_flags*()` exists |
| `rgb_matrix_get_mode_name` | Present (`RGB_MATRIX_MODE_NAME_ENABLE`) | Absent |
| Effect list | Stock | Plus `VIALRGB_DIRECT`, and each effect header defines `RGB_MATRIX_EFFECT_<NAME>` for VialRGB's table |
| Host control | None in a keymap without Vial | VialRGB over raw HID (`quantum/via.c:317-362`); VIA's own RGB Matrix channel is compiled out when `VIALRGB_ENABLE` is set (`quantum/via.c:45-47`) |
| Defaults, storage, suspend, timeout, brightness clamp, `RM_*` up to `RM_SPDD` | — | Same code |

Consequences of the differences for an indicator:

- In `vial-qmk` a basic hook that runs on both halves must not write an LED of
  the other half; in `qmk` such a write is dropped. The advanced hook with
  `RGB_MATRIX_INDICATOR_SET_COLOR` is safe in both, because its range is
  clipped to the half.
- VialRGB changes the settings with the `_noeeprom` functions and stores them
  only on its save command (`vial-qmk:quantum/vialrgb.c:161-189`). Its effect
  "off" switches RGB Matrix off, which stops the hooks (section 2).
- The `RM_*` keycodes work in a keymap with Vial as well
  (`vial-qmk:quantum/quantum.c:429`), without the two flag keycodes.

## Not verified

- Nothing was built or flashed; the merged `max_brightness` of 50 and the LED
  count of 40 are read from the JSON, not from a generated `config.h`.
- That `suspend_power_down_quantum` is reached on USB suspend on RP2040 was
  taken as given; only the function itself was read.
- The other half's behaviour in VialRGB direct mode is inferred from the
  absence of any sync of `g_direct_mode_colors`.
- Effects other than those named in section 1 were not checked one by one for
  partial redraws.
