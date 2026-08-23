#pragma once
#include <SD.h>
#include "mod_config.h"
#include "mod_kv.h"

static const char *SETTINGS_PATH = "/data/Settings/settings.json";

static String sjGetVal(const String &doc, const char *key, const String &def) {
  int k = doc.indexOf("\"" + String(key) + "\"");
  if (k < 0) return def;
  k = doc.indexOf(':', k + (int)strlen(key) + 1);
  if (k < 0) return def;
  k++;
  while (k < (int)doc.length() && isspace(doc[k])) k++;
  if (k >= (int)doc.length()) return def;
  if (doc[k] == '"') {
    int e = doc.indexOf('"', k + 1);
    return e < 0 ? def : doc.substring(k + 1, e);
  }
  int e = k;
  while (e < (int)doc.length() && strchr("}],\n\r", doc[e]) == nullptr) e++;
  String v = doc.substring(k, e);
  v.trim();
  return v.length() ? v : def;
}

static void sjSetValue(String &doc, const char *key, const String &encVal) {
  String pat = "\"" + String(key) + "\":";
  int k = doc.indexOf(pat);
  if (k < 0) {
    doc += doc.length() ? ",\n  " : "  ";
    doc += pat + " " + encVal;
    return;
  }
  int cs = doc.indexOf(':', k) + 1;
  int ce = cs;
  while (ce < (int)doc.length() && doc[ce] != ',' && doc[ce] != '}') ce++;
  doc = doc.substring(0, cs) + " " + encVal + doc.substring(ce);
}

static void settingsBuildDoc(String &d) {
  d = "";
  sjSetValue(d, "led_on", kvGetBool("ledon", true) ? "true" : "false");
  sjSetValue(d, "led_r", String(kvGetInt("ledr", 255)));
  sjSetValue(d, "led_g", String(kvGetInt("ledg", 255)));
  sjSetValue(d, "led_b", String(kvGetInt("ledb", 255)));
  sjSetValue(d, "brightness", String(kvGetInt("bl", 200)));
  sjSetValue(d, "invert", kvGetBool("inv", false) ? "true" : "false");
  String ssid = kvGet("wifi_ssid", "");
  if (ssid.length()) sjSetValue(d, "wifi_ssid", "\"" + ssid + "\"");
  d = "{\n" + d + "\n}";
}

static bool g_setDirty = true;

void settingsNoteChange() { g_setDirty = true; }

static void settingsSyncNow() {
  SD.mkdir("/data");
  SD.mkdir("/data/Settings");
  String d;
  settingsBuildDoc(d);
  File f = SD.open(SETTINGS_PATH, FILE_WRITE);
  if (!f) return;
  f.print(d);
  f.close();
  g_setDirty = false;
}

void settingsMaybeSync() {
  static uint32_t lastTry = 0;
  if (!g_setDirty || millis() - lastTry < 3000) return;
  lastTry = millis();
  settingsSyncNow();
}

static void settingsLoadApply() {
  if (!SD.exists(SETTINGS_PATH)) return;
  File f = SD.open(SETTINGS_PATH, "r");
  if (!f) return;
  String doc = f.readString();
  f.close();
  if (doc.length() < 2) return;
  String v;
  v = sjGetVal(doc, "brightness", "");
  if (v.length()) kvSetInt("bl", constrain(v.toInt(), 5, 255));
  v = sjGetVal(doc, "invert", "");
  if (v.length()) kvSetBool("inv", v == "true" || v == "1");
  v = sjGetVal(doc, "led_on", "");
  if (v.length()) kvSetBool("ledon", v == "true" || v == "1");
  v = sjGetVal(doc, "led_r", "");
  if (v.length()) kvSetInt("ledr", constrain(v.toInt(), 0, 255));
  v = sjGetVal(doc, "led_g", "");
  if (v.length()) kvSetInt("ledg", constrain(v.toInt(), 0, 255));
  v = sjGetVal(doc, "led_b", "");
  if (v.length()) kvSetInt("ledb", constrain(v.toInt(), 0, 255));
  v = sjGetVal(doc, "wifi_ssid", "");
  if (v.length()) kvSet("wifi_ssid", v);
  v = sjGetVal(doc, "wifi_pass", "");
  if (v.length()) {
    kvSet("wifi_pass", v);
    g_setDirty = true;
  }
}
