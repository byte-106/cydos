#pragma once
#include "mod_config.h"
#include "mod_ui.h"

static int kbY0 = 132;
static const int kbX0 = 4;
static const int kbStep = 39;
static const int kbW = 38;
static const int kbH = 24;

#define KB_PAGE 128
#define KB_SHIFT 129
#define KB_BS 130
#define KB_ENTER 131

static void kbSetY(int y) { kbY0 = y; }

static bool kbShift = false;
static bool kbNumPage = false;

static const char *kbLower[3] = { "qwertyui", "asdfghjk", "zxcvbnm" };
static const char *kbUpper[3] = { "QWERTYUI", "ASDFGHJK", "ZXCVBNM" };
static const char *kbNum[3] = { "12345678", "90@#$%&*", "()-_+=:;" };

static void kbKey(int16_t x, int16_t y, int16_t w, const char *label) {
  drawBtnBevel(x, y, w, kbH, label, false, C_BLACK);
}

void kbDraw() {
  gfx->fillRect(0, kbY0 - 6, SCREEN_W, SCREEN_H - kbY0 + 6, C_FACE);
  for (int r = 0; r < 3; r++) {
    int cols = (r == 2 && !kbNumPage) ? 7 : 8;
    for (int c = 0; c < cols; c++) {
      char ch = kbNumPage ? kbNum[r][c] : (kbShift ? kbUpper[r][c] : kbLower[r][c]);
      char s[2] = { ch, 0 };
      kbKey(kbX0 + c * kbStep, kbY0 + r * 26, kbW, s);
    }
    if (r == 2 && !kbNumPage) kbKey(kbX0 + 7 * kbStep, kbY0 + 2 * 26, kbW, kbShift ? "LOCK" : "^");
  }
  kbKey(kbX0, kbY0 + 78, 52, kbNumPage ? "abc" : "123");
  kbKey(kbX0 + 56, kbY0 + 78, 112, "_");
  kbKey(kbX0 + 172, kbY0 + 78, 38, "\\n");
  kbKey(kbX0 + 214, kbY0 + 78, kbW, ".");
  kbKey(kbX0 + 256, kbY0 + 78, 60, "<-");
}

int kbPoll(int16_t x, int16_t y) {
  if (!inRect(x, y, 0, kbY0, SCREEN_W, SCREEN_H - kbY0)) return 0;
  int ry = y - kbY0;
  if (ry < 78) {
    int r = ry / 26;
    if (r > 2) return 0;
    int cols = (r == 2 && !kbNumPage) ? 8 : 8;
    int c = (x - kbX0) / kbStep;
    if (c < 0 || c >= cols) return 0;
    if (r == 2 && !kbNumPage && c == 7) return KB_SHIFT;
    return kbNumPage ? kbNum[r][c] : (kbShift ? kbUpper[r][c] : kbLower[r][c]);
  }
  if (inRect(x, y, kbX0, kbY0 + 78, 52, kbH)) return KB_PAGE;
  if (inRect(x, y, kbX0 + 56, kbY0 + 78, 112, kbH)) return ' ';
  if (inRect(x, y, kbX0 + 172, kbY0 + 78, 38, kbH)) return KB_ENTER;
  if (inRect(x, y, kbX0 + 214, kbY0 + 78, kbW, kbH)) return '.';
  if (inRect(x, y, kbX0 + 256, kbY0 + 78, 60, kbH)) return KB_BS;
  return 0;
}

bool kbInput(const char *title, String &out, bool mask, int maxLen = 48) {
  out = "";
  kbShift = false;
  kbNumPage = false;
  kbY0 = 132;
  bool redrawKb = true;
  gfx->fillRect(0, 0, SCREEN_W, kbY0 - 6, C_DESKTOP);
  drawWindowFrame(title, 4, 8, 312, 84);
  drawWell(12, 44, 180, 32, C_WHITE);
  UiButton okBtn(204, 42, 52, 34, "OK");
  UiButton noBtn(262, 42, 46, 34, "X");
  okBtn.draw();
  noBtn.draw();
  String lastDisp = "\x01";
  while (true) {
    if (redrawKb) {
      kbDraw();
      redrawKb = false;
    }
    String disp = mask ? "" : out;
    if (mask) {
      int dots = min((int)out.length(), 14);
      for (int i = 0; i < dots; i++) disp += "*";
    }
    if ((int)disp.length() > 13) disp = disp.substring(disp.length() - 13);
    disp += "_";
    if (disp != lastDisp) {
      lastDisp = disp;
      gfx->fillRect(14, 46, 176, 28, C_WHITE);
      gfx->setTextSize(2);
      gfx->setTextColor(C_BLACK);
      gfx->setCursor(16, 54);
      gfx->print(disp);
    }
    touchPoll();
    TouchEvent e;
    while (tePop(e)) {
      if (e.type != TE_PRESS) continue;
      if (okBtn.hit(e.x, e.y)) { teWaitRelease(); return true; }
      if (noBtn.hit(e.x, e.y)) { teWaitRelease(); return false; }
      int k = kbPoll(e.x, e.y);
      if (k == 0) continue;
      teWaitRelease();
      if (k >= 32 && k < 128) {
        if ((int)out.length() < maxLen) {
          out += (char)k;
          if (kbShift && !kbNumPage) {
            kbShift = false;
            redrawKb = true;
          }
        }
      } else if (k == KB_BS) {
        if (out.length()) out.remove(out.length() - 1);
      } else if (k == KB_SHIFT) {
        kbShift = !kbShift;
        redrawKb = true;
      } else if (k == KB_PAGE) {
        kbNumPage = !kbNumPage;
        redrawKb = true;
      }
      break;
    }
    vTaskDelay(5);
  }
}
