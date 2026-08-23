#pragma once
#include <SD.h>
#include <TJpg_Decoder.h>
#include <vector>
#include <algorithm>
extern "C" {
#include "src/lua/lua.h"
#include "src/lua/lualib.h"
#include "src/lua/lauxlib.h"
#include "src/lua/lstate.h"
}
extern "C" void cydosOpenLibs(lua_State *L);
#include "mod_config.h"
#include "mod_display.h"
#include "mod_input.h"
#include "mod_kv.h"
#include "mod_sys.h"
#include "mod_net.h"
#include "mod_kbd.h"

#define LUA_EXIT_MAGIC "CYDOS_EXIT"

enum AppMode : uint8_t { AM_NONE, AM_EVENT, AM_FREERUN };

static lua_State *g_lua = nullptr;
static AppMode g_appMode = AM_NONE;
static bool g_exitReq = false;
static uint32_t g_lastYield = 0;
static String g_appName = "", g_appDir = "", g_dataDir = "";
static bool g_hasLoop = false, g_hasTouch = false;

static void shellDrawAppChrome();
static bool shellInterceptTap(const TouchEvent &e);
static void shellShowAppError(const String &name, const String &err);

static const char *luaFileReader(lua_State *L, void *ud, size_t *sz) {
  static char buf[1024];
  File *f = (File *)ud;
  int n = f->read((uint8_t *)buf, sizeof(buf));
  *sz = (size_t)n;
  return n > 0 ? buf : nullptr;
}

static int l_millis(lua_State *L) {
  lua_pushinteger(L, (lua_Integer)millis());
  return 1;
}

static void pumpIdle(uint32_t ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    touchPoll();
    netTick();
    vTaskDelay(2);
  }
}

static int l_delay(lua_State *L) {
  lua_Integer ms = luaL_checkinteger(L, 1);
  uint32_t t0 = millis();
  while ((lua_Integer)(millis() - t0) < ms && !g_exitReq) {
    g_lastYield = millis();
    touchPoll();
    netTick();
    TouchEvent ev;
    while (tePop(ev)) {
      if (ev.type == TE_PRESS && shellInterceptTap(ev)) {
        teWaitRelease();
        continue;
      }
      if (g_hasTouch && g_lua) {
        g_lastYield = millis();
        lua_getglobal(g_lua, "onTouch");
        if (!lua_isfunction(g_lua, -1)) {
          lua_pop(g_lua, 1);
        } else {
          lua_pushinteger(g_lua, ev.type);
          lua_pushinteger(g_lua, ev.x);
          lua_pushinteger(g_lua, ev.y);
          if (lua_pcall(g_lua, 3, 0, 0) != 0) lua_pop(g_lua, 1);
        }
      }
    }
    vTaskDelay(2);
  }
  g_lastYield = millis();
  return 0;
}

static int l_beep(lua_State *L) {
  tone(PIN_BUZZER, (int)luaL_optinteger(L, 1, 1000), (int)luaL_optinteger(L, 2, 100));
  return 0;
}

static int l_cls(lua_State *L) {
  gfx->fillScreen((uint16_t)luaL_optinteger(L, 1, C_BLACK));
  return 0;
}

static int l_px(lua_State *L) {
  gfx->drawPixel((int16_t)luaL_checkinteger(L, 1), (int16_t)luaL_checkinteger(L, 2), (uint16_t)luaL_checkinteger(L, 3));
  return 0;
}

static int l_line(lua_State *L) {
  gfx->drawLine((int16_t)luaL_checkinteger(L, 1), (int16_t)luaL_checkinteger(L, 2), (int16_t)luaL_checkinteger(L, 3),
                (int16_t)luaL_checkinteger(L, 4), (uint16_t)luaL_checkinteger(L, 5));
  return 0;
}

static int l_rect(lua_State *L) {
  gfx->drawRect((int16_t)luaL_checkinteger(L, 1), (int16_t)luaL_checkinteger(L, 2), (int16_t)luaL_checkinteger(L, 3),
                (int16_t)luaL_checkinteger(L, 4), (uint16_t)luaL_checkinteger(L, 5));
  return 0;
}

static int l_fillRect(lua_State *L) {
  gfx->fillRect((int16_t)luaL_checkinteger(L, 1), (int16_t)luaL_checkinteger(L, 2), (int16_t)luaL_checkinteger(L, 3),
                (int16_t)luaL_checkinteger(L, 4), (uint16_t)luaL_checkinteger(L, 5));
  return 0;
}

static int l_fillCircle(lua_State *L) {
  int x = (int)luaL_checkinteger(L, 1), y = (int)luaL_checkinteger(L, 2), r = (int)luaL_checkinteger(L, 3);
  uint16_t c = (uint16_t)luaL_checkinteger(L, 4);
  gfx->fillCircle(x, y, r, c);
  return 0;
}

static int l_circle(lua_State *L) {
  int x = (int)luaL_checkinteger(L, 1), y = (int)luaL_checkinteger(L, 2), r = (int)luaL_checkinteger(L, 3);
  uint16_t c = (uint16_t)luaL_checkinteger(L, 4);
  if (lua_toboolean(L, 5)) gfx->fillCircle(x, y, r, c);
  else gfx->drawCircle(x, y, r, c);
  return 0;
}

static int l_text(lua_State *L) {
  int16_t x = (int16_t)luaL_checkinteger(L, 1), y = (int16_t)luaL_checkinteger(L, 2);
  const char *s = luaL_checkstring(L, 3);
  uint16_t c = (uint16_t)luaL_optinteger(L, 4, C_WHITE);
  int size = std::max(1, std::min(6, (int)luaL_optinteger(L, 5, 1)));
  gfx->setTextSize(size);
  gfx->setTextColor(c);
  gfx->setCursor(x, y);
  gfx->print(s);
  lua_pushinteger(L, (lua_Integer)(strlen(s) * 6 * size));
  return 1;
}

static int l_textW(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  int size = std::max(1, std::min(6, (int)luaL_optinteger(L, 2, 1)));
  lua_pushinteger(L, (lua_Integer)(strlen(s) * 6 * size));
  return 1;
}

static int l_rgb565(lua_State *L) {
  lua_pushinteger(L, (lua_Integer)rgb565((uint8_t)luaL_checkinteger(L, 1), (uint8_t)luaL_checkinteger(L, 2),
                                         (uint8_t)luaL_checkinteger(L, 3)));
  return 1;
}

static bool jpg_out(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bmp) {
  if (y >= SCREEN_H) return false;
  int16_t dw = w;
  if (x + w > SCREEN_W) dw = SCREEN_W - x;
  if (dw > 0 && x >= 0) gfx->draw16bitRGBBitmap(x, y, bmp, dw, h);
  return true;
}

static void luaGfxInit() {
  TJpgDec.setCallback(jpg_out);
  TJpgDec.setJpgScale(1);
}

static String luaResolvePath(const char *p) {
  String s(p);
  if (s.startsWith("/")) return s;
  return g_appDir + s;
}

static int l_drawJpg(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int x = (int)luaL_checkinteger(L, 2), y = (int)luaL_checkinteger(L, 3);
  int scale = (int)luaL_optinteger(L, 4, 1);
  String full = luaResolvePath(path);
  TJpgDec.setJpgScale(scale);
  TJpgDec.drawSdJpg(x, y, full.c_str());
  g_lastYield = millis();
  return 0;
}

static int l_pollEvent(lua_State *L) {
  g_lastYield = millis();
  TouchEvent e;
  if (!tePop(e)) return 0;
  lua_pushinteger(L, e.type);
  lua_pushinteger(L, e.x);
  lua_pushinteger(L, e.y);
  return 3;
}

static bool fsSafePath(const String &base, const char *name, String &out) {
  String n(name);
  if (n.length() == 0 || n.startsWith("/") || n.indexOf("..") >= 0) return false;
  out = base + n;
  return true;
}

static int l_fsRead(lua_State *L) {
  String p;
  if (!fsSafePath(g_dataDir, luaL_checkstring(L, 1), p)) return 0;
  File f = SD.open(p, "r");
  if (!f) return 0;
  String d = f.readString();
  f.close();
  lua_pushstring(L, d.c_str());
  return 1;
}

static int l_fsWrite(lua_State *L) {
  String p;
  if (!fsSafePath(g_dataDir, luaL_checkstring(L, 1), p)) {
    lua_pushboolean(L, 0);
    return 1;
  }
  File f = SD.open(p, FILE_WRITE);
  if (!f) {
    lua_pushboolean(L, 0);
    return 1;
  }
  f.print(luaL_checkstring(L, 2));
  f.close();
  lua_pushboolean(L, 1);
  return 1;
}

static int l_fsAppend(lua_State *L) {
  String p;
  if (!fsSafePath(g_dataDir, luaL_checkstring(L, 1), p)) {
    lua_pushboolean(L, 0);
    return 1;
  }
  File f = SD.open(p, FILE_APPEND);
  if (!f) {
    lua_pushboolean(L, 0);
    return 1;
  }
  f.print(luaL_checkstring(L, 2));
  f.close();
  lua_pushboolean(L, 1);
  return 1;
}

static int l_fsExists(lua_State *L) {
  String p;
  if (!fsSafePath(g_dataDir, luaL_checkstring(L, 1), p)) {
    lua_pushboolean(L, 0);
    return 1;
  }
  lua_pushboolean(L, SD.exists(p));
  return 1;
}

static int l_fsDelete(lua_State *L) {
  String p;
  if (!fsSafePath(g_dataDir, luaL_checkstring(L, 1), p)) {
    lua_pushboolean(L, 0);
    return 1;
  }
  lua_pushboolean(L, SD.remove(p));
  return 1;
}

static int l_fsLs(lua_State *L) {
  String p;
  lua_newtable(L);
  if (!fsSafePath(g_dataDir, luaL_optstring(L, 1, ""), p) || !SD.exists(p)) return 1;
  File dir = SD.open(p);
  if (!dir) return 1;
  int i = 1;
  File e = dir.openNextFile();
  while (e) {
    lua_pushinteger(L, i++);
    lua_pushstring(L, e.name());
    lua_settable(L, -3);
    e.close();
    e = dir.openNextFile();
  }
  dir.close();
  return 1;
}

static int l_sysBacklight(lua_State *L) {
  sysSetBrightness((uint8_t)luaL_checkinteger(L, 1));
  g_lastYield = millis();
  return 0;
}

static int l_sysGetBacklight(lua_State *L) {
  lua_pushinteger(L, g_brightness);
  return 1;
}

static int l_sysSetLed(lua_State *L) {
  int r = std::max(0, std::min(255, (int)luaL_checkinteger(L, 1)));
  int g = std::max(0, std::min(255, (int)luaL_checkinteger(L, 2)));
  int b = std::max(0, std::min(255, (int)luaL_checkinteger(L, 3)));
  sysLed(r, g, b);
  sysSaveLed(true, r, g, b);
  return 0;
}

static int l_sysGetLed(lua_State *L) {
  lua_pushinteger(L, ledCurR);
  lua_pushinteger(L, ledCurG);
  lua_pushinteger(L, ledCurB);
  return 3;
}

static int l_sysLedOff(lua_State *L) {
  sysLedOff();
  sysSaveLed(false, 0, 0, 0);
  return 0;
}

static int l_sysInvert(lua_State *L) {
  sysSetInvert(lua_toboolean(L, 1));
  g_lastYield = millis();
  return 0;
}

static int l_sysGetInvert(lua_State *L) {
  lua_pushboolean(L, g_colorInvert);
  return 1;
}

static int l_sysReboot(lua_State *L) { sysReboot(); return 0; }

static int l_sysSleep(lua_State *L) { sysSleepNow(); return 0; }

static int l_kvSet(lua_State *L) {
  const char *k = luaL_checkstring(L, 1);
  size_t len;
  const char *v = luaL_tolstring(L, 2, &len);
  kvSet(k, String(v));
  lua_pop(L, 1);
  return 0;
}

static int l_kvGet(lua_State *L) {
  const char *k = luaL_checkstring(L, 1);
  if (!kvHas(k)) {
    if (lua_gettop(L) >= 2) {
      lua_pushvalue(L, 2);
      return 1;
    }
    lua_pushnil(L);
    return 1;
  }
  lua_pushstring(L, kvGet(k).c_str());
  return 1;
}

static int l_wifiConnect(lua_State *L) {
  const char *ssid = luaL_checkstring(L, 1);
  const char *pass = luaL_optstring(L, 2, "");
  bool ok = netConnect(ssid, pass, true);
  g_lastYield = millis();
  lua_pushboolean(L, ok);
  return 1;
}

static int l_wifiDisconnect(lua_State *L) {
  netDisconnect(lua_toboolean(L, 1));
  return 0;
}

static int l_wifiStatus(lua_State *L) {
  lua_pushstring(L, netStatusText().c_str());
  return 1;
}

static int l_wifiIp(lua_State *L) {
  lua_pushstring(L, netIp().c_str());
  return 1;
}

static int l_wifiSsid(lua_State *L) {
  lua_pushstring(L, netConnected() ? netSsid.c_str() : "");
  return 1;
}

static int l_wifiScan(lua_State *L) {
  std::vector<String> ssids;
  std::vector<int> rssi;
  std::vector<bool> secured;
  netScan(ssids, rssi, secured);
  g_lastYield = millis();
  lua_newtable(L);
  for (size_t i = 0; i < ssids.size(); i++) {
    lua_createtable(L, 0, 3);
    lua_pushstring(L, ssids[i].c_str());
    lua_setfield(L, -2, "ssid");
    lua_pushinteger(L, rssi[i]);
    lua_setfield(L, -2, "rssi");
    lua_pushboolean(L, secured[i]);
    lua_setfield(L, -2, "lock");
    lua_rawseti(L, -2, (int)i + 1);
  }
  return 1;
}

static int l_netTime(lua_State *L) {
  struct tm t;
  if (!netTime(t)) return 0;
  lua_createtable(L, 0, 7);
  lua_pushinteger(L, t.tm_year + 1900);
  lua_setfield(L, -2, "year");
  lua_pushinteger(L, t.tm_mon + 1);
  lua_setfield(L, -2, "month");
  lua_pushinteger(L, t.tm_mday);
  lua_setfield(L, -2, "day");
  lua_pushinteger(L, t.tm_hour);
  lua_setfield(L, -2, "hour");
  lua_pushinteger(L, t.tm_min);
  lua_setfield(L, -2, "min");
  lua_pushinteger(L, t.tm_sec);
  lua_setfield(L, -2, "sec");
  lua_pushinteger(L, t.tm_wday);
  lua_setfield(L, -2, "wday");
  return 1;
}

static int l_netGet(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  String err;
  String body = netHttpGet(String(url), err);
  g_lastYield = millis();
  if (err.length()) {
    lua_pushnil(L);
    lua_pushstring(L, err.c_str());
    return 2;
  }
  lua_pushstring(L, body.c_str());
  return 1;
}

static int l_kbInput(lua_State *L) {
  const char *title = luaL_optstring(L, 1, "Input");
  bool mask = lua_toboolean(L, 2);
  String out;
  bool ok = kbInput(title, out, mask);
  g_lastYield = millis();
  if (!ok) return 0;
  lua_pushstring(L, out.c_str());
  return 1;
}

static int l_exitApp(lua_State *L) {
  g_exitReq = true;
  return luaL_error(L, LUA_EXIT_MAGIC);
}

static void hookWatchdog(lua_State *L, lua_Debug *ar) {
  if (millis() - g_lastYield > 4000 && ar->event == LUA_HOOKCOUNT) {
    lua_pushfstring(L, "app froze: no delay()/yield for 4s");
    lua_error(L);
  }
}

static void regAll(lua_State *L) {
  lua_pushinteger(L, TE_PRESS);
  lua_setglobal(L, "TE_PRESS");
  lua_pushinteger(L, TE_MOVE);
  lua_setglobal(L, "TE_MOVE");
  lua_pushinteger(L, TE_RELEASE);
  lua_setglobal(L, "TE_RELEASE");
  lua_pushinteger(L, SCREEN_W);
  lua_setglobal(L, "SCREEN_W");
  lua_pushinteger(L, SCREEN_H);
  lua_setglobal(L, "SCREEN_H");
  lua_newtable(L);
#define LC(n, f)              \
  do {                        \
    lua_pushcfunction(L, f);  \
    lua_setfield(L, -2, #n);  \
  } while (0)
  LC(cls, l_cls);
  LC(px, l_px);
  LC(line, l_line);
  LC(rect, l_rect);
  LC(fillRect, l_fillRect);
  LC(circle, l_circle);
  LC(fillCircle, l_fillCircle);
  LC(text, l_text);
  LC(textW, l_textW);
  LC(rgb565, l_rgb565);
  LC(drawJpg, l_drawJpg);
  lua_setglobal(L, "gfx");

  lua_newtable(L);
  static const struct { const char *n; int v; } cols[] = {
    { "BLACK", 0x0000 }, { "WHITE", 0xFFFF }, { "RED", 0xF800 }, { "GREEN", 0x07E0 },
    { "BLUE", 0x001F }, { "YELLOW", 0xFFE0 }, { "CYAN", 0x07FF }, { "MAGENTA", 0xF81F },
    { "ORANGE", 0xFD20 }, { "GRAY", 0x8410 }, { "DGRAY", 0x4208 }, { "LGRAY", 0xC618 },
    { "TEAL", 0x0410 }, { "NAVY", 0x0010 }, { "FACE", 0xC618 },
  };
  for (auto &cc : cols) {
    lua_pushinteger(L, cc.v);
    lua_setglobal(L, cc.n);
    lua_pushinteger(L, cc.v);
    lua_setfield(L, -2, cc.n);
  }
  lua_setglobal(L, "COLOR");

  lua_newtable(L);
  LC(read, l_fsRead);
  LC(write, l_fsWrite);
  LC(append, l_fsAppend);
  LC(exists, l_fsExists);
  LC(delete, l_fsDelete);
  LC(ls, l_fsLs);
  lua_setglobal(L, "fs");

  lua_newtable(L);
  LC(setBacklight, l_sysBacklight);
  LC(getBacklight, l_sysGetBacklight);
  LC(setLed, l_sysSetLed);
  LC(getLed, l_sysGetLed);
  LC(ledOff, l_sysLedOff);
  LC(setColorInvert, l_sysInvert);
  LC(getColorInvert, l_sysGetInvert);
  LC(reboot, l_sysReboot);
  LC(sleep, l_sysSleep);
  LC(set, l_kvSet);
  LC(get, l_kvGet);
  lua_setglobal(L, "sys");

  lua_newtable(L);
  LC(connect, l_wifiConnect);
  LC(disconnect, l_wifiDisconnect);
  LC(status, l_wifiStatus);
  LC(ip, l_wifiIp);
  LC(ssid, l_wifiSsid);
  LC(scan, l_wifiScan);
  lua_setglobal(L, "wifi");

  lua_newtable(L);
  LC(time, l_netTime);
  LC(get, l_netGet);
  lua_setglobal(L, "net");

  lua_newtable(L);
  LC(input, l_kbInput);
  lua_setglobal(L, "kb");

  lua_register(L, "millis", l_millis);
  lua_register(L, "delay", l_delay);
  lua_register(L, "beep", l_beep);
  lua_register(L, "pollEvent", l_pollEvent);
  lua_register(L, "exitApp", l_exitApp);

  lua_pushstring(L, CYDOS_VERSION);
  lua_setglobal(L, "VERSION");
  lua_pushstring(L, g_appName.c_str());
  lua_setglobal(L, "APP_NAME");
  lua_pushstring(L, g_appDir.c_str());
  lua_setglobal(L, "APP_DIR");
  lua_pushstring(L, g_dataDir.c_str());
  lua_setglobal(L, "DATA_DIR");
#undef LC
}

static bool luaIsExitError(lua_State *L) {
  const char *msg = lua_tostring(L, -1);
  return msg && strstr(msg, LUA_EXIT_MAGIC) != nullptr;
}

static void luaDispatchEvent(const TouchEvent &e) {
  if (g_appMode != AM_EVENT || !g_lua || !g_hasTouch) return;
  g_lastYield = millis();
  lua_getglobal(g_lua, "onTouch");
  if (!lua_isfunction(g_lua, -1)) {
    lua_pop(g_lua, 1);
    g_hasTouch = false;
    return;
  }
  lua_pushinteger(g_lua, e.type);
  lua_pushinteger(g_lua, e.x);
  lua_pushinteger(g_lua, e.y);
  bool failed = (lua_pcall(g_lua, 3, 0, 0) != 0);
  if (failed) {
    if (!luaIsExitError(g_lua)) {
      const char *msg = lua_tostring(g_lua, -1);
      shellShowAppError(g_appName, msg ? msg : "onTouch error");
      g_exitReq = true;
    }
    lua_pop(g_lua, 1);
  }
}

static void luaCallLoop(uint32_t dt) {
  if (g_appMode != AM_EVENT || !g_lua || !g_hasLoop || g_exitReq) return;
  g_lastYield = millis();
  lua_getglobal(g_lua, "loop");
  if (!lua_isfunction(g_lua, -1)) {
    lua_pop(g_lua, 1);
    g_hasLoop = false;
    return;
  }
  lua_pushinteger(g_lua, dt);
  if (lua_pcall(g_lua, 1, 0, 0) != 0) {
    if (!luaIsExitError(g_lua)) {
      const char *msg = lua_tostring(g_lua, -1);
      shellShowAppError(g_appName, msg ? msg : "loop error");
      g_exitReq = true;
    }
    lua_pop(g_lua, 1);
  }
}

static void stopLuaApp() {
  if (g_lua) {
    lua_close(g_lua);
    g_lua = nullptr;
  }
  g_appMode = AM_NONE;
  g_hasLoop = g_hasTouch = false;
  g_exitReq = false;
  noTone(PIN_BUZZER);
}

static int luaPanic(lua_State *L) {
  const char *msg = lua_tostring(L, -1);
  Serial.printf("LUA PANIC: %s\n", msg ? msg : "(non-string error)");
  luaL_traceback(L, L, msg, 0);
  const char *tb = lua_tostring(L, -1);
  Serial.println(tb ? tb : "(no traceback)");
  Serial.flush();
  return 0;
}

static bool startLuaApp(const String &dir, const String &displayName) {
  String base = dir.endsWith("/") ? dir : dir + "/";
  String scriptPath = base + "app.lua";
  File f = SD.open(scriptPath, "r");
  if (!f) return false;

  g_appDir = base;
  g_appName = displayName;
  g_dataDir = "/data/" + displayName + "/";
  SD.mkdir("/data");
  SD.mkdir(g_dataDir);
  g_exitReq = false;
  g_lastYield = millis();

  lua_State *L = luaL_newstate();
  if (!L) {
    f.close();
    return false;
  }
  g_lua = L;
  lua_atpanic(L, luaPanic);
  lua_pushboolean(L, 1);
  cydosOpenLibs(L);
  regAll(L);
  lua_sethook(L, hookWatchdog, LUA_MASKCOUNT, 100000);

  int lr = lua_load(L, luaFileReader, &f, "@app.lua", NULL);
  f.close();
  if (lr == 0) lr = lua_pcall(L, 0, 0, 0);

  if (lr != 0) {
    if (g_exitReq || luaIsExitError(L)) {
      stopLuaApp();
      return true;
    }
    const char *msg = lua_tostring(L, -1);
    String emsg = msg ? String(msg) : "unknown error";
    stopLuaApp();
    shellShowAppError(displayName, emsg);
    return true;
  }

  lua_getglobal(L, "loop");
  g_hasLoop = lua_isfunction(L, -1);
  lua_pop(L, 1);
  lua_getglobal(L, "onTouch");
  g_hasTouch = lua_isfunction(L, -1);
  lua_pop(L, 1);

  if (g_hasLoop || g_hasTouch) {
    g_appMode = AM_EVENT;
    if (g_hasLoop) {
      lua_getglobal(L, "init");
      if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
          if (!luaIsExitError(L)) {
            const char *msg = lua_tostring(L, -1);
            shellShowAppError(displayName, msg ? msg : "init error");
          }
          stopLuaApp();
          return true;
        }
      } else lua_pop(L, 1);
    }
    return true;
  }

  stopLuaApp();
  return true;
}
