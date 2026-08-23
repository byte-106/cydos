#include "mod_config.h"
#include "mod_display.h"
#include "mod_kv.h"
#include "mod_input.h"
#include "mod_sys.h"
#include "mod_ui.h"
#include "mod_kbd.h"
#include "mod_net.h"
#include "mod_luart.h"
#include "mod_shell.h"

static SPIClass sdSPI(HSPI);

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN_BOOT, INPUT_PULLUP);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  sysLed(0, 255, 0);

  displayBegin();
  applyInvert();
  splashFrame(10, "storage");
  LittleFS.begin(true);
  kvLoadAll();
  sysApplySaved();

  splashFrame(28, "touch screen");
  inputBegin();

  splashFrame(46, "sd card");
  sdSPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  uint32_t t0 = millis();
  bool sdOk = false;
  while (millis() - t0 < 4000) {
    if (SD.begin(PIN_SD_CS, sdSPI, 30000000)) {
      sdOk = true;
      break;
    }
    vTaskDelay(100);
  }
  if (!sdOk) {
    gfx->fillScreen(C_RED);
    centerText(90, "NO SD CARD", C_WHITE, 3);
    centerText(130, "Insert a card - rebooting...", C_YELLOW, 1);
    delay(2500);
    ESP.restart();
  }

  splashFrame(62, "scanning apps");
  scanApps();
  luaGfxInit();

  splashFrame(80, "network");
  netBegin();

  splashFrame(96, "desktop");
  tone(PIN_BUZZER, 900, 60);
  delay(60);
  tone(PIN_BUZZER, 1400, 80);

  desktopLoop();
}

void loop() {
  vTaskDelay(1000);
}
