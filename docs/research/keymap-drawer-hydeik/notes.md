# Can keymap-drawer draw the hydeik keymap family from layout.h, and how?

Research notes for issue #68 (map: #67). Facts only; no decision of the map is
made here. This branch (`research/keymap-drawer-hydeik`) is throwaway.

Short answer: **yes**. `hydeik.svg` in this directory is all seven layers, 34
keys each, produced from `users/hydeik/layout.h` by two commands
(`qmk c2json`, then `keymap parse` + `keymap draw`), with no keyboard named in
the drawing step and no network access after the tool is installed.

Every statement is marked:

- **[ran]** verified by running it on this machine (macOS arm64, uv 0.13.9,
  QMK CLI 1.2.0, firmware source `qmk` at its pinned commit `80973aa`);
- **[docs]** read in a primary source, not run;
- **[unverified]** neither.

## Sources

- keymap-drawer v0.23.0, tag commit `a44809b`
  (<https://github.com/caksoylar/keymap-drawer>), the latest release on PyPI
  (2026-03-17): `README.md`, `KEYMAP_SPEC.md`, `CONFIGURATION.md`,
  `PHYSICAL_LAYOUTS.md`, `pyproject.toml`, and the source files named below.
- QMK: `docs/cli_commands.md` and `lib/python/qmk/keymap.py` of the pinned
  firmware source `qmk`.
- CI container: `Dockerfile` of <https://github.com/qmk/qmk_cli> and of
  <https://github.com/qmk/qmk_base_container>, and the bootstrap script
  <https://install.qmk.fm>, all as of 2026-11-22.

## Files in this directory

| File | What it is |
| --- | --- |
| `family_keymap.c` | Hand-written input for `qmk c2json`: includes `layout.h`, 34 keys per layer |
| `c2json.sh` | Step 1: `family_keymap.c` -> `family_keymap.json` |
| `family_keymap.json` | Output of step 1 |
| `config.yaml`, `base.yaml` | keymap-drawer configuration and base keymap of the configured run |
| `constraints.txt` | Exact versions of keymap-drawer and its 13 dependencies |
| `draw.sh` | Step 2: `family_keymap.json` -> `hydeik.yaml` -> `hydeik.svg` |
| `hydeik.yaml`, **`hydeik.svg`** | Output of step 2: the configured picture, two columns, light and dark style |
| `draw-out-of-the-box.sh`, `out-of-the-box.*` | The same with no configuration at all |
| `variants.sh`, `variants/` | Side experiments; `variants/output.txt` is their log |
| `layout-34.json` | A hand-made 34-key column-staggered physical layout in QMK `info.json` format |
| `check-linux-wheels.sh` | Resolves the install for Linux x86_64, wheels only |

To repeat (needs `uv`, the QMK CLI, and a firmware checkout from
`make firmware`):

```sh
PATH="$HOME/.local/share/qmk/bin:$PATH" ./c2json.sh   # QMK_HOME defaults to ../../../.firmware/qmk
./draw.sh
```

## 1. Input

- **[docs]** keymap-drawer draws from its own keymap YAML (`KEYMAP_SPEC.md`).
  `keymap parse` produces that YAML; for QMK it accepts **only a
  `keymap.json`** (`-q`), "exported from QMK Configurator, converted from
  `keymap.c` via `qmk c2json`, or from a VIA backup json via `qmk via2json`"
  (`README.md`). It does not read `keymap.c` or headers. The YAML can also be
  written by hand or by a script; `parse` is a convenience.
- **[ran]** `qmk c2json` on any of the three real keymaps fails or is useless:
  - with the preprocessor (default) it fails ("Something went wrong. Try to
    use --no-cpp."): c2json runs plain `cpp <file>` with no include paths
    (`_c_preprocess` in `lib/python/qmk/keymap.py`), so `#include "hydeik.h"`
    and `QMK_KEYBOARD_H` cannot be resolved;
  - with `--no-cpp` it returns the unexpanded text:
    `"layers": [["BASE_LAYER"], ["SYM_LAYER"], ...]`.

  Had the includes resolved, the layout adapter would have expanded
  `LAYOUT_HYDEIK_34` into the keyboard's own `LAYOUT` call with its extra
  `KC_NO`/`QK_BOOT` positions, i.e. a keymap of one keyboard. **None of the
  three keymaps is a usable starting point**; they also add nothing, since all
  three place the same seven macros.
- **[ran]** What works is a small separate C file, `family_keymap.c`, that
  includes `users/hydeik/layout.h` by relative path and calls an *undefined*
  `LAYOUT_HYDEIK_34(BASE_LAYER)` for each layer. `cpp` expands the layer
  macros and the key aliases (`TAB_SYM` -> `LT(_SYM, KC_TAB)`); c2json then
  takes the 34 arguments of each call as text. Result: 7 layers x 34 keys,
  in the order of `layout.h`.
- **[ran]** The layer numbers must reach keymap-drawer as digits: its QMK
  parser matches `MO\((\d+)\)` and `LT\((\d+),(\S+)\)`
  (`keymap_drawer/parse/qmk.py`). With enum names left in, `LT(_SYM, KC_TAB)`
  is drawn as the literal text `LT( SYM,TAB)` and no held key is detected
  (`variants.sh` no. 6). `family_keymap.c` therefore `#define`s `_BASE` ..
  `_MOD` as 0 .. 6 before including `layout.h`. This duplicates the enum of
  `layout.h` (a second place to keep in step); rewriting the JSON after c2json
  would be the other way.
- **[ran]** Constraints of `qmk c2json`: it is a subcommand that lives in the
  firmware checkout, so `QMK_HOME` must point at one (without it: "invalid
  choice: 'c2json'"); `-kb` and `-km` are required for a file outside the
  firmware tree, and `-kb` must be a keyboard that exists in that checkout
  (`mothwing` and `crkbd/rev4_1/mini` both do). The keyboard name only ends up
  in the JSON's `keyboard` field. c2json writes `XXXXXXX` as `KC_NO` and
  `_______` as `KC_TRNS`.
- **[docs]** QMK calls c2json fragile: "Parsing C source files is not easy,
  therefore this subcommand may not work with your keymap"
  (`docs/cli_commands.md`).
- **[unverified]** That GNU `cpp` on Linux gives c2json the same result as
  Apple's `cpp` here. No "compiled keymap" route was found or tried.

## 2. Custom keys

Out of the box (`out-of-the-box.yaml`, `out-of-the-box.ortho.svg`) — all **[ran]**:

| Key | Drawn as |
| --- | --- |
| One-shot mod keys `COS_LCTL` .. (custom keycodes) | The keycode with `_` -> space, on two lines: `COS LCTL`. Unknown to the tool. |
| Holding keys `LT(1, KC_TAB)`, `LT(2, KC_ENT)` | Tap legend `TAB` / `ENT` in the centre, layer name `SYM` / `NUM` small at the bottom, underlined |
| Smart mod layer keys `LT(6, KC_SPC)`, `LT(6, KC_KP_0)` | `SPC` over `MOD`; `SMT_MOD` shows its never-sent tap keycode as `KP 0` |
| Trigger keys `MO(3)` .. | The layer name `NAV` / `MOU` / `FUN`, underlined |
| Layer Lock `QK_LLCK` | `QK LLCK` (unknown to the tool) |
| Mouse keys `MS_WHLL`, `MS_BTN1`, .. | `MS WHLL`, `MS BTN1`, .. (unknown) |
| `QK_BOOT` | `QK BOOT` (unknown) |
| Transparent `KC_TRNS` | `▽` in lighter text (class `trans`) |
| `KC_NO` | An empty key |

- Layer names come from `--layer-names` (c2json loses them; default `L0`..).
  The underlined layer names are `<a href="#NAV">` links to the layer header
  inside the SVG (`style_layer_activators`, default on).

**Held keys per layer [ran]:** the parser marks them by itself, from `MO`/`LT`
(also `OSL`, `TT`), with `type: held` (red key). It records the *first* way
into each layer, plus the keys held to reach the layer that key is on
(`update_layer_activated_from` in `parse/parse.py`). For this keymap:

| Layer | Marked held (key index) |
| --- | --- |
| SYM | `TAB_SYM` (30) |
| NUM | `ENT_NUM` (33) |
| NAV | `TAB_SYM` (30) and the trigger key `MO(NAV)` (29) |
| MOU | `TAB_SYM` (30) and the trigger key `MO(MOU)` (20) |
| FUN | `ENT_NUM` (33) and the trigger key `MO(FUN)` (29) |
| MOD | `SMT_MOD` (31) only; `SPC_MOD` (32) is drawn as transparent |

So the tool reads a trigger key as a momentary key that is being held, which
is not what the firmware does. A held key that is transparent loses its `▽`
and is drawn as an empty red key.

- `mark_alternate_layer_activators: true` adds `type: "held alternate"` for
  further ways in: `SPC_MOD` on MOD, but also the `NAV` trigger key on MOU
  (`variants.sh` no. 4).

With configuration (`config.yaml`, `hydeik.svg`) — **[ran]** unless marked:

- `parse_config.raw_binding_map` maps a keycode string, exactly as c2json
  writes it (`"LT(6, KC_KP_0)"` with the space), to a legend or to a full key
  spec `{t, h, s, left, right, tl, tr, bl, br, type}`. This fixes every
  "unknown" row above. `type` becomes a CSS class of the key, so one-shot mod
  keys can be given their own colour through `draw_config.svg_extra_style`
  (class `osm` in the experiment). A space in a legend is a line break.
- A `raw_binding_map` match skips all other parsing of that key, **including
  held-key detection**: with `SMT_MOD` mapped this way, the held key of MOD
  moves from `SMT_MOD` (31) to `SPC_MOD` (32).
- `parse_config.qmk_keycode_map` in a config file **replaces** the built-in
  map instead of extending it: with only `KP_0` given, `;` is drawn `SCLN`
  and `KC_NO` as `NO` (`variants.sh` no. 5). Its ~70 defaults would have to be
  restated.
- `parse_config.trans_legend` sets the legend and class of transparent keys;
  `layer_legend_map` renames a layer on the keys that lead to it **[docs]**.
- **Marking a held key by hand** has no configuration option. Tried:
  `keymap parse -b base.yaml` keeps a field of the base keymap only where the
  new parse leaves that field empty. It works for a key with no parsed `type`
  (the class `holding` on the two holding keys of BASE survives), and does
  **not** work on a transparent key, whose parsed `type: trans` wins (the
  `held` for `SMT_MOD` on MOD in `base.yaml` is dropped). What remains is
  editing the YAML between `parse` and `draw` (a script of our own), or
  writing the YAML directly. `keymap draw a.yaml b.yaml` merges whole layers
  by name, not single keys **[docs: `__main__.py`]**.
- Combos, tap dances: not relevant (out of scope).

## 3. Physical layout

A 34-key split can be drawn without naming a keyboard in three ways — **[ran]**:

- `ortho_layout: {split: true, rows: 3, columns: 5, thumbs: 2}` — a plain
  grid, two inner thumb keys per half. Used for `hydeik.svg`.
- `cols_thumbs_notation: "33333+2 2+33333"` — byte-identical output to the
  above. Its stagger modifiers are of little use here: a spread of a whole key
  (`v` and `^` together) puts legends on the wrong keys, because the generator
  orders keys by `int(y)` (`variants/cols-thumbs-stagger.svg`); a spread of
  half a key keeps the order but the thumb row overlaps the lowered columns
  (`variants/cols-thumbs-half-stagger.svg`).
- `qmk_info_json: <local file>` — any positions, including column stagger and
  rotation (`x`, `y`, `r`, `rx`, `ry`); `layout-34.json` is a 34-key example
  (`variants/qmk-info-json.svg`). `dts_layout` (ZMK devicetree) is the same
  idea **[docs]**.

Other facts:

- **[ran]** `-k generic/split_3x5_2` draws on QMK's community layout of that
  name, fetched from `raw.githubusercontent.com/qmk/qmk_firmware/master/...`
  on first use and then cached in the user cache directory. It is in the
  source (`physical_layout.py`) but not in the documentation, and needs the
  network. `-k ferris/sweep` works too (it resolves to `ferris_rotated`, one
  of the ~50 layouts bundled with the tool, so no network), but names a
  keyboard; a keyboard that is not bundled is fetched from the QMK metadata
  API **[docs]**.
- **[ran]** The parsed YAML carries `layout: {qmk_keyboard: mothwing,
  layout_name: LAYOUT_HYDEIK_34}` from the JSON. With `--ortho-layout` or `-n`
  this only prints a warning on stderr; with `-j` or `-k` the draw fails
  unless `-l <layout name>` is given.
- **[ran]** `draw_config.n_columns` lays the layers out side by side (2 in
  `hydeik.svg`: 1380 x 1176; one column: 690 x 2016). `-s` draws chosen
  layers only. One SVG holds all layers.

## 4. Output

- **[docs]** SVG is the only output format (`README.md` points to CairoSVG or
  Inkscape for anything else).
- **[ran] Same bytes on every run:** `draw.sh` run twice gives identical
  `hydeik.yaml` and `hydeik.svg` (`cmp`); sha256 of `hydeik.svg`:
  `be5c3d5b636e6c8afb63e893bb487a3e3681555a13fb34f0a0c9ab8f30dd712d`.
- **[ran]** Same bytes under Python 3.12, 3.13 and 3.14 with the dependencies
  pinned by `constraints.txt` (`variants.sh` no. 8).
- **[unverified] Same bytes on every machine.** Only this one machine was
  used. For it: the drawer writes the SVG with plain string formatting, no
  timestamp, no version stamp, no random id, and no embedded font
  (`draw/draw.py`); no glyph is fetched unless a legend uses `$$...$$`.
  Against it: not tried on Linux (step 1 runs the system `cpp`); an unpinned
  dependency (pyyaml, pydantic) could change the YAML or number formatting;
  on Windows the documentation warns about text encoding when redirecting
  stdout, so `-o` is recommended.
- **[ran]** The picture is *rendered* with the viewer's fonts: the SVG names
  the font stack `SFMono-Regular,Consolas,Liberation Mono,Menlo,monospace`
  and embeds none, so it looks slightly different per machine while the bytes
  stay the same. Arrows such as `←` in a legend depend on that font.
- **Dark theme [ran]:** `draw_config.dark_mode` is `false` (default), `true`
  (dark colours always, `variants/dark-env.svg`) or `"auto"`, which appends
  the dark overrides inside `@media (prefers-color-scheme: dark)`.
  `hydeik.svg` uses `auto`. The SVG has no background rectangle
  (transparent). `svg_extra_style` is appended *after* the dark block, so
  colours added there need their own `@media` block for the dark theme (as in
  `config.yaml`).
- **[unverified]** How GitHub shows such an SVG in a README in its dark
  theme. An SVG shown as an image follows the *browser's*
  `prefers-color-scheme`, which need not match the theme chosen in GitHub's
  settings; this was not tested. GitHub's documentation states that the
  `<picture>` element is supported in Markdown **[docs]**, which would allow
  two fixed-theme files, against the map's "one image file". The deleted
  family's image (`git show 71f2869^:users/hydeik/images/rendered_keymap.svg`)
  was keymap-drawer output with the `auto` media query in it.

## 5. Installing and pinning

- **[docs]** On PyPI as `keymap-drawer`; the command is `keymap`. The README
  recommends `pipx install keymap-drawer`, or `pip install` in a virtual
  environment. v0.23.0 requires Python `>=3.12,<4.0` (0.21.0 and older:
  `>=3.10`). Pure Python itself; of its dependencies `pydantic-core`,
  `pyyaml`, `tree-sitter` and `tree-sitter-devicetree` are compiled.
- **[ran]** Without installing anything globally:
  `uvx --from keymap-drawer==0.23.0 keymap ...` (`uvx keymap-drawer` does not
  work: the package and the command have different names). The version is
  pinned by `==0.23.0` in the command.
- **[ran]** That pins keymap-drawer only. `uvx ... --constraints
  constraints.txt` pins the 13 dependencies as well; `constraints.txt` was
  made with `uv pip compile --universal`. `pip install -c constraints.txt
  keymap-drawer==0.23.0` is the pip equivalent **[docs]**.
- **CI container** (`ghcr.io/qmk/qmk_cli:latest`, used by the `build` and
  `test` jobs):
  - **[docs]** It is Debian 13 with a uv-managed virtual environment at
    `/opt/uv/tools/qmk` on `PATH`, created by the QMK bootstrap script with
    Python 3.14 by default and with `pip` in it (the workflow already runs
    `pip install -r qmk_firmware/requirements.txt` there). The `uv` binary is
    installed in the build stage only; the final stages copy `/opt/uv` and
    `/opt/qmk`, not `/usr/local/bin`, so `uv`/`uvx` is probably **not**
    available in the image. `pipx` is not installed.
  - **[ran]** `check-linux-wheels.sh`: for Linux x86_64 and Python 3.13 and
    3.14, keymap-drawer 0.23.0 and all dependencies resolve with binary
    wheels only, so `pip install` needs no compiler there.
  - **[unverified]** An actual install and run inside the container: no
    Docker or Podman on this machine. Also unverified: which stage the
    `latest` tag is built from, and the Python version in today's image.
  - **[docs]** Step 1 needs the firmware checkout and `cpp`; the `build` and
    `test` jobs have both (gcc is in the image).
- **[ran]** Side effects of the tool on the machine: uv's cache; and
  `~/.cache/keymap-drawer` for fetched keyboard layouts and glyphs, only when
  `-k` or `$$...$$` glyphs are used (`use_local_cache`).

## 6. Alternatives

Nothing above failed outright; the weak points are the c2json step (fragile by
QMK's own words, needs a firmware checkout and the duplicated layer numbers)
and marking held keys by hand. In brief:

- **A script of our own in front of keymap-drawer, instead of `qmk c2json` and
  `keymap parse` [unverified, not built].** Preprocess `layout.h` (system
  `cpp`, or `pcpp`, the pure-Python preprocessor keymap-drawer already depends
  on), split the 34 keys per layer, and write the keymap YAML directly with
  the legends, classes and held keys wanted. Needs no firmware checkout and no
  QMK CLI; `keymap draw` then only draws. The enum of `layout.h` could be read
  by the same script.
- **A script between `parse` and `draw` [unverified, not built]** that edits
  single keys of the YAML (held marks). keymap-drawer's README lists one user
  doing this (infused-kim's `keymap_img_adjuster.py`).
- **[docs]** Other tools named in keymap-drawer's README under "Related
  projects": Keymapviz (<https://github.com/yskoht/keymapviz>), callum-oakley's
  original `keymap`, jbarr21's `keymap-display` (a `keymap.c` -> keymap-drawer
  YAML converter). None was tried.
- **[unverified]** A wholly own SVG generator: 238 rectangles and legends; the
  format keymap-drawer emits is plain enough to imitate. Not tried.
