#include "mod_config.h"
#include "mod_theme.h"
#include "mod_display.h"
#include "mod_kv.h"
#include "mod_input.h"
#include "mod_sys.h"
#include "mod_ui.h"
#include "mod_kbd.h"
#include "mod_net.h"
#include "mod_luart.h"
#include "mod_settings.h"
#include "mod_shell.h"
#include "sdpack.h"

static SPIClass sdSPI(HSPI);

#define SD_MARKER "/.apps_installed"

static void wizWaitPress(int16_t *px, int16_t *py) {
  for (;;) {
    touchPoll();
    TouchEvent e;
    while (tePop(e)) {
      if (e.type == TE_PRESS) {
        *px = e.x;
        *py = e.y;
        return;
      }
    }
    vTaskDelay(2);
  }
}

static void wizText(int16_t x, int16_t y, const char *s, uint16_t c, int size) {
  gfx->setTextSize(size);
  gfx->setTextColor(c);
  gfx->setCursor(x, y);
  gfx->print(s);
}

static void setupWelcome() {
  gfx->fillScreen(C_DESKTOP);
  drawWindowFrame("CYDOS Setup", 14, 22, 292, 196);
  wizText(38, 50, "Welcome to CYDOS!", C_TITLE, 2);
  wizText(38, 84, "A fast little OS for your CYD,", C_BLACK, 1);
  wizText(38, 98, "made for small things.", C_BLACK, 1);
  wizText(38, 128, "Set up CYDOS on this SD card?", C_BLACK, 1);
  UiButton yesB(44, 158, 96, 42, "Yes"), noB(180, 158, 96, 42, "No");
  yesB.draw();
  noB.draw();
  for (;;) {
    int16_t x, y;
    wizWaitPress(&x, &y);
    if (noB.hit(x, y)) sysReboot();
    if (yesB.hit(x, y)) return;
  }
}

static void setupInstall(bool showIntro) {
  gfx->fillScreen(C_DESKTOP);
  drawWindowFrame("CYDOS Setup", 14, 22, 292, 196);
  wizText(32, 48, "Installing CYDOS", C_TITLE, 2);
  if (showIntro) {
    wizText(32, 80, "All apps live on the SD card -", C_BLACK, 1);
    wizText(32, 94, "add your own anytime. See", C_BLACK, 1);
    wizText(32, 108, "HOWTOMAKEAPP.md to get started.", C_BLACK, 1);
  } else {
    wizText(32, 94, "Reinstalling starter apps...", C_BLACK, 1);
  }
  drawProgress(32, 132, 256, 20, 0, C_BLUE);
  int ok = 0;
  for (int i = 0; i < EMBED_COUNT; i++) {
    const EmbedFile &f = EMBED_FILES[i];
    String p(f.path);
    int slash = p.lastIndexOf('/');
    String walk = "";
    int start = 1;
    while (true) {
      int next = p.indexOf('/', start);
      if (next < 0 || next > slash) break;
      walk = p.substring(0, next);
      if (!SD.exists(walk)) SD.mkdir(walk);
      start = next + 1;
    }
    SD.remove(p);
    File out = SD.open(p, FILE_WRITE);
    if (out) {
      out.write(f.data, f.len);
      out.close();
      ok++;
    }
    drawProgress(32, 132, 256, 20, (i + 1) * 100 / EMBED_COUNT, C_BLUE);
    gfx->fillRect(32, 162, 256, 12, C_DESKTOP);
    wizText(32, 162, p.substring(6).c_str(), C_LGRAY, 1);
  }
  File m = SD.open(SD_MARKER, FILE_WRITE);
  if (m) {
    m.print(SDPACK_VERSION);
    m.close();
  }
  wizText(32, 186, ("Installed " + String(ok) + "/" + String(EMBED_COUNT) + " files.").c_str(), C_GREEN, 1);
  delay(900);
}

static void setupDone() {
  gfx->fillScreen(C_DESKTOP);
  drawWindowFrame("CYDOS Setup", 14, 22, 292, 196);
  wizText(60, 70, "Setup complete!", C_TITLE, 2);
  wizText(88, 110, "Enjoy CYDOS!", C_BLACK, 2);
  int16_t tw = textWidth("tap anywhere to start", 1);
  wizText((SCREEN_W - tw) / 2, 170, "tap anywhere to start", C_WHITE, 1);
  int16_t x, y;
  wizWaitPress(&x, &y);
  teWaitRelease();
}

static void uiTask(void *) {
  applyTheme(CYDOS_THEME);
  displayBegin();
  applyInvert();
  splashFrame(10, "storage");
  LittleFS.begin(true);
  kvLoadAll();

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
  settingsLoadApply();
  sysApplySaved();
  bool forcedSetup = digitalRead(PIN_BTN_BOOT) == LOW;
  bool freshCard = true;
  if (SD.exists(SD_MARKER)) {
    File m = SD.open(SD_MARKER, "r");
    if (m && m.readString() == SDPACK_VERSION) freshCard = false;
    if (m) m.close();
  }
  if (forcedSetup) {
    setupInstall(false);
    delay(400);
  } else if (freshCard) {
    splashDone();
    setupWelcome();
    setupInstall(true);
    setupDone();
    gfx->fillScreen(C_BLACK);
    splashFrame(62, "scanning apps");
  }
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

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN_BOOT, INPUT_PULLUP);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  sysLed(0, 255, 0);
  xTaskCreatePinnedToCore(uiTask, "cydos_ui", 32768, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
