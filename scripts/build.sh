#!/usr/bin/env bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${ARDUINO_CLI:-$(find /var/lib/flatpak/app/cc.arduino.IDE2 -name arduino-cli -type f 2>/dev/null | head -1)}"
if [ -z "$CLI" ]; then echo "arduino-cli not found"; exit 1; fi
CFG="$(mktemp)"
trap 'rm -f "$CFG"' EXIT
printf 'directories:\n    user: %s/other/Arduino\nboard_manager:\n    additional_urls: []\n' "$HOME" > "$CFG"
exec "$CLI" --config-file "$CFG" compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app --build-path /tmp/opencode/cydos_build "$DIR/firmware/cydos" "$@"
