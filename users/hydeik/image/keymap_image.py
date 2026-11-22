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

import io
import re

import pcpp

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
        keys = _split_keys(line.group(1))
        if len(keys) != KEY_COUNT:
            raise KeymapImageError(
                f"Layer {name}: `{name}_LAYER` has to hold {KEY_COUNT} keys, but holds {len(keys)}."
            )
        layers[name] = keys
    return layers
