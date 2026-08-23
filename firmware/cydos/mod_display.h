#pragma once
#include <Arduino_GFX_Library.h>
#include "mod_config.h"

static Arduino_DataBus *gfxBus = new Arduino_HWSPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_CLK, PIN_TFT_MOSI, PIN_TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(gfxBus, PIN_TFT_RST, 1);

static bool g_colorInvert = false;
static uint8_t g_brightness = 100;

void displayBegin() {
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);
  gfx->begin(80000000);
  gfx->setRotation(1);
}

void applyInvert() {
  gfx->invertDisplay(g_colorInvert);
}

void applyBrightness() {
  static bool pwmReady = false;
  if (!pwmReady) {
    pwmReady = ledcAttach(PIN_TFT_BL, 5000, 8);
    if (!pwmReady) return;
  }
  ledcWrite(PIN_TFT_BL, (uint32_t)g_brightness * 255 / 100);
}

uint16_t textWidth(const char *s, uint8_t size) {
  return strlen(s) * 6 * size;
}

void centerText(int16_t y, const char *s, uint16_t color, uint8_t size) {
  gfx->setTextSize(size);
  gfx->setTextColor(color);
  int16_t x = (SCREEN_W - (int16_t)textWidth(s, size)) / 2;
  if (x < 0) x = 0;
  gfx->setCursor(x, y);
  gfx->print(s);
}

static void splashBar(uint16_t pct, const char *label) {
  gfx->fillRect(70, 168, 180, 18, C_BLACK);
  gfx->drawRect(69, 167, 182, 20, C_LGRAY);
  uint16_t w = (uint16_t)(178 * pct / 100);
  if (w > 0) gfx->fillRect(71, 169, w, 14, C_GREEN);
  gfx->fillRect(60, 196, 200, 12, C_BLACK);
  centerText(196, label, C_LGRAY, 1);
}

static void splashFrame(uint16_t pct, const char *label) {
  static bool drawn = false;
  if (!drawn) {
    drawn = true;
    gfx->fillScreen(C_BLACK);
    gfx->fillRect(30, 40, 260, 160, C_FACE);
    gfx->drawRect(28, 38, 264, 164, C_WHITE);
    gfx->drawRect(29, 39, 262, 162, C_SHADOW);
    gfx->fillRect(31, 41, 258, 24, C_TITLE);
    gfx->setTextSize(2);
    gfx->setTextColor(C_TITLETEXT);
    gfx->setCursor(40, 46);
    gfx->print("CYDOS");
    gfx->fillRect(258, 44, 18, 16, C_FACE);
    gfx->drawRect(257, 43, 20, 18, C_BLACK);
    gfx->setTextSize(1);
    gfx->setTextColor(C_BLACK);
    gfx->setCursor(263, 48);
    gfx->print("X");
    gfx->setTextSize(4);
    gfx->setTextColor(C_TITLE);
    int16_t x = 160 - (int16_t)(5 * 6 * 4 + 4 * 4) / 2;
    gfx->setCursor(x, 90);
    gfx->print("CYDOS");
    centerText(130, "CYD Operating System", C_DGRAY, 1);
    char vbuf[24];
    snprintf(vbuf, sizeof(vbuf), "v%s", CYDOS_VERSION);
    centerText(144, vbuf, C_DGRAY, 1);
  }
  splashBar(pct, label);
}

void splashDone() { splashBar(100, ""); }
