# CYDOS

A tiny Windows-9x-inspired operating system for the
[Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
(ESP32-2432S028 "2USB" variant).

One firmware lives on the ESP32. Everything else - every app - is a folder on
the SD card. No apps are baked into the firmware: plug the card into a
computer, drop in a new app folder, reboot, done.

![theme](https://img.shields.io/badge/theme-Win95-teal)

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

- Boot splash with progress bar
- Desktop with pages of icons (tap an icon to launch), taskbar with clock,
  Start menu (Files, Editor, About, Power), window chrome around every app
- Apps are discovered by scanning `/apps/*` on the SD card at boot.
  A folder needs `app.lua` to count; `manifest.txt` (first line = display
  name) and `icon.jpg` (48x48) are optional.
- On-screen keyboard for dialogs
- WiFi + NTP time sync (configured on-device via Settings)
- Settings persisted in flash (brightness, LED color, invert)

## Flashing

```sh
scripts/build.sh    # compile
scripts/flash.sh    # upload over USB
```

Requirements: `arduino-cli` (a flatpak install is auto-detected), the
`esp32:esp32` platform, and these libraries in the sketchbook
(`~/other/Arduino/libraries`): `GFX_Library_for_Arduino`, `TJpg_Decoder`,
`XPT2046_Touchscreen`. Lua 5.4 sources are vendored under
`firmware/cydos/src/lua`.

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

## Making apps

See [HOWTOMAKEAPP.md](HOWTOMAKEAPP.md) for the full Lua API and a template.

## Repo layout

```
firmware/cydos/      the OS firmware
sdcard/apps/         starter app pack (source of truth for the installer)
scripts/             build + flash helpers
```
