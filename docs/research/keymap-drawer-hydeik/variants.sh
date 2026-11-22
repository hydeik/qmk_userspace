#!/bin/sh
# Side experiments behind single statements of the notes. Writes to variants/.
# Run draw.sh first: most of these start from hydeik.yaml.
set -u
cd "$(dirname "$0")"
mkdir -p variants
KEYMAP="uvx --from keymap-drawer==0.23.0 --constraints constraints.txt keymap"
NAMES="BASE SYM NUM NAV MOU FUN MOD"

echo "== 1. physical layouts that need no keyboard name"
$KEYMAP -c config.yaml draw -n "33333+2 2+33333" hydeik.yaml -o variants/cols-thumbs.svg
cmp hydeik.svg variants/cols-thumbs.svg && echo "cols+thumbs '33333+2 2+33333' == ortho_layout: same bytes"
# Column stagger: v pushes a column down half a key, ^ up; > and < after the
# thumb count shift the thumb row half a key.
# A spread of a whole key (v and ^ together) reorders the keys: the generator
# sorts them by int(y), so the legends land on the wrong positions.
$KEYMAP -c config.yaml draw -n "3v333^3+2> 2<+33^333v" hydeik.yaml -o variants/cols-thumbs-stagger.svg
echo "stagger rc=$?"
# A spread of half a key keeps the order, but the thumb row overlaps the
# columns that are pushed down.
$KEYMAP -c config.yaml draw -n "3v3333v+2 2+3v3333v" hydeik.yaml -o variants/cols-thumbs-half-stagger.svg
echo "half stagger rc=$?"
# A local file in the QMK info.json format. -l is needed here and below: the
# parsed YAML carries `layout_name: LAYOUT_HYDEIK_34` (the macro name c2json
# saw), and draw looks that name up in the given layout source.
$KEYMAP -c config.yaml draw -j layout-34.json -l LAYOUT_hydeik_34 hydeik.yaml -o variants/qmk-info-json.svg
echo "qmk-info-json rc=$?"

echo "== 2. physical layouts fetched over the network (cached afterwards)"
$KEYMAP -c config.yaml draw -k generic/split_3x5_2 -l LAYOUT_split_3x5_2 hydeik.yaml -o variants/generic-split_3x5_2.svg
echo "generic/split_3x5_2 rc=$?"
$KEYMAP -c config.yaml draw -k ferris/sweep -l LAYOUT_split_3x5_2 hydeik.yaml -o variants/ferris-sweep.svg
echo "ferris/sweep rc=$?"

echo "== 3. dark_mode: true (dark colours unconditionally), via the environment"
KEYMAP_dark_mode=true $KEYMAP draw --ortho-layout '{split: true, rows: 3, columns: 5, thumbs: 2}' \
    out-of-the-box.yaml -o variants/dark-env.svg
echo "dark rc=$?"
cmp -s out-of-the-box.ortho.svg variants/dark-env.svg && echo "env var had NO effect" || echo "env var changed the output"

echo "== 4. mark_alternate_layer_activators: true"
KEYMAP_mark_alternate_layer_activators=true $KEYMAP parse -c 10 -q family_keymap.json \
    --layer-names $NAMES -o variants/alternate.yaml
diff out-of-the-box.yaml variants/alternate.yaml

echo "== 5. qmk_keycode_map in a config file: merged with the defaults, or replacing them?"
printf 'parse_config:\n  qmk_keycode_map:\n    KP_0: Shift\n' > variants/keycode-map.config.yaml
$KEYMAP -c variants/keycode-map.config.yaml parse -c 10 -q family_keymap.json \
    --layer-names $NAMES -o variants/keycode-map.yaml
sed -n '2,13p' variants/keycode-map.yaml

echo "== 6. layer names left as enum names (no #define of the layer numbers)"
sed -e 's/LT(1, /LT(_SYM, /' -e 's/MO(3)/MO(_NAV)/g' family_keymap.json > variants/symbolic-layers.json
$KEYMAP parse -c 10 -q variants/symbolic-layers.json --layer-names $NAMES -o variants/symbolic-layers.yaml
echo "symbolic rc=$?"
grep -n "SYM\|NAV" variants/symbolic-layers.yaml | head -8

echo "== 7. wrong number of layer names"
$KEYMAP parse -q family_keymap.json --layer-names A B -o /dev/null 2>&1 | tail -1

echo "== 8. same bytes under another Python? (the pinned dependencies stay the same)"
for python in 3.12 3.14; do
    PY="uvx --python $python --from keymap-drawer==0.23.0 --constraints constraints.txt keymap"
    $PY -c config.yaml parse -c 10 -q family_keymap.json --layer-names $NAMES -b base.yaml \
        -o "variants/py$python.yaml"
    $PY -c config.yaml draw --ortho-layout '{split: true, rows: 3, columns: 5, thumbs: 2}' \
        "variants/py$python.yaml" -o "variants/py$python.svg"
    cmp hydeik.yaml "variants/py$python.yaml" && cmp hydeik.svg "variants/py$python.svg" &&
        echo "Python $python: same bytes"
    rm -f "variants/py$python.yaml" "variants/py$python.svg"
done
