#pragma once
#include <XPT2046_Touchscreen.h>
#include "mod_config.h"

enum TouchEventType : uint8_t { TE_PRESS = 1, TE_MOVE = 2, TE_RELEASE = 3 };

struct TouchEvent {
  uint8_t type;
  int16_t x, y;
};

static SPIClass touchSPI(VSPI);
static XPT2046_Touchscreen touch(PIN_TOUCH_CS, PIN_TOUCH_IRQ);

#define TOUCH_QUEUE_SIZE 24
static TouchEvent teQueue[TOUCH_QUEUE_SIZE];
static volatile uint8_t teHead = 0, teTail = 0;

static bool teDown = false;
static int16_t teX = 0, teY = 0, teStartX = 0, teStartY = 0;
static uint32_t teDownMs = 0;
static bool teTapPending = false;

static int16_t calXmin = 200, calXmax = 3700, calYmin = 240, calYmax = 3800;

void inputBegin() {
  touchSPI.begin(PIN_TOUCH_CLK, PIN_TOUCH_MISO, PIN_TOUCH_MOSI, PIN_TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);
}

static void mapTouchRaw(uint16_t rx, uint16_t ry, int16_t &ox, int16_t &oy) {
  ox = map(rx, calXmin, calXmax, 0, SCREEN_W);
  oy = map(ry, calYmin, calYmax, 0, SCREEN_H);
  if (ox < 0) ox = 0;
  if (ox > SCREEN_W - 1) ox = SCREEN_W - 1;
  if (oy < 0) oy = 0;
  if (oy > SCREEN_H - 1) oy = SCREEN_H - 1;
}

static void tePush(uint8_t type, int16_t x, int16_t y) {
  uint8_t next = (teHead + 1) % TOUCH_QUEUE_SIZE;
  if (next == teTail) return;
  teQueue[teHead] = { type, x, y };
  teHead = next;
}

bool touchPoll() {
  bool raw = touch.touched();
  bool activity = false;
  if (raw && !teDown) {
    TS_Point p = touch.getPoint();
    mapTouchRaw(p.x, p.y, teX, teY);
    teDown = true;
    teStartX = teX;
    teStartY = teY;
    teDownMs = millis();
    tePush(TE_PRESS, teX, teY);
    activity = true;
  } else if (raw && teDown) {
    TS_Point p = touch.getPoint();
    int16_t nx, ny;
    mapTouchRaw(p.x, p.y, nx, ny);
    if (abs(nx - teX) > 4 || abs(ny - teY) > 4) {
      teX = nx;
      teY = ny;
      tePush(TE_MOVE, nx, ny);
      activity = true;
    }
    delay(8);
  } else if (!raw && teDown) {
    teDown = false;
    teTapPending = (millis() - teDownMs < 400) && (abs(teX - teStartX) < 12 && abs(teY - teStartY) < 12);
    tePush(TE_RELEASE, teX, teY);
    activity = true;
  }
  return activity;
}

bool tePop(TouchEvent &e) {
  if (teTail == teHead) return false;
  e = teQueue[teTail];
  teTail = (teTail + 1) % TOUCH_QUEUE_SIZE;
  return true;
}

bool teIsDown() { return teDown; }
int16_t teCurX() { return teX; }
int16_t teCurY() { return teY; }

void teWaitRelease() {
  while (touch.touched()) {
    touchPoll();
    vTaskDelay(5);
  }
}

bool inRect(int16_t x, int16_t y, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
  return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}
