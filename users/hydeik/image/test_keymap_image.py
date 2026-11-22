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


# --- Marks ------------------------------------------------------------------


def keycodes(overrides=None, fill="KC_NO"):
    """34 keycodes as read_layers gives them."""
    return keys(overrides, fill)


def family():
    """Layers in the shape of the keymap family: the keys that carry marks."""
    thumbs = {30: "KC_TRNS", 31: "KC_TRNS", 32: "KC_TRNS", 33: "KC_TRNS"}
    return {
        "BASE": keycodes(
            {30: "LT(1,KC_TAB)", 31: "LT(6,KC_KP_0)", 32: "LT(6,KC_SPC)", 33: "LT(2,KC_ENT)"},
            fill="KC_A",
        ),
        "SYM": keycodes({20: "MO(4)", 29: "MO(3)", **thumbs, 33: "KC_ESC"}),
        "NUM": keycodes({29: "MO(5)", **thumbs, 30: "KC_ESC"}),
        "NAV": keycodes({10: "COS_LCTL", 20: "MO(4)", 29: "MO(3)", **thumbs}),
        "MOU": keycodes({20: "MO(4)", 29: "MO(3)", **thumbs, 32: "MS_BTN1", 33: "MS_BTN2"}),
        "FUN": keycodes({19: "COS_RCTL", 29: "MO(5)", **thumbs}),
        "MOD": keycodes({10: "COS_LCTL", 19: "COS_RCTL"}, fill="KC_TRNS"),
    }


def positions(marks, layer, mark):
    return sorted(position for position, m in marks[layer].items() if m == mark)


def test_the_holding_keys_are_marked_on_the_base_layer_and_nothing_else_is():
    marks = keymap_image.derive_marks(family())

    assert marks["BASE"] == {30: "holding", 33: "holding"}


def test_a_trigger_key_is_marked_on_every_layer_that_holds_it():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "SYM", "trigger") == [20, 29]
    assert positions(marks, "NUM", "trigger") == [29]
    assert positions(marks, "NAV", "trigger") == [20, 29]
    assert positions(marks, "MOU", "trigger") == [20, 29]
    assert positions(marks, "FUN", "trigger") == [29]


def test_a_one_shot_mod_key_is_marked():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "NAV", "one-shot-mod") == [10]
    assert positions(marks, "FUN", "one-shot-mod") == [19]
    assert positions(marks, "MOD", "one-shot-mod") == [10, 19]


def test_the_held_key_of_a_holding_layer_is_its_own_holding_key():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "SYM", "held") == [30]
    assert positions(marks, "NUM", "held") == [33]


def test_the_held_key_of_a_target_layer_is_the_holding_key_of_its_holding_layer():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "NAV", "held") == [30]
    assert positions(marks, "MOU", "held") == [30]
    assert positions(marks, "FUN", "held") == [33]


def test_both_smart_mod_layer_keys_are_held_on_the_smart_mod_layer():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "MOD", "held") == [31, 32]


def test_a_layer_that_no_key_leads_to_stops_the_script():
    layers = family()
    layers["SYM"][29] = "KC_NO"  # No trigger key of NAV on a holding layer.

    with pytest.raises(KeymapImageError, match="NAV"):
        keymap_image.derive_marks(layers)


def test_a_smart_mod_layer_without_a_smart_mod_layer_key_stops_the_script():
    layers = family()
    layers["BASE"][31] = "KC_A"
    layers["BASE"][32] = "KC_SPC"

    with pytest.raises(KeymapImageError, match="MOD"):
        keymap_image.derive_marks(layers)


def test_a_held_key_that_is_not_transparent_on_its_layer_stops_the_script():
    layers = family()
    layers["NAV"][30] = "KC_ESC"

    with pytest.raises(KeymapImageError, match="NAV.*KC_ESC"):
        keymap_image.derive_marks(layers)


def test_a_trigger_key_of_a_layer_that_does_not_exist_stops_the_script():
    layers = family()
    layers["SYM"][0] = "MO(9)"

    with pytest.raises(KeymapImageError, match=r"MO\(9\)"):
        keymap_image.derive_marks(layers)


def test_a_layer_tap_key_off_the_base_layer_stops_the_script():
    layers = family()
    layers["SYM"][0] = "LT(2,KC_A)"

    with pytest.raises(KeymapImageError, match=r"SYM.*LT\(2,KC_A\)"):
        keymap_image.derive_marks(layers)
