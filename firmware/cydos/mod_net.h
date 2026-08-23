#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include "mod_kv.h"
#include "mod_ui.h"

enum NetState : uint8_t { NET_OFF, NET_CONNECTING, NET_ON, NET_ERROR };
static NetState netState = NET_OFF;
static String netSsid = "", netPass = "";
static uint32_t netTimer = 0;
static bool ntpReady = false;
static int tzHours = 3;

void netApplySavedConfig() {
  tzHours = kvGetInt("tz", 3);
}

static void netStartConnect() {
  if (netSsid.length() == 0) {
    netState = NET_OFF;
    return;
  }
  netState = NET_CONNECTING;
  netTimer = millis();
  ntpReady = false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(netSsid.c_str(), netPass.c_str());
}

void netBegin() {
  netApplySavedConfig();
  netSsid = kvGet("wifi_ssid");
  netPass = kvGet("wifi_pass");
  if (kvGetBool("wifi_on", true) && netSsid.length()) {
    netStartConnect();
  } else {
    WiFi.mode(WIFI_OFF);
  }
}

void netTick() {
  if (netState == NET_CONNECTING && (WiFi.status() == WL_CONNECTED || millis() - netTimer > 20000)) {
    if (WiFi.status() == WL_CONNECTED) {
      netState = NET_ON;
      configTime(tzHours * 3600, 0, "pool.ntp.org", "time.nist.gov");
    } else {
      netState = NET_ERROR;
      netTimer = millis();
    }
  } else if (netState == NET_ERROR && millis() - netTimer > 30000) {
    netStartConnect();
  } else if (netState == NET_ON && WiFi.status() != WL_CONNECTED) {
    netState = NET_CONNECTING;
    netTimer = millis();
    ntpReady = false;
  }
  if ((netState == NET_ON || netState == NET_CONNECTING) && !ntpReady) {
    struct tm t;
    if (getLocalTime(&t, 50)) ntpReady = true;
  }
}

bool netConnected() { return netState == NET_ON; }

String netStatusText() {
  switch (netState) {
    case NET_ON: return "connected";
    case NET_CONNECTING: return "connecting...";
    case NET_ERROR: return "failed";
    default: return "off";
  }
}

bool netScan(std::vector<String> &ssids, std::vector<int> &rssi, std::vector<bool> &secured) {
  gfx->fillRect(0, 0, SCREEN_W, SCREEN_H, C_DESKTOP);
  centerText(100, "Scanning Wi-Fi...", C_WHITE, 2);
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  ssids.clear();
  rssi.clear();
  secured.clear();
  for (int i = 0; i < n; i++) {
    ssids.push_back(WiFi.SSID(i));
    rssi.push_back(WiFi.RSSI(i));
    secured.push_back(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  WiFi.scanDelete();
  if (netSsid.length() && !netConnected()) netStartConnect();
  return n > 0;
}

bool netConnect(const char *ssid, const char *pass, bool save) {
  netSsid = ssid;
  netPass = pass;
  if (save) {
    kvSet("wifi_ssid", netSsid);
    kvSet("wifi_pass", netPass);
    kvSetBool("wifi_on", true);
  }
  netStartConnect();
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) vTaskDelay(50);
  return WiFi.status() == WL_CONNECTED;
}

void netDisconnect(bool forget) {
  kvSetBool("wifi_on", false);
  if (forget) {
    kvSet("wifi_ssid", "");
    kvSet("wifi_pass", "");
    netSsid = "";
    netPass = "";
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  netState = NET_OFF;
  ntpReady = false;
}

bool netReconnectSaved() {
  if (!netSsid.length()) return false;
  kvSetBool("wifi_on", true);
  netStartConnect();
  return true;
}

String netIp() {
  return netConnected() ? WiFi.localIP().toString() : String("-");
}

String netSsidName() { return netSsid; }

bool netTime(struct tm &t) {
  return ntpReady && getLocalTime(&t, 10);
}

String netHttpGet(const String &url, String &err) {
  err = "";
  if (!netConnected()) {
    err = "no wifi";
    return "";
  }
  HTTPClient http;
  String body = "";
  if (url.startsWith("https://")) {
    WiFiClientSecure sec;
    sec.setInsecure();
    sec.setTimeout(10000);
    if (!http.begin(sec, url)) {
      err = "bad url";
      return "";
    }
  } else {
    if (!http.begin(url)) {
      err = "bad url";
      return "";
    }
  }
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int code = http.GET();
  if (code == 200) {
    body = http.getString();
  } else {
    err = "http " + String(code);
  }
  http.end();
  return body;
}
