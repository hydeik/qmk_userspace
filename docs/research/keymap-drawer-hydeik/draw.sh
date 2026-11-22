#!/bin/sh
# Step 2b of the experiment: family_keymap.json -> hydeik.yaml -> hydeik.svg,
# with config.yaml and base.yaml. No network access once the tool is installed:
# the physical layout is generated from parameters and no glyph is fetched.
#
# Usage: ./draw.sh [<output stem>]   (default: hydeik)
set -eu
cd "$(dirname "$0")"
out=${1:-hydeik}
KEYMAP="uvx --from keymap-drawer==0.23.0 --constraints constraints.txt keymap"

$KEYMAP -c config.yaml parse -c 10 -q family_keymap.json \
    --layer-names BASE SYM NUM NAV MOU FUN MOD \
    -b base.yaml \
    -o "$out.yaml"

$KEYMAP -c config.yaml draw \
    --ortho-layout '{split: true, rows: 3, columns: 5, thumbs: 2}' \
    "$out.yaml" -o "$out.svg"
