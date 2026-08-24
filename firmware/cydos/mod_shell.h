#pragma once
#include <vector>
#include <algorithm>
#include "mod_config.h"
#include "mod_display.h"
#include "mod_input.h"
#include "mod_kv.h"
#include "mod_sys.h"
#include "mod_ui.h"
#include "mod_kbd.h"
#include "mod_net.h"
#include "mod_luart.h"

struct AppEntry {
  String dir;
  String name;
};
static std::vector<AppEntry> g_apps;
static int deskPage = 0;
static bool g_inApp = false;
static bool startMenuOpen = false;

static void runApp(const AppEntry &app);
static void toolFiles();

static void scanApps() {
  g_apps.clear();
  File root = SD.open("/apps");
  if (!root) {
    SD.mkdir("/apps");
    return;
  }
  std::vector<String> dirs;
  File e = root.openNextFile();
  while (e) {
    if (e.isDirectory()) dirs.push_back(String(e.name()));
    e.close();
    e = root.openNextFile();
  }
  root.close();
  std::sort(dirs.begin(), dirs.end());
  for (auto &d : dirs) {
    String dir = "/apps/" + d + "/";
    if (!SD.exists(dir + "app.lua")) continue;
    String disp = d;
    File mf = SD.open(dir + "manifest.txt", "r");
    if (mf) {
      String line = mf.readStringUntil('\n');
      line.trim();
      if (line.length()) disp = line;
      mf.close();
    }
    g_apps.push_back({ dir, disp });
  }
}

static void pollPowerButton();
static void showMessageBox(const char *title, const String &msg);

#define APPS_PER_PAGE 8
#define DESK_H (SCREEN_H - TASKBAR_H)

static void drawAppIcon(const AppEntry &app, int16_t x, int16_t y) {
  const int16_t cellW = 74, iconS = 48;
  String iconPath = app.dir + "icon.jpg";
  if (SD.exists(iconPath)) {
    TJpgDec.setJpgScale(1);
    TJpgDec.drawSdJpg(x + (cellW - iconS) / 2, y, iconPath.c_str());
  } else {
    uint16_t hue = 0;
    for (char c : app.name) hue += c;
    uint16_t color = rgb565((hue * 37) & 255, (hue * 91) & 255, (hue * 53) & 255);
    gfx->fillRect(x + (cellW - iconS) / 2, y, iconS, iconS, color);
    gfx->drawRect(x + (cellW - iconS) / 2, y, iconS, iconS, C_WHITE);
    char initial[2] = { toupper(app.name[0]), 0 };
    gfx->setTextSize(3);
    gfx->setTextColor(C_WHITE);
    int16_t tw = textWidth(initial, 3);
    gfx->setCursor(x + (cellW - tw) / 2, y + 13);
    gfx->print(initial);
  }
  String label = app.name;
  if (label.length() > 13) label = label.substring(0, 12) + "~";
  gfx->setTextSize(1);
  gfx->setTextColor(C_DESKTXT);
    int16_t lw = textWidth(label.c_str(), 1);
    gfx->setCursor(x + max((int)0, ((int)cellW - lw) / 2), y + iconS + 5);
  gfx->print(label);
}

static void drawDesktop() {
  gfx->fillRect(0, 0, SCREEN_W, DESK_H, C_DESKTOP);
  int pages = max(1, (int)((g_apps.size() + APPS_PER_PAGE - 1) / APPS_PER_PAGE));
  deskPage = constrain(deskPage, 0, pages - 1);
  int startIdx = deskPage * APPS_PER_PAGE;
  for (int i = 0; i < APPS_PER_PAGE && startIdx + i < (int)g_apps.size(); i++) {
    int col = i % 4, row = i / 4;
    drawAppIcon(g_apps[startIdx + i], 10 + col * 78, 14 + row * 90);
  }
  if (!g_apps.size()) centerText(84, "No apps on SD card", C_LGRAY, 2);
  if (!g_apps.size()) centerText(104, "Copy the apps folder to it", C_DGRAY, 1);
  if (pages > 1) {
    char pb[6], nb[6];
    snprintf(pb, sizeof(pb), "%d<", deskPage);
    snprintf(nb, sizeof(nb), ">%d", pages);
    UiButton prev(240, 120, 36, 34, deskPage > 0 ? "<" : ""), next(280, 120, 36, 34, deskPage < pages - 1 ? ">" : "");
    prev.draw();
    next.draw();
  }
}

static void drawTrayClock() {
  struct tm t;
  char timebuf[8] = "--:--";
  if (netTime(t)) snprintf(timebuf, sizeof(timebuf), "%02d:%02d", t.tm_hour, t.tm_min);
  gfx->fillRect(SCREEN_W - 68, SCREEN_H - TASKBAR_H + 2, 64, TASKBAR_H - 4, C_FACE);
  gfx->setTextSize(2);
  gfx->setTextColor(C_BLACK);
  gfx->setCursor(SCREEN_W - 62, SCREEN_H - TASKBAR_H + 14);
  gfx->print(timebuf);
  gfx->fillRect(SCREEN_W - 92, SCREEN_H - TASKBAR_H + 15, 20, 16, C_FACE);
  if (netConnected()) {
    gfx->fillRect(SCREEN_W - 88, SCREEN_H - TASKBAR_H + 25, 4, 4, C_BLACK);
    gfx->drawLine(SCREEN_W - 86, SCREEN_H - TASKBAR_H + 22, SCREEN_W - 80, SCREEN_H - TASKBAR_H + 16, C_BLACK);
    gfx->drawRect(SCREEN_W - 89, SCREEN_H - TASKBAR_H + 24, 9, 9, C_BLACK);
  } else {
    gfx->setTextSize(1);
    gfx->setTextColor(netState == NET_ERROR ? C_RED : C_DGRAY);
    gfx->setCursor(SCREEN_W - 88, SCREEN_H - TASKBAR_H + 21);
    gfx->print(netState == NET_ERROR ? "x" : "~");
  }
}

static void drawTaskbar() {
  gfx->fillRect(0, SCREEN_H - TASKBAR_H, SCREEN_W, TASKBAR_H, C_FACE);
  gfx->drawFastHLine(0, SCREEN_H - TASKBAR_H, SCREEN_W, C_HILIGHT);
  UiButton start(4, SCREEN_H - TASKBAR_H + 3, 68, 38, "START");
  start.draw();
  gfx->setTextSize(2);
  gfx->setTextColor(C_TITLE);
  gfx->setCursor(17, SCREEN_H - TASKBAR_H + 14);
  gfx->print("C");
  drawTrayClock();
}

static const char *menuItems[5] = { "Files", "Editor", "Images", "About", "Power" };
#define MENU_X 4
#define MENU_Y (SCREEN_H - TASKBAR_H - 204)
#define MENU_W 210

static void drawStartMenu() {
  bevel(MENU_X, MENU_Y, MENU_W, 204, true);
  gfx->fillRect(MENU_X + 2, MENU_Y + 2, MENU_W - 4, 200, C_FACE);
  gfx->fillRect(MENU_X + 2, MENU_Y + 2, 30, 200, C_TITLE);
  gfx->setTextSize(2);
  gfx->setTextColor(C_TITLETEXT);
  gfx->setCursor(MENU_X + 9, MENU_Y + 176);
  gfx->print("C");
  gfx->setTextSize(2);
  gfx->setTextColor(C_BLACK);
  for (int i = 0; i < 5; i++) {
    gfx->setCursor(MENU_X + 44, MENU_Y + 18 + i * 38);
    gfx->print(menuItems[i]);
  }
}

static int menuHit(int16_t x, int16_t y) {
  if (!inRect(x, y, MENU_X + 34, MENU_Y + 4, MENU_W - 38, 196)) return -1;
  int idx = (y - MENU_Y - 4) / 38;
  return (idx >= 0 && idx <= 4) ? idx : -1;
}

static void shellDrawAppChrome() {
  if (g_appMode == AM_NONE) return;
  uint32_t now = millis();
  static uint32_t last = 0;
  if (now - last < 250) return;
  last = now;
  drawTitleBar(g_appName.c_str(), true);
}

static bool shellInterceptTap(const TouchEvent &e) {
  if (g_appMode == AM_NONE || e.y >= TITLEBAR_H) return false;
  if (inRect(e.x, e.y, SCREEN_W - TITLEBAR_H, 2, TITLEBAR_H - 4, TITLEBAR_H - 4)) g_exitReq = true;
  return true;
}

static void shellShowAppError(const String &name, const String &err) {
  showMessageBox(("Error - " + name).c_str(), err);
}

static void appEventLoop() {
  g_inApp = true;
  uint32_t lastFrame = millis();
  while (!g_exitReq && g_appMode == AM_EVENT) {
    touchPoll();
    TouchEvent e;
    while (tePop(e)) {
      if (shellInterceptTap(e)) {
        teWaitRelease();
        continue;
      }
      luaDispatchEvent(e);
    }
    uint32_t now = millis();
    uint32_t dt = now - lastFrame;
    lastFrame = now;
    luaCallLoop(dt);
    netTick();
    pollPowerButton();
    settingsMaybeSync();
    shellDrawAppChrome();
    vTaskDelay(3);
  }
  g_inApp = false;
}

static void runApp(const AppEntry &app) {
  gfx->fillScreen(C_BLACK);
  sysBeep(1100, 40);
  bool started = startLuaApp(app.dir, app.name);
  if (started && g_appMode == AM_EVENT) appEventLoop();
  stopLuaApp();
  sysBeep(700, 40);
}

static void powerMenu() {
  drawWindowFrame("Power", 60, 40, 200, 150, true);
  UiButton offB(80, 84, 160, 38, "Power off"), restB(80, 130, 160, 38, "Restart");
  offB.draw();
  restB.draw();
  while (true) {
    touchPoll();
    TouchEvent e;
    while (tePop(e)) {
      if (e.type != TE_PRESS) continue;
      teWaitRelease();
      if (offB.hit(e.x, e.y)) {
        sysSleepNow();
        return;
      }
      if (restB.hit(e.x, e.y)) {
        sysReboot();
        return;
      }
      if (closeButtonHit(60, 40, 200, e.x, e.y)) return;
    }
    vTaskDelay(5);
  }
}

static uint32_t pwrDownSince = 0;

static void pollPowerButton() {
  bool down = digitalRead(PIN_BTN_BOOT) == LOW;
  if (down && pwrDownSince == 0) pwrDownSince = millis();
  if (down && millis() - pwrDownSince > 800) {
    pwrDownSince = 0;
    sysBeep(500, 80);
    teWaitRelease();
    powerMenu();
    if (g_appMode == AM_NONE) {
      drawDesktop();
      drawTaskbar();
    }
  }
  if (!down) pwrDownSince = 0;
}

static void showMessageBox(const char *title, const String &msg) {
  int wx = 14, wy = 46, ww = 292, wh = 148;
  drawWindowFrame(title, wx, wy, ww, wh, true);
  gfx->setTextSize(1);
  gfx->setTextColor(C_BLACK);
  int col = 0, line = 0;
  int16_t ty = wy + 40;
  gfx->setCursor(wx + 10, ty);
  int maxChars = 45, maxLines = 6;
  for (const char *p = msg.c_str(); *p && line < maxLines; p++) {
    if (*p == '\n' || col >= maxChars) {
      gfx->println();
      gfx->setCursor(wx + 10, ty += 11);
      col = 0;
      if (++line >= maxLines) break;
      if (*p == '\n') continue;
    }
    gfx->print(*p);
    col++;
  }
  UiButton ok(wx + ww / 2 - 50, wy + wh - 44, 100, 34, "OK");
  ok.draw();
  while (true) {
    touchPoll();
    TouchEvent e;
    while (tePop(e))
      if (e.type == TE_PRESS && (ok.hit(e.x, e.y) || closeButtonHit(wx, wy, ww, e.x, e.y))) {
        teWaitRelease();
        return;
      }
    vTaskDelay(5);
  }
}

static void toolAbout() {
  gfx->fillScreen(C_DESKTOP);
  drawWindowFrame("About CYDOS", 20, 30, 280, 170, true);
  centerText(70, "CYDOS", C_TITLE, 4);
  centerText(106, "CYD Operating System v" CYDOS_VERSION, C_BLACK, 1);
  centerText(118, themeName(THEME_VER), C_DGRAY, 1);
  centerText(130, "Lua-powered app platform", C_DGRAY, 1);
  centerText(120, "for the Cheap Yellow Display", C_DGRAY, 1);
  centerText(146, "Hold BOOT for power menu", C_DGRAY, 1);
  centerText(170, "Tap anywhere to close", C_DGRAY, 1);
  while (true) {
    touchPoll();
    TouchEvent e;
    while (tePop(e))
      if (e.type == TE_PRESS) {
        teWaitRelease();
        return;
      }
    vTaskDelay(5);
  }
}

static void runViewer(const String &path);

static void toolEditor(const String &openPath) {
  std::vector<String> lines;
  String fname = openPath;
  bool dirty = false;

  auto splitBuffer = [&](const String &content) {
    lines.clear();
    int start = 0;
    for (int i = 0; i <= (int)content.length(); i++) {
      if (i == (int)content.length() || content[i] == '\n') {
        lines.push_back(content.substring(start, i));
        start = i + 1;
      }
    }
    if (!lines.size()) lines.push_back("");
  };

  if (openPath.length() && SD.exists(openPath)) {
    File f = SD.open(openPath, "r");
    if (f) {
      String content;
      while (f.available()) content += (char)f.read();
      f.close();
      splitBuffer(content);
    }
  } else {
    lines.push_back("");
    if (!openPath.length()) fname = "/untitled.txt";
  }

  int curLine = lines.size() - 1, curCol = lines[curLine].length(), topLine = 0;
  kbSetY(142);
  kbShift = false;
  kbNumPage = false;

  const int textTop = 64, lineH = 10, visLines = (142 - 6 - textTop) / lineH;

  auto ensureVisible = [&]() {
    if (curLine < topLine) topLine = curLine;
    if (curLine >= topLine + visLines) topLine = curLine - visLines + 1;
    if (topLine < 0) topLine = 0;
  };

  auto renderText = [&]() {
    ensureVisible();
    drawTitleBar("Editor", true);
    gfx->fillRect(0, textTop, SCREEN_W, visLines * lineH + 4, C_BLACK);
    gfx->setTextSize(1);
    for (int r = 0; r < visLines && topLine + r < (int)lines.size(); r++) {
      const String &ln = lines[topLine + r];
      gfx->setCursor(2, textTop + r * lineH);
      gfx->setTextColor(topLine + r == curLine ? C_YELLOW : C_LGRAY);
      if (ln.length()) gfx->print(ln.substring(0, min((size_t)52, ln.length())));
    }
    int cx = 2 + min((size_t)51, (size_t)curCol) * 6;
    int cy = textTop + (curLine - topLine) * lineH;
    gfx->fillRect(cx, cy, 6, 9, C_GREEN);
    gfx->fillRect(0, 30, SCREEN_W, 32, C_FACE);
    gfx->setTextSize(1);
    gfx->setTextColor(C_DGRAY);
    String shortName = fname;
    if (shortName.length() > 24) shortName = "..." + shortName.substring(shortName.length() - 21);
    gfx->setCursor(6, 40);
    gfx->printf("%s  L%d:%d%s", shortName.c_str(), curLine + 1, curCol + 1, dirty ? " *" : "");
    UiButton saveB(250, 33, 64, 26, "SAVE");
    saveB.draw();
    kbDraw();
  };

  auto doSave = [&]() {
    if (fname.length() && fname != "/untitled.txt") {
      File f = SD.open(fname, FILE_WRITE);
      if (f) {
        for (size_t i = 0; i < lines.size(); i++) {
          if (i) f.print("\n");
          f.print(lines[i]);
        }
        f.close();
        dirty = false;
        sysBeep(1300, 50);
      }
    } else {
      String nn;
      if (kbInput("Save as:", nn, false, 32) && nn.length()) {
        fname = nn.startsWith("/") ? nn : "/" + nn;
        File f = SD.open(fname, FILE_WRITE);
        if (f) {
          for (size_t i = 0; i < lines.size(); i++) {
            if (i) f.print("\n");
            f.print(lines[i]);
          }
          f.close();
          dirty = false;
          sysBeep(1300, 50);
        }
      }
    }
  };

  renderText();
  bool exitEditor = false;
  while (!exitEditor) {
    touchPoll();
    TouchEvent e;
    while (tePop(e)) {
      if (e.type != TE_PRESS) continue;
      if (e.y < kbY0) {
        teWaitRelease();
        if (closeButtonHit(0, 0, SCREEN_W, e.x, e.y)) {
          if (dirty) doSave();
          exitEditor = true;
          break;
        }
        if (inRect(e.x, e.y, 250, 33, 64, 26)) {
          doSave();
          renderText();
          break;
        }
        if (e.y >= textTop && e.y < textTop + visLines * lineH) {
          int l = topLine + (e.y - textTop) / lineH;
          if (l >= 0 && l < (int)lines.size()) {
            curLine = l;
            curCol = min((size_t)(e.x - 2) / 6, lines[l].length());
            renderText();
          }
        }
        break;
      }
      int k = kbPoll(e.x, e.y);
      if (k == 0) break;
      teWaitRelease();
      String &ln = lines[curLine];
      if (k >= 32 && k < 128) {
        ln = ln.substring(0, curCol) + (char)k + ln.substring(curCol);
        curCol++;
        if (kbShift && !kbNumPage) kbShift = false;
        dirty = true;
      } else if (k == KB_BS) {
        if (curCol > 0) {
          ln = ln.substring(0, curCol - 1) + ln.substring(curCol);
          curCol--;
        } else if (curLine > 0) {
          curCol = lines[curLine - 1].length();
          lines[curLine - 1] += ln;
          lines.erase(lines.begin() + curLine);
          curLine--;
        }
        dirty = true;
      } else if (k == KB_SHIFT) {
        kbShift = !kbShift;
      } else if (k == KB_PAGE) {
        kbNumPage = !kbNumPage;
      } else if (k == ' ') {
        ln = ln.substring(0, curCol) + " " + ln.substring(curCol);
        curCol++;
        dirty = true;
      }
      if (k == KB_ENTER || k == '\n') {
        String rest = ln.substring(curCol);
        ln = ln.substring(0, curCol);
        lines.insert(lines.begin() + curLine + 1, rest);
        curLine++;
        curCol = 0;
        dirty = true;
      }
      renderText();
      break;
    }
    if (digitalRead(PIN_BTN_BOOT) == LOW) {
      uint32_t t0 = millis();
      while (digitalRead(PIN_BTN_BOOT) == LOW && millis() - t0 < 800) vTaskDelay(10);
      if (millis() - t0 >= 800) {
        doSave();
        exitEditor = true;
      }
    }
    vTaskDelay(4);
  }
  kbSetY(132);
}

static void toolEditorEntry() {
  toolEditor("");
}

static void toolViewerEntry() {
  gfx->fillScreen(C_DESKTOP);
  drawWindowFrame("Images", 0, 0, SCREEN_W, TITLEBAR_H, true);
  gfx->fillRect(0, TITLEBAR_H, SCREEN_W, SCREEN_H - TITLEBAR_H, C_DESKTOP);
  centerText(100, "Tap a .jpg file in Files", C_WHITE, 2);
  centerText(130, "(Start > Files)", C_LGRAY, 1);
  uint32_t t0 = millis();
  while (millis() - t0 < 1800) {
    touchPoll();
    TouchEvent e;
    while (tePop(e))
      if (e.type == TE_PRESS) return;
    vTaskDelay(10);
  }
}

static void runViewer(const String &path) {
  gfx->fillScreen(C_BLACK);
  if (path.endsWith(".bmp")) {
    File f = SD.open(path, "r");
    if (!f) return;
    uint8_t hdr[54];
    f.read(hdr, 54);
    int32_t w = *(int32_t *)&hdr[18], h = *(int32_t *)&hdr[22];
    uint16_t bpp = *(uint16_t *)&hdr[28];
    if (bpp != 24 || w < 1 || w > 480 || abs(h) > 480) {
      f.close();
      showMessageBox("Images", "Unsupported BMP");
      return;
    }
    bool flip = h > 0;
    h = abs(h);
    int rowSize = (w * 3 + 3) & ~3;
    for (int yy = 0; yy < h && yy < SCREEN_H; yy++) {
      int dy = flip ? (h - 1 - yy) : yy;
      f.seek(54 + yy * rowSize);
      for (int xx = 0; xx < w && xx < SCREEN_W; xx++) {
        uint8_t bgr[3];
        f.read(bgr, 3);
        gfx->drawPixel(xx, TITLEBAR_H + dy, rgb565(bgr[2], bgr[1], bgr[0]));
      }
    }
    f.close();
  } else {
    File f = SD.open(path, "r");
    if (!f) return;
    size_t sz = min((size_t)f.size(), (size_t)60000);
    uint8_t *buf = (uint8_t *)malloc(sz);
    if (!buf) {
      f.close();
      return;
    }
    f.read(buf, sz);
    f.close();
    uint16_t w, h;
    TJpgDec.getJpgSize(&w, &h, buf, sz);
    int scale = 1;
    while (scale < 8 && ((w + scale - 1) / scale > SCREEN_W || (h + scale - 1) / scale > SCREEN_H - TITLEBAR_H)) scale *= 2;
    TJpgDec.setJpgScale(scale);
    int dx = (SCREEN_W - (w + scale - 1) / scale) / 2;
    int dy = TITLEBAR_H + max(0, ((SCREEN_H - TITLEBAR_H) - (h + scale - 1) / scale) / 2);
    TJpgDec.drawJpg(dx, dy, buf, sz);
    free(buf);
  }
  drawWindowFrame(path.substring(path.lastIndexOf('/') + 1).c_str(), 0, 0, SCREEN_W, TITLEBAR_H, true);
  while (true) {
    touchPoll();
    TouchEvent e;
    while (tePop(e))
      if (e.type == TE_PRESS) {
        teWaitRelease();
        return;
      }
    vTaskDelay(5);
  }
}

static void toolFiles() {
  String path = "/";
  int sel = -1;
  std::vector<String> names;
  std::vector<bool> isDir;
  int scroll = 0;
  const int rowH = 42, listY = 66, toolbarY = 30;

  auto reload = [&]() {
    names.clear();
    isDir.clear();
    sel = -1;
    scroll = 0;
    if (path != "/") {
      names.push_back("..");
      isDir.push_back(true);
    }
    File dir = SD.open(path);
    if (!dir) return;
    std::vector<String> ds, fs2;
    File e = dir.openNextFile();
    while (e) {
      String n = String(e.name());
      if (n != "System Volume Information") {
        if (e.isDirectory()) ds.push_back(n);
        else fs2.push_back(n);
      }
      e.close();
      e = dir.openNextFile();
    }
    dir.close();
    std::sort(ds.begin(), ds.end());
    std::sort(fs2.begin(), fs2.end());
    for (auto &s : ds) {
      names.push_back(s);
      isDir.push_back(true);
    }
    for (auto &s : fs2) {
      names.push_back(s);
      isDir.push_back(false);
    }
  };

  auto renderAll = [&]() {
    gfx->fillRect(0, TITLEBAR_H, SCREEN_W, SCREEN_H - TITLEBAR_H, C_DESKTOP);
    drawWindowFrame("Files", 0, 0, SCREEN_W, TITLEBAR_H, true);
    gfx->setTextSize(1);
    gfx->setTextColor(C_LGRAY);
    gfx->setCursor(6, 34);
    String p = path;
    if (p.length() > 26) p = "..." + p.substring(p.length() - 23);
    gfx->print(p);
    UiButton up(277, 32, 38, 26, "UP"), del(237, 32, 38, 26, "DEL"), ren(197, 32, 38, 26, "REN");
    ren.draw();
    del.draw();
    up.draw();
    int rows = (SCREEN_H - listY) / rowH + 1;
    for (int r = 0; r < rows; r++) {
      int idx = scroll + r;
      int y = listY + r * rowH;
      gfx->fillRect(0, y, SCREEN_W, rowH, C_DESKTOP);
      if (idx < 0 || idx >= (int)names.size()) continue;
      bool selected = idx == sel;
      if (selected) gfx->fillRect(0, y, SCREEN_W, rowH - 2, C_SELLINE);
      gfx->setTextSize(2);
      gfx->setTextColor(selected ? C_WHITE : (isDir[idx] ? C_YELLOW : C_WHITE));
      String n = names[idx];
      if (n.length() > 19) n = n.substring(0, 18) + "~";
      gfx->setCursor(12, y + 12);
      gfx->print(isDir[idx] ? ("[" + n + "]") : n);
    }
  };

  reload();
  renderAll();

  while (true) {
    pollPowerButton();
    touchPoll();
    static int lastDragY = -1;
    if (teIsDown() && teCurY() >= listY) {
      if (lastDragY >= 0) {
        int dy = teCurY() - lastDragY;
        if (abs(dy) >= rowH) {
          scroll -= dy / rowH;
          int maxScroll = max(0, (int)names.size() - 4);
          scroll = constrain(scroll, 0, maxScroll);
          lastDragY = teCurY();
          renderAll();
        }
      } else lastDragY = teCurY();
    } else
      lastDragY = -1;

    TouchEvent e;
    while (tePop(e)) {
      if (e.type != TE_PRESS) continue;
      teWaitRelease();
      if (closeButtonHit(0, 0, SCREEN_W, e.x, e.y)) return;
      if (inRect(e.x, e.y, 277, toolbarY, 38, 26)) {
        if (path != "/") {
          int cut = path.lastIndexOf('/', path.length() - 2);
          path = path.substring(0, cut + 1);
          reload();
          renderAll();
        }
        goto nextEvt;
      }
      if (inRect(e.x, e.y, 237, toolbarY, 38, 26)) {
        if (sel >= 0 && sel < (int)names.size() && names[sel] != "..") {
          String target = path == "/" ? "/" + names[sel] : path + names[sel];
          SD.remove(target);
          if (SD.exists(target)) SD.rmdir(target);
          reload();
          renderAll();
        }
        goto nextEvt;
      }
      if (inRect(e.x, e.y, 197, toolbarY, 38, 26)) {
        if (sel >= 0 && sel < (int)names.size() && names[sel] != ".." && names[sel] != "..") {
          String nn;
          if (kbInput("Rename to:", nn, false, 32) && nn.length()) {
            String oldP = path == "/" ? "/" + names[sel] : path + names[sel];
            String newP = path == "/" ? "/" + nn : path + nn;
            SD.rename(oldP, newP);
            reload();
          }
          renderAll();
        }
        goto nextEvt;
      }
      if (e.y >= listY) {
        int idx = scroll + (e.y - listY) / rowH;
        if (idx < 0 || idx >= (int)names.size()) goto nextEvt;
        sel = idx;
        String n = names[idx];
        if (isDir[idx]) {
          if (n == "..") {
            int cut = path.lastIndexOf('/', path.length() - 2);
            path = path.substring(0, cut + 1);
          } else {
            path = path.endsWith("/") ? path + n : path + "/" + n;
            if (!path.endsWith("/")) path += "/";
          }
          reload();
          renderAll();
        } else {
          String full = path == "/" ? "/" + n : path + n;
          String lower = full;
          lower.toLowerCase();
          if (lower.endsWith(".jpg") || lower.endsWith(".jpeg") || lower.endsWith(".bmp")) {
            runViewer(full);
            renderAll();
          } else if (lower.endsWith(".txt") || lower.endsWith(".md") || lower.endsWith(".cfg") ||
                     lower.endsWith(".ini") || lower.endsWith(".log") || lower.endsWith(".lua")) {
            toolEditor(full);
            renderAll();
          } else {
            sysBeep(300, 60);
            renderAll();
          }
        }
        goto nextEvt;
      }
    nextEvt:;
    }
    vTaskDelay(5);
  }
}

static void desktopLoop() {
  drawDesktop();
  drawTaskbar();
  startMenuOpen = false;
  uint32_t lastClock = millis();

  while (true) {
    pollPowerButton();
    touchPoll();
    netTick();
    TouchEvent e;
    while (tePop(e)) {
      if (e.type != TE_PRESS) continue;
      teWaitRelease();
      if (inRect(e.x, e.y, 4, SCREEN_H - TASKBAR_H + 3, 68, 38)) {
        startMenuOpen = !startMenuOpen;
        sysBeep(900, 20);
        if (startMenuOpen)
          drawStartMenu();
        else {
          drawDesktop();
          drawTaskbar();
        }
        goto handled;
      }
      if (startMenuOpen) {
        int mi = menuHit(e.x, e.y);
        if (mi >= 0) {
          startMenuOpen = false;
          switch (mi) {
            case 0: toolFiles(); break;
            case 1: toolEditorEntry(); break;
            case 2: toolViewerEntry(); break;
            case 3: toolAbout(); break;
            case 4: powerMenu(); break;
          }
          drawDesktop();
          drawTaskbar();
          goto handled;
        }
        startMenuOpen = false;
        drawDesktop();
        drawTaskbar();
        goto handled;
      }
      if (e.y < DESK_H) {
        int pages = max(1, (int)((g_apps.size() + APPS_PER_PAGE - 1) / APPS_PER_PAGE));
        if (pages > 1 && inRect(e.x, e.y, 240, 120, 76, 34)) {
          if (deskPage > 0) {
            deskPage--;
            drawDesktop();
          }
          goto handled;
        }
        if (pages > 1 && inRect(e.x, e.y, 240, 120, 76, 34)) goto handled;
        if (pages > 1 && e.x >= 280 && e.y >= 120 && e.y < 154 && deskPage < pages - 1) {
          deskPage++;
          drawDesktop();
          goto handled;
        }
        int col = (e.x - 6) / 78, row = (e.y - 10) / 90;
        if (col >= 0 && col < 4 && row >= 0 && row < 2) {
          int idx = deskPage * APPS_PER_PAGE + row * 4 + col;
          if (idx >= 0 && idx < (int)g_apps.size()) {
            runApp(g_apps[idx]);
            scanApps();
            drawDesktop();
            drawTaskbar();
            goto handled;
          }
        }
      }
    handled:;
    }
    if (millis() - lastClock > 30000) {
      lastClock = millis();
      drawTrayClock();
    }
    settingsMaybeSync();
    vTaskDelay(8);
  }
}
