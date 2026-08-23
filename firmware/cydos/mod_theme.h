#pragma once
#include <Arduino.h>
#include "mod_config.h"

static uint16_t C_BLACK = 0x0000;
static uint16_t C_WHITE = 0xFFFF;
static uint16_t C_RED = 0xF800;
static uint16_t C_GREEN = 0x07E0;
static uint16_t C_BLUE = 0x001F;
static uint16_t C_YELLOW = 0xFFE0;
static uint16_t C_CYAN = 0x07FF;
static uint16_t C_MAGENTA = 0xF81F;
static uint16_t C_ORANGE = 0xFD20;
static uint16_t C_GRAY = 0x8410;
static uint16_t C_DGRAY = 0x4208;
static uint16_t C_LGRAY = 0xC618;
static uint16_t C_DESKTOP = 0x0410;
static uint16_t C_TITLE = 0x0010;
static uint16_t C_FACE = 0xC618;
static uint16_t C_SHADOW = 0x8410;
static uint16_t C_HILIGHT = 0xFFFF;
static uint16_t C_WELL = 0x86B0;
static uint16_t C_SELLINE = 0x399A;
static uint16_t C_TITLETEXT = 0xFFFF;
static uint16_t C_DESKTXT = 0xFFFF;

static int THEME_VER = CYDOS_THEME;
static bool THEME_FLAT = false;

static void applyTheme(int ver) {
  THEME_VER = ver;
  THEME_FLAT = (ver < 3);
  if (ver == 1) {
    C_DESKTOP = 0x57EA;
    C_TITLE = 0xFFEA;
    C_TITLETEXT = 0x0000;
    C_DESKTXT = 0x0000;
    C_FACE = 0xFFFF;
    C_SHADOW = 0x0000;
    C_HILIGHT = 0x0000;
    C_WELL = 0xFFFF;
    C_SELLINE = 0xFAAA;
  } else if (ver == 2) {
    C_DESKTOP = 0x57EA;
    C_TITLE = 0xFFEA;
    C_TITLETEXT = 0x0000;
    C_DESKTXT = 0x0000;
    C_FACE = 0xFFFF;
    C_SHADOW = 0x0000;
    C_HILIGHT = 0x0000;
    C_WELL = 0xFFFF;
    C_SELLINE = 0xFAAA;
  } else if (ver == 3) {
    C_DESKTOP = 0x0410;
    C_TITLE = 0x0010;
    C_FACE = 0xC618;
    C_SHADOW = 0x8410;
    C_HILIGHT = 0xFFFF;
    C_WELL = 0x86B0;
    C_SELLINE = 0x399A;
    C_TITLETEXT = 0xFFFF;
    C_DESKTXT = 0xFFFF;
    THEME_FLAT = true;
  } else {
    C_DESKTOP = 0x0410;
    C_TITLE = 0x0010;
    C_FACE = 0xC618;
    C_SHADOW = 0x8410;
    C_HILIGHT = 0xFFFF;
    C_WELL = 0x86B0;
    C_SELLINE = 0x399A;
    C_TITLETEXT = 0xFFFF;
    C_DESKTXT = 0xFFFF;
    THEME_FLAT = false;
  }
}
