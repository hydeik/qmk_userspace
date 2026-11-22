"""PROTOTYPE, throwaway: three looks of the keymap image of the hydeik keymap family.

Question (issue "What does the keymap image look like?"): one image of all
seven layers, in which arrangement, on which physical layout, with which
legends and marks, and how it reads in a dark theme.

Starts from the keymap-drawer YAML of the research branch
(docs/research/keymap-drawer-hydeik/hydeik.yaml) and edits it per variant.
How the YAML is produced from layout.h is a later ticket; here it is edited
freely to show what the picture could say.

Run: ./run.sh   (writes out/<variant>.<theme>.svg and opens index.html)
"""

import copy
import pathlib
import subprocess

import yaml

HERE = pathlib.Path(__file__).parent
RESEARCH = HERE.parent.parent / "research" / "keymap-drawer-hydeik"
OUT = HERE / "out"
KEYMAP = [
    "uvx",
    "--from",
    "keymap-drawer==0.23.0",
    "--constraints",
    str(RESEARCH / "constraints.txt"),
    "keymap",
]

ORTHO = ["--ortho-layout", "{split: true, rows: 3, columns: 5, thumbs: 2}"]
STAGGER = ["-j", str(RESEARCH / "layout-34.json"), "-l", "LAYOUT_hydeik_34"]

# (light, dark) fill of a key by class.
COLOURS = {
    "osm": ("#dff3e4", "#2f5a3c"),
    "holding": ("#dde7fb", "#2c3f66"),
    "trigger": ("#fde9c8", "#6b4a12"),
}

TRIGGERS = {"NAV", "MOU", "FUN"}


def style(theme, classes):
    """The extra CSS for the classes used, for a fixed or an automatic theme."""
    light = "".join(f"rect.{c} {{ fill: {COLOURS[c][0]}; }}\n" for c in classes)
    dark = "".join(f"rect.{c} {{ fill: {COLOURS[c][1]}; }}\n" for c in classes)
    if theme == "light":
        return light
    if theme == "dark":
        return dark
    return light + "@media (prefers-color-scheme: dark) {\n" + dark + "}\n"


def text_of(key):
    return key.get("t", "") if isinstance(key, dict) else key


def mark_faithfully(layers):
    """Marks as the firmware behaves, not as keymap-drawer guesses.

    A trigger key is its own kind of key, not a held one; the held key of a
    layer is its holding key; both smart mod layer keys reach MOD.
    """
    for name, rows in layers.items():
        for corner in (0, 9):
            key = rows[2][corner]
            if text_of(key) in TRIGGERS:
                rows[2][corner] = {"t": text_of(key), "type": "trigger"}
    layers["MOD"][3] = [
        {"t": "▽", "type": "trans"},
        {"type": "held"},
        {"type": "held"},
        {"t": "▽", "type": "trans"},
    ]


def relabel(layers, table, trans=None):
    for rows in layers.values():
        for row in rows:
            for i, key in enumerate(row):
                if isinstance(key, dict):
                    if key.get("type") == "trans" and trans is not None:
                        key["t"] = trans
                    if key.get("t") in table:
                        key["t"] = table[key["t"]]
                elif key in table:
                    row[i] = table[key]


def variant_a(keymap):
    """Baseline: what keymap-drawer marks by itself. Ortho, two columns."""
    return keymap, ORTHO, {"n_columns": 2}, ["osm", "holding"], None


def variant_b(keymap):
    """Faithful marks, words as legends, column stagger, two columns."""
    mark_faithfully(keymap["layers"])
    relabel(keymap["layers"], {"Layer Lock": "Lock"})
    footer = "green: one-shot mod key · blue: holding key · amber: trigger key · red: held · ▽: transparent"
    return keymap, STAGGER, {"n_columns": 2}, ["osm", "holding", "trigger"], footer


def variant_c(keymap):
    """Faithful marks, symbols as legends, blank transparent keys, one column."""
    mark_faithfully(keymap["layers"])
    relabel(
        keymap["layers"],
        {
            "Ctrl": "⌃",
            "Alt": "⌥",
            "Gui": "⌘",
            "Shift": "⇧",
            "LEFT": "←",
            "DOWN": "↓",
            "UP": "↑",
            "RGHT": "→",
            "BSPC": "⌫",
            "DEL": "⌦",
            "ENT": "⏎",
            "TAB": "⇥",
            "SPC": "␣",
            "Layer Lock": "Lock",
            "Mouse ←": "Ms ←",
            "Mouse ↓": "Ms ↓",
            "Mouse ↑": "Ms ↑",
            "Mouse →": "Ms →",
            "Wheel ←": "Wh ←",
            "Wheel ↓": "Wh ↓",
            "Wheel ↑": "Wh ↑",
            "Wheel →": "Wh →",
            "Button 1": "Btn 1",
            "Button 2": "Btn 2",
            "Button 3": "Btn 3",
        },
        trans="",
    )
    footer = (
        "green: one-shot mod key · blue: holding key · amber: trigger key · red: held"
    )
    return keymap, ORTHO, {"n_columns": 1}, ["osm", "holding", "trigger"], footer


VARIANTS = {"a": variant_a, "b": variant_b, "c": variant_c}
THEMES = {"light": False, "dark": True, "auto": "auto"}


def main():
    OUT.mkdir(exist_ok=True)
    base = yaml.safe_load((RESEARCH / "hydeik.yaml").read_text())
    base.pop("layout", None)
    for name, build in VARIANTS.items():
        keymap, layout, draw, classes, footer = build(copy.deepcopy(base))
        keymap_file = OUT / f"{name}.yaml"
        keymap_file.write_text(
            yaml.safe_dump(keymap, allow_unicode=True, sort_keys=False)
        )
        for theme, dark_mode in THEMES.items():
            config = {
                "draw_config": {
                    "dark_mode": dark_mode,
                    "append_colon_to_layer_header": False,
                    "svg_extra_style": style(theme, classes),
                    **draw,
                }
            }
            if footer:
                config["draw_config"]["footer_text"] = footer
            config_file = OUT / f"{name}.{theme}.config.yaml"
            config_file.write_text(
                yaml.safe_dump(config, allow_unicode=True, sort_keys=False)
            )
            svg = OUT / f"{name}.{theme}.svg"
            subprocess.run(
                [
                    *KEYMAP,
                    "-c",
                    str(config_file),
                    "draw",
                    *layout,
                    str(keymap_file),
                    "-o",
                    str(svg),
                ],
                check=True,
            )
            print("wrote", svg.relative_to(HERE))


if __name__ == "__main__":
    main()
