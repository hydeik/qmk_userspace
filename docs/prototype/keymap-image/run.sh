#!/bin/sh
# PROTOTYPE, throwaway: draws the variants and opens the page that shows them.
set -eu
cd "$(dirname "$0")"
uv run --no-project --with pyyaml python3 make_variants.py
[ "${1:-}" = "--no-open" ] || open index.html
