#!/usr/bin/env bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
PORT="${PORT:-/dev/ttyUSB0}"
"$DIR/scripts/build.sh" >/dev/null
CLI="${ARDUINO_CLI:-$(find /var/lib/flatpak/app/cc.arduino.IDE2 -name arduino-cli -type f 2>/dev/null | head -1)}"
CFG="$(mktemp)"
trap 'rm -f "$CFG"' EXIT
printf 'directories:\n    user: %s/other/Arduino\nboard_manager:\n    additional_urls: []\n' "$HOME" > "$CFG"
exec "$CLI" --config-file "$CFG" upload -p "$PORT" --fqbn esp32:esp32:esp32:PartitionScheme=huge_app --input-dir /tmp/opencode/cydos_build "$DIR/firmware/cydos"
