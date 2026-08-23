#pragma once
#include "mod_config.h"
#include "mod_kv.h"

static uint8_t ledCurR = 0, ledCurG = 255, ledCurB = 0;
static bool ledPwmReady = false;

void sysLed(uint8_t r, uint8_t g, uint8_t b) {
  if (!ledPwmReady) {
    ledPwmReady = true;
    ledcAttach(PIN_LED_R, 5000, 8);
    ledcAttach(PIN_LED_G, 5000, 8);
    ledcAttach(PIN_LED_B, 5000, 8);
  }
  ledCurR = r;
  ledCurG = g;
  ledCurB = b;
  ledcWrite(PIN_LED_R, 255 - r);
  ledcWrite(PIN_LED_G, 255 - g);
  ledcWrite(PIN_LED_B, 255 - b);
}

void sysLedOff() {
  sysLed(0, 0, 0);
}

void sysBeep(int freq, int ms) {
  tone(PIN_BUZZER, freq, ms);
}

void sysSetBrightness(uint8_t pct) {
  if (pct < 5) pct = 5;
  if (pct > 100) pct = 100;
  g_brightness = pct;
  applyBrightness();
  kvSetInt("bl", pct);
}

uint8_t sysGetBrightness() { return g_brightness; }

void sysSetInvert(bool inv) {
  g_colorInvert = inv;
  applyInvert();
  kvSetBool("inv", inv);
}

bool sysGetInvert() { return g_colorInvert; }

void sysApplySaved() {
  g_brightness = (uint8_t)kvGetInt("bl", 100);
  if (g_brightness < 10) g_brightness = 100;
  applyBrightness();
  g_colorInvert = kvGetBool("inv", false);
  applyInvert();
  bool ledOn = kvGetBool("ledon", true);
  if (ledOn) {
    sysLed(kvGetInt("ledr", 0), kvGetInt("ledg", 255), kvGetInt("ledb", 0));
  } else {
    sysLedOff();
  }
}

void sysSaveLed(bool on, uint8_t r, uint8_t g, uint8_t b) {
  kvSetBool("ledon", on);
  kvSetInt("ledr", r);
  kvSetInt("ledg", g);
  kvSetInt("ledb", b);
}

void sysSleepNow() {
  sysBeep(600, 80);
  delay(120);
  gfx->fillScreen(C_BLACK);
  gfx->displayOff();
  ledcWrite(PIN_TFT_BL, 0);
  ledcDetach(PIN_TFT_BL);
  digitalWrite(PIN_TFT_BL, LOW);
  ledcDetach(PIN_LED_R);
  ledcDetach(PIN_LED_G);
  ledcDetach(PIN_LED_B);
  ledPwmReady = false;
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, HIGH);
  digitalWrite(PIN_LED_B, HIGH);
  gpio_hold_en((gpio_num_t)PIN_LED_R);
  gpio_hold_en((gpio_num_t)PIN_LED_G);
  gpio_hold_en((gpio_num_t)PIN_LED_B);
  gpio_deep_sleep_hold_en();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_BOOT, 0);
  esp_deep_sleep_start();
}

void sysReboot() {
  sysBeep(1200, 60);
  delay(150);
  ESP.restart();
}
