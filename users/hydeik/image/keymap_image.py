"""Writes the keymap image of the keymap family `hydeik` from layout.h.

Run with `make keymap-image`. The route from `users/hydeik/layout.h` to
`users/hydeik/image/keymap.svg`:

1. `layout.h` is preprocessed with `pcpp`, the pure-Python C preprocessor that
   keymap-drawer depends on, into a QMK keymap JSON of seven layers of 34
   keys.
2. `keymap parse` turns the keycodes into legends.
3. The marks (one-shot mod keys, holding keys, trigger keys and held keys) are
   set on the parsed YAML. They are derived from the keycodes of `layout.h`.
4. `keymap draw` writes the SVG.

Why a script of our own and not `qmk c2json`: `qmk c2json` lives in a firmware
checkout and needs the QMK CLI and a valid keyboard name, it cannot read
`layout.h` itself but only a separate C file that places the layers, and that
file has to repeat the layer numbers of `enum layer_t` as digits. This script
needs `uv` and nothing else, reads the layer numbers from the enum, and has to
exist anyway: keymap-drawer marks a trigger key as a held momentary layer key
and only one of the two smart mod layer keys, so the marks have to be set
between `parse` and `draw`.

keymap-drawer is called as a command, not as a library: the command line is
its documented interface. The intermediate files go to a temporary directory.
"""

import argparse
import io
import json
import pathlib
import re
import subprocess
import sys
import tempfile

import pcpp
import yaml

KEY_COUNT = 34


class KeymapImageError(Exception):
    """The keymap image cannot be written; the message says why."""


def _preprocess(text):
    preprocessor = pcpp.Preprocessor()
    preprocessor.line_directive = None
    preprocessor.parse(text, "layout.h")
    output = io.StringIO()
    preprocessor.write(output)
    return output.getvalue()


def _layer_names(layout_h):
    """The names of the layers in the order of `enum layer_t`, without the `_`."""
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", _preprocess(layout_h), flags=re.DOTALL)
    enum = re.search(r"enum\s+layer_t\s*\{([^}]*)\}", text)
    if enum is None:
        raise KeymapImageError(
            "layout.h has no `enum layer_t` to read the layer numbers from."
        )
    entries = [entry.strip() for entry in enum.group(1).split(",") if entry.strip()]
    for entry in entries:
        if not re.fullmatch(r"_[A-Z0-9]+", entry):
            raise KeymapImageError(
                f"`enum layer_t` has the entry `{entry}`; "
                "every entry has to be a plain name such as `_BASE`, without a value."
            )
    return [entry[1:] for entry in entries]


def _split_keys(text):
    """The comma-separated keycodes of a layer, each without whitespace."""
    keys, depth, current = [], 0, ""
    for character in text:
        if character == "," and depth == 0:
            keys.append(current)
            current = ""
            continue
        depth += {"(": 1, ")": -1}.get(character, 0)
        current += character
    keys.append(current)
    return ["".join(key.split()) for key in keys]


def read_layers(layout_h):
    """The layers of `layout.h`: the name of each layer and its 34 keycodes.

    The layers are in the order of `enum layer_t`, so the position of a layer
    is its layer number. A keycode is as the C preprocessor expands it, without
    whitespace, with a layer number in place of each name of the enum, and
    with `KC_TRNS` and `KC_NO` for `_______` and `XXXXXXX`: the form that
    keymap-drawer reads from a QMK keymap JSON.
    """
    names = _layer_names(layout_h)
    numbers = "".join(
        f"#define _{name} {number}\n" for number, name in enumerate(names)
    )
    aliases = "#define _______ KC_TRNS\n#define XXXXXXX KC_NO\n"
    # One line per layer after the header: a string literal that names the
    # layer, followed by its macro for the preprocessor to expand.
    requests = "".join(f'"layer {name}" {name}_LAYER\n' for name in names)
    output = _preprocess(numbers + aliases + layout_h + "\n" + requests)

    layers = {}
    for name in names:
        line = re.search(rf'^"layer {name}"(.*)$', output, flags=re.MULTILINE)
        if line is None:
            raise KeymapImageError(
                f"Layer {name}: the preprocessor did not expand `{name}_LAYER`."
            )
        keys = _split_keys(line.group(1))
        if len(keys) != KEY_COUNT:
            raise KeymapImageError(
                f"Layer {name}: `{name}_LAYER` has to hold {KEY_COUNT} keys, but holds {len(keys)}."
            )
        layers[name] = keys
    return layers


# The marks of a key. Each is the `type` of the key in the keymap-drawer YAML
# and so a CSS class of the key in the image; keymap_drawer.yaml gives the
# first three their colours, and keymap-drawer has its own colour for `held`.
ONE_SHOT_MOD = "one-shot-mod"
HOLDING = "holding"
TRIGGER = "trigger"
HELD = "held"

# The name of the smart mod layer in `enum layer_t`, without the `_`. A
# layer-tap key of this layer is a smart mod layer key; a layer-tap key of any
# other layer is a holding key.
SMART_MOD_LAYER = "MOD"

TRANSPARENT = "KC_TRNS"

_TRIGGER_KEY = re.compile(r"MO\((\d+)\)")
_LAYER_TAP_KEY = re.compile(r"LT\((\d+),(.+)\)")
_ONE_SHOT_MOD_KEY = re.compile(r"COS_\w+")


def _layer_of(match, keycode, names):
    number = int(match.group(1))
    if number >= len(names):
        raise KeymapImageError(
            f"Cannot derive the marks: `{keycode}` names layer {number}, "
            f"but there are only the layers 0 to {len(names) - 1}."
        )
    return names[number]


def derive_marks(layers):
    """The marks of the keys, derived from the keycodes: `{layer: {position: mark}}`.

    `layers` is what `read_layers` returns; the first layer is the base layer.

    - `COS_*` is a one-shot mod key and `MO(layer)` a trigger key, wherever
      they are.
    - `LT(layer, ...)` on the base layer is a holding key, or a smart mod
      layer key if the layer is the smart mod layer.
    - The held key of a holding layer is its holding key. The held key of a
      target layer is the holding key of its holding layer: the holding layer
      that holds the trigger key of the target layer. The held keys of the
      smart mod layer are all smart mod layer keys.

    A trigger key is never held. Raises KeymapImageError if the marks cannot be
    derived.
    """
    names = list(layers)
    base = names[0]
    marks = {name: {} for name in names}
    holding_keys = {
        name: [] for name in names
    }  # holding layer -> positions on the base layer
    smart_mod_layer_keys = []
    trigger_keys = {
        name: set() for name in names
    }  # target layer -> layers that hold its trigger key

    for name, keycodes in layers.items():
        for position, keycode in enumerate(keycodes):
            if _ONE_SHOT_MOD_KEY.fullmatch(keycode):
                marks[name][position] = ONE_SHOT_MOD
            elif match := _TRIGGER_KEY.fullmatch(keycode):
                trigger_keys[_layer_of(match, keycode, names)].add(name)
                marks[name][position] = TRIGGER
            elif match := _LAYER_TAP_KEY.fullmatch(keycode):
                layer = _layer_of(match, keycode, names)
                if name != base:
                    raise KeymapImageError(
                        f"Cannot derive the marks: layer {name} holds `{keycode}`, "
                        f"but a holding key or smart mod layer key is expected on layer {base} only."
                    )
                if layer == SMART_MOD_LAYER:
                    smart_mod_layer_keys.append(position)
                else:
                    holding_keys[layer].append(position)
                    marks[name][position] = HOLDING

    for name in names[1:]:
        if holding_keys[name]:
            held, reason = holding_keys[name], "its holding key"
        elif name == SMART_MOD_LAYER:
            held, reason = smart_mod_layer_keys, "a smart mod layer key"
            if not held:
                raise KeymapImageError(
                    f"Cannot derive the held keys of layer {name}: "
                    f"layer {base} holds no smart mod layer key `LT({names.index(name)}, ...)`."
                )
        else:
            held = sorted(
                position
                for holding_layer in trigger_keys[name]
                for position in holding_keys[holding_layer]
            )
            reason = "the holding key of its holding layer"
            if not held:
                raise KeymapImageError(
                    f"Cannot derive the held key of layer {name}: no holding layer holds "
                    f"its trigger key `MO({names.index(name)})`, and layer {base} holds "
                    f"no holding key `LT({names.index(name)}, ...)`."
                )
        for position in held:
            keycode = layers[name][position]
            if keycode != TRANSPARENT:
                raise KeymapImageError(
                    f"Cannot derive the marks: position {position} of layer {name} is held "
                    f"({reason}), but layer {name} holds `{keycode}` there instead of a transparent key."
                )
            marks[name][position] = HELD
    return marks


def number_layers(config, names):
    """The keymap-drawer configuration with layer numbers in its keycodes.

    The keycodes that `parse_config.raw_binding_map` gives a legend to are
    written like in `layout.h`, with the name of a layer (`LT(_MOD, KC_KP_0)`).
    keymap-drawer matches them against the keycodes of the QMK keymap JSON, so
    they get the form that `read_layers` returns: the layer number of
    `names`, and no whitespace.
    """
    numbers = {f"_{name}": str(number) for number, name in enumerate(names)}

    def numbered(keycode):
        keycode = re.sub(
            r"\b_[A-Z0-9]+\b",
            lambda name: numbers.get(name.group(), name.group()),
            keycode,
        )
        return "".join(keycode.split())

    parse_config = dict(config.get("parse_config", {}))
    parse_config["raw_binding_map"] = {
        numbered(keycode): legend
        for keycode, legend in parse_config.get("raw_binding_map", {}).items()
    }
    return {**config, "parse_config": parse_config}


def _has_legend(keycode, parse_config):
    if keycode in parse_config["raw_binding_map"]:
        return True
    if _TRIGGER_KEY.fullmatch(keycode):
        return True  # Its legend is the name of its target layer.
    if match := _LAYER_TAP_KEY.fullmatch(keycode):
        return _has_legend(match.group(2), parse_config)
    # A basic keycode: keymap-drawer removes the prefix and draws the rest, or
    # the entry of its own table for it.
    return keycode in parse_config["qmk_keycode_map"] or any(
        keycode.startswith(prefix)
        for prefix in parse_config["qmk_remove_keycode_prefix"]
    )


def check_legends(layers, parse_config):
    """Stops with an error that names the first keycode without a legend.

    `parse_config` is the `parse_config` that keymap-drawer works with, its
    defaults included (`keymap dump-config`). A keycode has a legend if the
    configuration gives it one in `raw_binding_map`, or if keymap-drawer
    knows it: a basic keycode (`KC_*`) or an entry of its own table. For any
    other keycode keymap-drawer would draw the raw text, such as `COS FOO`.
    """
    for name, keycodes in layers.items():
        for keycode in keycodes:
            if not _has_legend(keycode, parse_config):
                raise KeymapImageError(
                    f"Layer {name} holds `{keycode}`, which has no legend: keymap-drawer does not "
                    "know it, and `parse_config.raw_binding_map` of keymap_drawer.yaml does not "
                    "list it. Give it a legend there."
                )


def apply_marks(keymap, layers, marks, trans_legend):
    """The keymap-drawer YAML of `keymap parse` with the marks set on its keys.

    keymap-drawer marks keys as held by a guess of its own, which takes a
    trigger key for a held momentary layer key; that guess is dropped and
    `marks` decides. A held key has no legend. `trans_legend` is what
    keymap-drawer writes for a transparent key.
    """
    marked_layers = {}
    for name, parsed_keys in keymap["layers"].items():
        marked_layers[name] = []
        for position, parsed in enumerate(parsed_keys):
            key = dict(parsed) if isinstance(parsed, dict) else {"t": parsed}
            if key.get("type") == HELD:
                del key["type"]
                if layers[name][position] == TRANSPARENT:
                    key = dict(trans_legend)
            mark = marks[name].get(position)
            if mark == HELD:
                key = {"type": HELD}
            elif mark:
                key["type"] = mark
            marked_layers[name].append(key["t"] if list(key) == ["t"] else key)
    return {**keymap, "layers": marked_layers}


HERE = pathlib.Path(__file__).resolve().parent
LAYOUT_H = HERE.parent / "layout.h"
CONFIG = HERE / "keymap_drawer.yaml"
PHYSICAL_LAYOUT = HERE / "layout.json"
IMAGE = HERE / "keymap.svg"


def _keymap_drawer(*arguments):
    """Runs the `keymap` command of keymap-drawer and returns what it prints."""
    # keymap-drawer 0.23.0 calls a deprecated function of pydantic on every run.
    command = [
        sys.executable,
        "-W",
        "ignore::DeprecationWarning",
        "-m",
        "keymap_drawer",
        *map(str, arguments),
    ]
    result = subprocess.run(
        command, stdout=subprocess.PIPE, encoding="utf-8", check=False
    )
    if result.returncode != 0:
        raise KeymapImageError(
            f"keymap-drawer failed: `keymap {' '.join(map(str, arguments))}`."
        )
    return result.stdout


def write_image(layout_h, image):
    """Writes the keymap image of the layers in the file `layout_h` to the file `image`."""
    layers = read_layers(layout_h.read_text(encoding="utf-8"))
    marks = derive_marks(layers)

    with tempfile.TemporaryDirectory() as directory:
        work = pathlib.Path(directory)

        config = work / "keymap_drawer.yaml"
        numbered = number_layers(
            yaml.safe_load(CONFIG.read_text(encoding="utf-8")), list(layers)
        )
        config.write_text(
            yaml.safe_dump(numbered, allow_unicode=True, sort_keys=False),
            encoding="utf-8",
        )
        parse_config = yaml.safe_load(_keymap_drawer("-c", config, "dump-config"))[
            "parse_config"
        ]
        check_legends(layers, parse_config)

        qmk_keymap = work / "keymap.json"
        qmk_keymap.write_text(
            json.dumps({"layers": list(layers.values())}), encoding="utf-8"
        )
        parsed = work / "parsed.yaml"
        _keymap_drawer(
            "-c",
            config,
            "parse",
            "-q",
            qmk_keymap,
            "--layer-names",
            *layers,
            "-o",
            parsed,
        )

        keymap = apply_marks(
            yaml.safe_load(parsed.read_text(encoding="utf-8")),
            layers,
            marks,
            parse_config["trans_legend"],
        )
        marked = work / "marked.yaml"
        marked.write_text(
            yaml.safe_dump(keymap, allow_unicode=True, sort_keys=False),
            encoding="utf-8",
        )

        drawn = work / "keymap.svg"
        _keymap_drawer("-c", config, "draw", "-j", PHYSICAL_LAYOUT, marked, "-o", drawn)
        image.write_bytes(drawn.read_bytes())


def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Write the keymap image of the keymap family `hydeik`."
    )
    parser.add_argument(
        "-o",
        "--output",
        type=pathlib.Path,
        default=IMAGE,
        help="the SVG to write (default: %(default)s)",
    )
    options = parser.parse_args(arguments)
    try:
        write_image(LAYOUT_H, options.output)
    except KeymapImageError as error:
        print(f"keymap_image: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
