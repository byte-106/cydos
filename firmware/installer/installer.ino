#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <SD.h>

#include "data.h"

#define BLACK 0x0000
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0

#define TFT_BL 21
#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

Arduino_DataBus *bus = new Arduino_ESP32SPI(2, 15, 13, 14, 12);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 12, 1);
SPIClass sdSPI(HSPI);

void msg(const char *a, const char *b, uint16_t c) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(c);
  gfx->setTextSize(2);
  gfx->setCursor(20, 90);
  gfx->print(a);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(20, 130);
  gfx->print(b);
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  gfx->begin(80000000);
  gfx->invertDisplay(false);

  msg("SD INSTALLER", "Mounting card...", YELLOW);
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI, 30000000)) {
    Serial.println("SD FAIL");
    msg("NO SD CARD", "Insert card & press RESET", RED);
    while (true) delay(1000);
  }

  File root = SD.open("/");
  Serial.println("--- ROOT BEFORE ---");
  File e = root.openNextFile();
  while (e) {
    Serial.println(e.name());
    e.close();
    e = root.openNextFile();
  }
  root.close();

  int ok = 0;
  for (int i = 0; i < EMBED_COUNT; i++) {
    const EmbedFile &f = EMBED_FILES[i];
    String p = f.path;
    int slash = p.lastIndexOf('/');
    String dir = p.substring(0, slash);
    if (!SD.exists(dir)) {
      String walk = "";
      int start = 1;
      while (true) {
        int next = p.indexOf('/', start);
        if (next < 0 || next > slash) break;
        walk = p.substring(0, next);
        if (!SD.exists(walk)) SD.mkdir(walk);
        start = next + 1;
      }
    }
    SD.remove(p);
    File out = SD.open(p, FILE_WRITE);
    if (!out) {
      Serial.printf("OPEN FAIL %s\n", f.path);
      continue;
    }
    out.write(f.data, f.len);
    out.close();
    ok++;
    Serial.printf("wrote %s (%u)\n", f.path, (unsigned)f.len);
    gfx->fillRect(20, 150, 280, 40, BLACK);
    gfx->setCursor(20, 160);
    gfx->print(String(f.path).substring(6));
  }

  Serial.printf("DONE %d/%d\n", ok, EMBED_COUNT);
  char line[48];
  snprintf(line, sizeof(line), "Done: %d/%d files written", ok, EMBED_COUNT);
  msg("INSTALL OK", line, GREEN);
  gfx->setTextSize(1);
  gfx->setTextColor(YELLOW);
  gfx->setCursor(20, 180);
  gfx->print("Reflash CYDOS firmware next.");
}

void loop() {
  delay(1000);
}
