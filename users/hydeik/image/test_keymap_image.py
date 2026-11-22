"""Unit tests of keymap_image.py. Run with `make keymap-image-test`."""

import pathlib

import keymap_image
import pytest
from keymap_image import KeymapImageError

HERE = pathlib.Path(__file__).parent
LAYOUT_H = HERE.parent / "layout.h"


def header(layers, extra=""):
    """A header in the shape of layout.h: `enum layer_t` and one macro per layer.

    `layers` maps the name of a layer to its keys as C source.
    """
    enum = "".join(f"    _{name},\n" for name in layers)
    macros = "".join(
        f"#define {name}_LAYER \\\n    {', '.join(keys)}\n"
        for name, keys in layers.items()
    )
    return f"#pragma once\nenum layer_t {{\n{enum}}};\n{extra}\n{macros}"


def keys(overrides=None, fill="XXXXXXX"):
    """34 keys as C source: `fill` everywhere but at the overridden positions."""
    row = [fill] * 34
    for position, key in (overrides or {}).items():
        row[position] = key
    return row


# --- Reading layout.h -------------------------------------------------------


def test_layout_h_holds_seven_layers_of_34_keys():
    layers = keymap_image.read_layers(LAYOUT_H.read_text())

    assert list(layers) == ["BASE", "SYM", "NUM", "NAV", "MOU", "FUN", "MOD"]
    assert [len(layer) for layer in layers.values()] == [34] * 7


def test_the_keys_of_a_layer_are_read_in_the_order_of_its_macro():
    text = header({"BASE": keys({0: "KC_Q", 1: "KC_W", 33: "KC_ENT"}, fill="KC_A")})

    layers = keymap_image.read_layers(text)

    assert layers["BASE"][:3] == ["KC_Q", "KC_W", "KC_A"]
    assert layers["BASE"][33] == "KC_ENT"


def test_the_two_aliases_are_read_as_transparent_and_empty_keys():
    text = header({"BASE": keys({0: "_______", 1: "XXXXXXX"}, fill="KC_A")})

    layers = keymap_image.read_layers(text)

    assert layers["BASE"][:2] == ["KC_TRNS", "KC_NO"]


def test_layer_numbers_come_from_the_enum():
    text = header(
        {
            "BASE": keys({30: "TAB_SYM"}),
            "SYM": keys({29: "TRG_NAV"}),
            "NAV": keys(),
        },
        extra="#define TAB_SYM  LT(_SYM, KC_TAB)\n#define TRG_NAV  MO(_NAV)\n",
    )

    layers = keymap_image.read_layers(text)

    assert layers["BASE"][30] == "LT(1,KC_TAB)"
    assert layers["SYM"][29] == "MO(2)"


def test_a_layer_that_has_not_34_keys_stops_the_script():
    text = header({"BASE": keys()[:33]})

    with pytest.raises(KeymapImageError, match="BASE.*34.*33"):
        keymap_image.read_layers(text)


def test_a_header_without_the_layer_enum_stops_the_script():
    with pytest.raises(KeymapImageError, match="enum layer_t"):
        keymap_image.read_layers("#define BASE_LAYER KC_A\n")
