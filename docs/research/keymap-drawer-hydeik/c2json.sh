#!/bin/sh
# Step 1 of the experiment: family_keymap.c -> family_keymap.json.
#
# `qmk c2json` is a subcommand of the QMK CLI that lives in the firmware
# checkout, so QMK_HOME must point at one (here: the `qmk` firmware source
# fetched by `make firmware`). Nothing is written into the checkout.
#
# Usage: [QMK_HOME=<path of a qmk_firmware checkout>] ./c2json.sh
set -eu
cd "$(dirname "$0")"
QMK_HOME=${QMK_HOME:-$(cd ../../.. && pwd)/.firmware/qmk}
export QMK_HOME
qmk c2json -kb mothwing -km hydeik -o family_keymap.json family_keymap.c
