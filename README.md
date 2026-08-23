# CYDOS

A tiny retro-Windows operating system for the
[Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
(ESP32-2432S028 "2USB" variant).

One firmware lives on the ESP32. Everything else - every app - is a folder on
the SD card. No apps are baked into the firmware: plug the card into a
computer, drop in a new app folder, reboot, done.

CYDOS wears its version on its sleeve: **CYDOS 1.0** is skinned after
**Windows 1.0** (1985) - flat 2D chrome, double-line window borders, green
desktop, yellow title bars. Future versions will dress up as Windows 2.0,
3.x and 95 - bump one define and the whole OS re-skins itself.

![theme](https://img.shields.io/badge/theme-Windows%201.0%20%281985%29-yellow)
![lua](https://img.shields.io/badge/apps-Lua%205.5-blue)
![board](https://img.shields.io/badge/board-CYD--2USB-green)

## Hardware

| Part | Pin |
|---|---|
| TFT ILI9341 | BL=21 DC=2 RST=12 CS=15 MOSI=13 CLK=14 MISO=12 |
| SD card | CS=5 MOSI=23 MISO=19 SCK=18 |
| Touch XPT2046 | CS=33 IRQ=36 MOSI=32 MISO=39 CLK=25 |
| Buzzer | 26 |
| RGB LED (active-low) | R=4 G=16 B=17 |

Display must run with `invertDisplay(false)` - the 2USB panel variant shows
inverted colors otherwise.

## Firmware features

- First-boot **setup wizard** (welcome -> install progress -> done), Windows
  setup edition. Re-runs automatically whenever the embedded app pack changes;
  hold BOOT at power-on to force a silent reinstall
- Desktop with pages of icons (tap an icon to launch), taskbar with clock and
  wifi/LED tray, Start menu (Files, Editor, Images, About, Power)
- Apps are discovered by scanning `/apps/*` on the SD card at boot.
  A folder needs `app.lua` to count; `manifest.txt` (first line = display
  name) and `icon.jpg` (48x48) are optional
- Lua 5.5 app runtime with a sandboxed API (graphics, touch, files, JSON,
  keyboard, WiFi, sound) - see [HOWTOMAKEAPP.md](HOWTOMAKEAPP.md)
- On-screen keyboard for dialogs, built-in text editor and image viewer
- Theme engine: all colors + flat/beveled styling come from one
  `applyTheme(version)` call (`mod_theme.h`)
- WiFi + NTP time sync (configured on-device via Settings or by editing the
  settings file)
- Settings persisted in flash and mirrored to a human-editable
  `/data/Settings/settings.json` on the card; the file is applied on boot,
  so hand edits survive reboots. It always contains `wifi_ssid` /
  `wifi_pass` slots you can fill in directly
- Apps save their own JSON config in `/data/<AppName>/` via the `json`
  global (high scores, brush colors, ...)

## Flashing

```sh
scripts/build.sh    # compile
scripts/flash.sh    # upload over USB
```

Requirements: `arduino-cli` (a flatpak install is auto-detected), the
`esp32:esp32` platform with the `huge_app` partition scheme, and these
libraries in the sketchbook (`~/other/Arduino/libraries`):
`GFX_Library_for_Arduino`, `TJpg_Decoder`, `XPT2046_Touchscreen`.
Lua 5.5 sources are vendored under `firmware/cydos/src/lua` (trimmed to
base/table/string/math).

The build path is fixed to `/tmp/opencode/cydos_build` because arduino-cli
chokes on paths containing spaces (the project lives on a mount with spaces).

## SD card layout

```
/apps
  /Settings        {app.lua, manifest.txt, icon.jpg}
  /Snake           ...
  /AnythingYouLike ...
/data/<AppName>/   per-app writable storage (sandboxed)
```

If your PC's card reader is broken, no problem: the firmware carries the
starter pack inside. On first boot (or whenever `/.apps_installed` is missing
on the card - or when you hold the BOOT button while powering on) CYDOS
writes all starter apps onto the card through the CYD's own slot and then
continues to the desktop. One flash sets up everything.

To rebuild the embedded pack after editing `sdcard/apps/`:

```sh
python3 tools/gen_sdpack.py
```

## Repo layout

```
firmware/cydos/      the OS firmware (mod_*.h = subsystems, src/lua = Lua 5.5)
firmware/cydos/sdpack.h  generated: starter pack embedded in the firmware
sdcard/apps/         starter app pack (source of truth for the installer)
tools/gen_sdpack.py  regenerates sdpack.h + its content-hash version
scripts/             build + flash helpers
HOWTOMAKEAPP.md      full Lua API reference for app developers
```

## Themes

`mod_config.h` has `#define CYDOS_THEME n`. Each major CYDOS version mimics
the Windows of the same era:

| CYDOS | Skin | Status |
|---|---|---|
| 1.0 | Windows 1.0 (1985) | done - flat, green desktop, yellow title bars |
| 2.0 | Windows 2.0 | reserved |
| 3.0 | Windows 3.x | reserved |
| ... | ... | ... |

## Making apps

See [HOWTOMAKEAPP.md](HOWTOMAKEAPP.md) for the full Lua API and a template.
