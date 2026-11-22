#!/bin/sh
# Step 2a of the experiment: family_keymap.json -> SVG with no configuration
# at all, to record what keymap-drawer does out of the box.
set -eu
cd "$(dirname "$0")"
KEYMAP="uvx --from keymap-drawer==0.23.0 keymap"

$KEYMAP parse -c 10 -q family_keymap.json \
    --layer-names BASE SYM NUM NAV MOU FUN MOD \
    -o out-of-the-box.yaml

# The two parametrized physical layouts that need no keyboard name.
$KEYMAP draw -n "33333+2 2+33333" out-of-the-box.yaml \
    -o out-of-the-box.cols-thumbs.svg
$KEYMAP draw --ortho-layout '{split: true, rows: 3, columns: 5, thumbs: 2}' out-of-the-box.yaml \
    -o out-of-the-box.ortho.svg
