# Source this file:  . ./env.sh
# Sets up the project-local ESP-IDF (fw20/esp-idf) and tools (fw20/.espressif).
_FW20_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
export IDF_TOOLS_PATH="$_FW20_DIR/.espressif"
export IDF_PATH="$_FW20_DIR/esp-idf"
. "$IDF_PATH/export.sh"
unset _FW20_DIR
