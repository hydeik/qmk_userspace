#!/bin/sh
# PROTOTYPE - throwaway.
#
# Expands SYM / NUM / FUN for both keymap families under each variant, prints
# the resulting 34-key grids, and reports whether the three variants agree.
# Needs only a C preprocessor; QMK is not involved, so keycode names stay
# readable instead of turning into numbers.
#
# Usage: sh users/prototype_shared_layers/expand.sh
set -eu
cd "$(dirname "$0")"

expand() { # $1 = variant letter, $2 = family macro
    printf 'SYM: SYM_LAYER\nNUM: NUM_LAYER\nFUN: FUN_LAYER\n' |
        cc -E -P -D"$2" -include "variant_$1.h" -x c - |
        awk -F': ' 'NF == 2 {
            n = split($2, k, /, */)
            printf "  %s\n", $1
            for (i = 1; i <= n; i++) {
                if (i == 31) printf "%33s", ""
                printf "%-10s", k[i]
                if (i % 10 == 0 || i == n) printf "\n"
            }
            if (n != 34) printf "  !! %d keys, expected 34\n", n
        }'
}

for family in FAMILY_HYDEIK FAMILY_VIAL; do
    echo "=================== $family ==================="
    expand a "$family"
    for v in b c; do
        if [ "$(expand a "$family")" = "$(expand "$v" "$family")" ]; then
            echo "  variant $v: identical to variant a"
        else
            echo "  variant $v: DIFFERS from variant a"
            expand "$v" "$family"
        fi
    done
    echo
done
