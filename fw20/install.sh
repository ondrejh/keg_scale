#!/usr/bin/env bash
# One-time setup: fetch ESP-IDF submodule and install its tools into fw20/.espressif
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
git -C "$DIR/.." submodule update --init --recursive --depth 1 fw20/esp-idf
export IDF_TOOLS_PATH="$DIR/.espressif"
"$DIR/esp-idf/install.sh" esp32c6
