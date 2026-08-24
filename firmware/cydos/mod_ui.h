#pragma once
#include "mod_config.h"
#include "mod_theme.h"
#include "mod_display.h"
#include "mod_input.h"

static void bevel(int16_t x, int16_t y, int16_t w, int16_t h, bool raised, bool face = true) {
  if (THEME_FLAT) {
    if (face) gfx->fillRect(x, y, w, h, C_FACE);
    gfx->drawRect(x, y, w, h, C_TITLE);
    return;
  }
  if (face) gfx->fillRect(x, y, w, h, C_FACE);
  gfx->drawLine(x, y, x + w - 1, y, raised ? C_HILIGHT : C_SHADOW);
  gfx->drawLine(x, y, x, y + h - 1, raised ? C_HILIGHT : C_SHADOW);
  gfx->drawLine(x + w - 1, y, x + w - 1, y + h - 1, raised ? C_SHADOW : C_HILIGHT);
  gfx->drawLine(x, y + h - 1, x + w - 1, y + h - 1, raised ? C_SHADOW : C_HILIGHT);
}

static void drawBtnBevel(int16_t x, int16_t y, int16_t w, int16_t h, const char *label, bool pressed, uint16_t textColor) {
  bevel(x, y, w, h, !pressed);
  if (pressed && w > 2 && h > 2) gfx->fillRect(x + 1, y + 1, w - 2, h - 2, C_WELL);
  gfx->setTextSize(1);
  gfx->setTextColor(textColor);
  int16_t tx = x + ((int16_t)w - (int16_t)textWidth(label, 1)) / 2;
  int16_t ty = y + (h - 8) / 2 + (pressed ? 1 : 0);
  gfx->setCursor(tx > x ? tx : x + 1, ty);
  gfx->print(label);
}

struct UiButton {
  int16_t x, y, w, h;
  const char *label;
  uint16_t textColor;
  UiButton(int16_t _x, int16_t _y, int16_t _w, int16_t _h, const char *_l, uint16_t tc = C_BLACK)
    : x(_x), y(_y), w(_w), h(_h), label(_l), textColor(tc) {}
  bool hit(int16_t px, int16_t py) const { return inRect(px, py, x, y, w, h); }
  void draw(bool pressed = false) const { drawBtnBevel(x, y, w, h, label, pressed, textColor); }
};

static void drawTitleBar(const char *t, bool showClose = true) {
  gfx->fillRect(0, 0, SCREEN_W, TITLEBAR_H, C_TITLE);
  gfx->setTextSize(2);
  gfx->setTextColor(C_TITLETEXT);
  String s(t);
  if (s.length() > 21) s = s.substring(0, 20) + "~";
  int16_t x1, y1;
  uint16_t tw, th;
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &tw, &th);
  if (THEME_CENTER_TITLE) {
    gfx->setCursor((SCREEN_W - tw) / 2, 7);
    gfx->print(s);
  } else {
    gfx->setCursor(8, 7);
    gfx->print(s);
  }
  if (showClose) {
    gfx->fillRect(SCREEN_W - TITLEBAR_H, 2, TITLEBAR_H - 4, TITLEBAR_H - 4, C_FACE);
    gfx->drawRect(SCREEN_W - TITLEBAR_H, 2, TITLEBAR_H - 4, TITLEBAR_H - 4, C_BLACK);
    gfx->setTextSize(2);
    gfx->setTextColor(C_RED);
    gfx->setCursor(SCREEN_W - TITLEBAR_H + 7, 6);
    gfx->print("X");
  }
}

static void drawWindowFrame(const char *title, int16_t x, int16_t y, int16_t w, int16_t h, bool showClose = false) {
  gfx->fillRect(x, y, w, h, C_FACE);
  if (THEME_FLAT) {
    gfx->drawRect(x, y, w, h, C_TITLE);
    gfx->drawRect(x + 2, y + 2, w - 4, h - 4, C_TITLE);
    gfx->fillRect(x + 4, y + 4, w - 8, TITLEBAR_H - 6, C_TITLE);
    gfx->setTextSize(2);
    gfx->setTextColor(C_TITLETEXT);
    if (THEME_CENTER_TITLE) {
      int16_t x1, y1;
      uint16_t tw, th;
      String s(title);
      if (s.length() > 21) s = s.substring(0, 20) + "~";
      gfx->getTextBounds(s, 0, 0, &x1, &y1, &tw, &th);
      gfx->setCursor(x + (w - tw) / 2, y + 7);
      gfx->print(s);
    } else {
      gfx->setCursor(x + 10, y + 7);
      gfx->print(title);
    }
    if (showClose) {
      int16_t cx = x + w - TITLEBAR_H - 5;
      gfx->fillRect(cx, y + 6, TITLEBAR_H - 10, TITLEBAR_H - 10, C_FACE);
      gfx->drawRect(cx, y + 6, TITLEBAR_H - 10, TITLEBAR_H - 10, C_TITLE);
      gfx->setTextSize(2);
      gfx->setTextColor(C_BLACK);
      gfx->setCursor(cx + (TITLEBAR_H - 10 - 12) / 2, y + 9);
      gfx->print("X");
    }
    return;
  }
  gfx->drawRect(x, y, w, h, C_SHADOW);
  gfx->drawRect(x + 1, y + 1, w - 2, h - 2, C_WHITE);
  gfx->fillRect(x + 3, y + 3, w - 6, TITLEBAR_H - 4, C_TITLE);
  gfx->setTextSize(2);
  gfx->setTextColor(C_TITLETEXT);
  gfx->setCursor(x + 8, y + 7);
  gfx->print(title);
  if (showClose) {
    int16_t cx = x + w - TITLEBAR_H - 4;
    bevel(cx, y + 5, TITLEBAR_H - 8, TITLEBAR_H - 8, true);
    gfx->setTextSize(2);
    gfx->setTextColor(C_BLACK);
    gfx->setCursor(cx + (TITLEBAR_H - 6 - 12) / 2, y + 9);
    gfx->print("X");
  }
}

bool closeButtonHit(int16_t wx, int16_t wy, int16_t ww, int16_t px, int16_t py) {
  return inRect(px, py, wx + ww - TITLEBAR_H - 5, wy + 2, TITLEBAR_H, TITLEBAR_H);
}

static void drawWell(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t inner) {
  gfx->fillRect(x, y, w, h, inner);
  if (THEME_FLAT) {
    gfx->drawRect(x, y, w, h, C_TITLE);
    return;
  }
  gfx->drawLine(x, y, x + w - 1, y, C_SHADOW);
  gfx->drawLine(x, y, x, y + h - 1, C_SHADOW);
  gfx->drawLine(x + w - 1, y, x + w - 1, y + h - 1, C_HILIGHT);
  gfx->drawLine(x, y + h - 1, x + w - 1, y + h - 1, C_HILIGHT);
}

static void drawProgress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t pct, uint16_t color) {
  drawWell(x, y, w, h, C_FACE);
  int16_t iw = (w - 4) * pct / 100;
  if (iw <= 0) return;
  if (THEME_FLAT) {
    gfx->fillRect(x + 2, y + 2, iw, h - 4, color);
    return;
  }
  for (int16_t i = 0; i < iw; i += 4) gfx->fillRect(x + 2 + i, y + 2, min(3, iw - i), h - 4, color);
}
