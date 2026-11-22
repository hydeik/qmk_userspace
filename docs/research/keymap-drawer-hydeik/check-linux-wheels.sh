#!/bin/sh
# Stand-in for "does it install in the CI container?", run on a machine without
# Docker: resolve keymap-drawer for Linux x86_64 and the Python versions of the
# container, allowing binary wheels only. A resolution that succeeds means
# `pip install keymap-drawer==0.23.0` needs no compiler there. It does not
# prove that the tool runs in the container.
set -eu
cd "$(dirname "$0")"
for python in 3.13 3.14; do
    echo "== Python $python, x86_64-manylinux_2_28, wheels only"
    echo "keymap-drawer==0.23.0" |
        uv pip compile - --python-version "$python" \
            --python-platform x86_64-manylinux_2_28 \
            --only-binary :all: --no-header --no-annotate
done
