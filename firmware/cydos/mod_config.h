#pragma once
#include <Arduino.h>

#define CYDOS_NAME "CYDOS"
#define CYDOS_VERSION "1.0.0"

#define PIN_TFT_BL   21
#define PIN_TFT_DC   2
#define PIN_TFT_RST  12
#define PIN_TFT_CS   15
#define PIN_TFT_MOSI 13
#define PIN_TFT_CLK  14
#define PIN_TFT_MISO 12

#define PIN_SD_CS   5
#define PIN_SD_MOSI 23
#define PIN_SD_MISO 19
#define PIN_SD_SCK  18

#define PIN_TOUCH_CS  33
#define PIN_TOUCH_IRQ 36
#define PIN_TOUCH_MOSI 32
#define PIN_TOUCH_MISO 39
#define PIN_TOUCH_CLK 25

#define PIN_BUZZER 26

#define PIN_LED_R 4
#define PIN_LED_G 16
#define PIN_LED_B 17

#define PIN_BTN_BOOT 0

#define SCREEN_W 320
#define SCREEN_H 240
#define TASKBAR_H 44
#define TITLEBAR_H 28

#define C_BLACK      0x0000
#define C_WHITE      0xFFFF
#define C_RED        0xF800
#define C_GREEN      0x07E0
#define C_BLUE       0x001F
#define C_YELLOW     0xFFE0
#define C_CYAN       0x07FF
#define C_MAGENTA    0xF81F
#define C_ORANGE     0xFD20
#define C_GRAY       0x8410
#define C_DGRAY      0x4208
#define C_LGRAY      0xC618
#define C_DESKTOP    0x0410
#define C_TITLE      0x0010
#define C_FACE       0xC618
#define C_SHADOW     0x8410
#define C_HILIGHT    0xFFFF
#define C_WELL       0x86B0
#define C_SELLINE    0x399A

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}
