"""Unit tests of keymap_image.py. Run with `make keymap-image-test`."""

import keymap_image
import pytest
from keymap_image import (
    HELD,
    HOLDING,
    IMAGE,
    KEY_COUNT,
    LAYOUT_H,
    ONE_SHOT_MOD,
    TRIGGER,
    KeymapImageError,
)


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


def keys(overrides=None, fill="KC_NO"):
    """The keys of a layer: `fill` everywhere but at the overridden positions.

    They serve as C source for `header` and as what `read_layers` returns.
    """
    row = [fill] * KEY_COUNT
    for position, key in (overrides or {}).items():
        row[position] = key
    return row


# --- Reading layout.h -------------------------------------------------------


def test_layout_h_holds_seven_layers_of_34_keys():
    layers = keymap_image.read_layers(LAYOUT_H.read_text())

    assert list(layers) == ["BASE", "SYM", "NUM", "NAV", "MOU", "FUN", "MOD"]
    assert [len(layer) for layer in layers.values()] == [KEY_COUNT] * 7


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
    text = header({"BASE": keys()[: KEY_COUNT - 1]})

    with pytest.raises(KeymapImageError, match="BASE.*34.*33"):
        keymap_image.read_layers(text)


def test_a_header_without_the_layer_enum_stops_the_script():
    with pytest.raises(KeymapImageError, match="enum layer_t"):
        keymap_image.read_layers("#define BASE_LAYER KC_A\n")


# --- Marks ------------------------------------------------------------------


def family():
    """Layers in the shape of the keymap family: the keys that carry marks."""
    thumbs = {30: "KC_TRNS", 31: "KC_TRNS", 32: "KC_TRNS", 33: "KC_TRNS"}
    return {
        "BASE": keys(
            {
                30: "LT(1,KC_TAB)",
                31: "LT(6,KC_KP_0)",
                32: "LT(6,KC_SPC)",
                33: "LT(2,KC_ENT)",
            },
            fill="KC_A",
        ),
        "SYM": keys({20: "MO(4)", 29: "MO(3)", **thumbs, 33: "KC_ESC"}),
        "NUM": keys({29: "MO(5)", **thumbs, 30: "KC_ESC"}),
        "NAV": keys({10: "COS_LCTL", 20: "MO(4)", 29: "MO(3)", **thumbs}),
        "MOU": keys({20: "MO(4)", 29: "MO(3)", **thumbs, 32: "MS_BTN1", 33: "MS_BTN2"}),
        "FUN": keys({19: "COS_RCTL", 29: "MO(5)", **thumbs}),
        "MOD": keys({10: "COS_LCTL", 19: "COS_RCTL"}, fill="KC_TRNS"),
    }


def positions(marks, layer, mark):
    return sorted(position for position, m in marks[layer].items() if m == mark)


def test_the_holding_keys_are_marked_on_the_base_layer_and_nothing_else_is():
    marks = keymap_image.derive_marks(family())

    assert marks["BASE"] == {30: HOLDING, 33: HOLDING}


def test_a_trigger_key_is_marked_on_every_layer_that_holds_it():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "SYM", TRIGGER) == [20, 29]
    assert positions(marks, "NUM", TRIGGER) == [29]
    assert positions(marks, "NAV", TRIGGER) == [20, 29]
    assert positions(marks, "MOU", TRIGGER) == [20, 29]
    assert positions(marks, "FUN", TRIGGER) == [29]


def test_a_one_shot_mod_key_is_marked():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "NAV", ONE_SHOT_MOD) == [10]
    assert positions(marks, "FUN", ONE_SHOT_MOD) == [19]
    assert positions(marks, "MOD", ONE_SHOT_MOD) == [10, 19]


def test_the_held_key_of_a_holding_layer_is_its_own_holding_key():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "SYM", HELD) == [30]
    assert positions(marks, "NUM", HELD) == [33]


def test_the_held_key_of_a_target_layer_is_the_holding_key_of_its_holding_layer():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "NAV", HELD) == [30]
    assert positions(marks, "MOU", HELD) == [30]
    assert positions(marks, "FUN", HELD) == [33]


def test_both_smart_mod_layer_keys_are_held_on_the_smart_mod_layer():
    marks = keymap_image.derive_marks(family())

    assert positions(marks, "MOD", HELD) == [31, 32]


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


# --- Legends ----------------------------------------------------------------


def parse_config(raw_binding_map):
    """The part of the keymap-drawer configuration that decides the legends.

    In the shape of `keymap dump-config`: the table of keymap-drawer is keyed
    by the keycode without its prefix.
    """
    return {
        "raw_binding_map": raw_binding_map,
        "qmk_remove_keycode_prefix": ["KC_"],
        "qmk_keycode_map": {
            "XXXXXXX": "",
            "NO": "",
            "SEMICOLON": ";",
            "SCLN": ";",
            "LT": "<",
        },
    }


def test_layer_names_in_the_configured_keycodes_become_layer_numbers():
    config = {
        "parse_config": {
            "raw_binding_map": {"LT(_MOD, KC_KP_0)": "Shift", "QK_LLCK": "Lock"}
        }
    }

    numbered = keymap_image.number_layers(config, ["BASE", "SYM", "MOD"])

    assert numbered["parse_config"]["raw_binding_map"] == {
        "LT(2,KC_KP_0)": "Shift",
        "QK_LLCK": "Lock",
    }


def test_keycodes_of_keymap_drawer_and_of_the_configuration_have_a_legend():
    layers = {
        "BASE": keys(
            {0: "KC_SCLN", 1: "KC_TRNS", 2: "KC_NO", 3: "QK_LLCK", 30: "LT(1,KC_TAB)"},
            fill="KC_A",
        ),
        "SYM": keys({0: "MO(0)", 1: "LT(1,KC_KP_0)"}),
    }
    config = parse_config({"QK_LLCK": "Lock", "LT(1,KC_KP_0)": "Shift"})

    keymap_image.check_legends(layers, config)


def test_a_basic_keycode_that_keymap_drawer_does_not_know_stops_the_script_and_is_named():
    layers = {"BASE": keys({5: "KC_FOO"}, fill="KC_A")}

    with pytest.raises(KeymapImageError, match="KC_FOO"):
        keymap_image.check_legends(layers, parse_config({}))


def test_a_basic_keycode_whose_name_is_its_legend_has_a_legend():
    layers = {
        "BASE": keys(
            {0: "KC_Z", 1: "KC_0", 2: "KC_F12", 3: "KC_BSPC", 30: "LT(1,KC_ENT)"},
            fill="KC_A",
        )
    }

    keymap_image.check_legends(layers, parse_config({}))


def test_a_keycode_without_a_legend_stops_the_script_and_is_named():
    layers = {"BASE": keys({5: "COS_FOO"}, fill="KC_A")}

    with pytest.raises(KeymapImageError, match="COS_FOO"):
        keymap_image.check_legends(layers, parse_config({"COS_LCTL": "Ctrl"}))


def test_a_tap_keycode_without_a_legend_stops_the_script_and_is_named():
    layers = {"BASE": keys({30: "LT(1,MS_BTN1)"}, fill="KC_A")}

    with pytest.raises(KeymapImageError, match="MS_BTN1"):
        keymap_image.check_legends(layers, parse_config({}))


# --- Setting the marks on the keymap-drawer YAML ------------------------------

TRANS_LEGEND = {"t": "▽", "type": "trans"}


def marked(parsed, keycode, mark):
    """One key of the keymap-drawer YAML after the marks are set."""
    keymap = keymap_image.apply_marks(
        {"layers": {"NAV": [parsed]}},
        {"NAV": [keycode]},
        {"NAV": {0: mark} if mark else {}},
        TRANS_LEGEND,
    )
    return keymap["layers"]["NAV"][0]


def test_a_mark_becomes_the_type_of_the_key_and_keeps_its_legend():
    assert marked("Ctrl", "COS_LCTL", ONE_SHOT_MOD) == {
        "t": "Ctrl",
        "type": ONE_SHOT_MOD,
    }
    assert marked({"t": "TAB", "h": "SYM"}, "LT(1,KC_TAB)", HOLDING) == {
        "t": "TAB",
        "h": "SYM",
        "type": HOLDING,
    }


def test_a_trigger_key_that_keymap_drawer_marked_as_held_is_a_trigger_key():
    assert marked({"t": "NAV", "type": HELD}, "MO(3)", TRIGGER) == {
        "t": "NAV",
        "type": TRIGGER,
    }


def test_a_held_key_has_no_legend():
    assert marked(dict(TRANS_LEGEND), "KC_TRNS", HELD) == {"type": HELD}


def test_a_transparent_key_that_keymap_drawer_marked_as_held_is_transparent_again():
    assert marked({"type": HELD}, "KC_TRNS", None) == TRANS_LEGEND


def test_a_key_without_a_mark_stays_as_parsed():
    assert marked("ESC", "KC_ESC", None) == "ESC"
    assert marked("", "KC_NO", None) == ""
    assert marked(dict(TRANS_LEGEND), "KC_TRNS", None) == TRANS_LEGEND


# --- The whole route, with keymap-drawer --------------------------------------


def test_the_committed_keymap_image_is_what_the_script_writes(tmp_path):
    image = tmp_path / "keymap.svg"

    assert keymap_image.main(["--output", str(image)]) == 0

    assert image.read_bytes() == IMAGE.read_bytes()


def test_a_keycode_of_layout_h_without_a_legend_stops_the_script(tmp_path):
    text = LAYOUT_H.read_text()
    assert "QK_BOOT,  KC_F9" in text
    layout_h = tmp_path / "layout.h"
    layout_h.write_text(text.replace("QK_BOOT,  KC_F9", "QK_FOO,   KC_F9"))
    image = tmp_path / "keymap.svg"

    with pytest.raises(KeymapImageError, match="QK_FOO"):
        keymap_image.write_image(layout_h, image)

    assert not image.exists()
