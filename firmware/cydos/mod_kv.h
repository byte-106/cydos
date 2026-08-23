#pragma once
#include <LittleFS.h>
#include <vector>
#include <WString.h>

struct KvPair {
  String k;
  String v;
};
static std::vector<KvPair> kvStore;

void kvLoadAll() {
  kvStore.clear();
  File f = LittleFS.open("/kv.txt", "r");
  if (!f) return;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    int eq = line.indexOf('=');
    if (eq > 0) kvStore.push_back({ line.substring(0, eq), line.substring(eq + 1) });
  }
  f.close();
}

void kvCommit() {
  File f = LittleFS.open("/kv.txt", "w");
  if (!f) return;
  for (auto &p : kvStore) f.print(p.k + "=" + p.v + "\n");
  f.close();
}

bool kvHas(const char *key) {
  for (auto &p : kvStore)
    if (p.k == key) return true;
  return false;
}

String kvGet(const char *key, const char *def = "") {
  for (auto &p : kvStore)
    if (p.k == key) return p.v;
  return String(def);
}

void kvSet(const char *key, const String &val) {
  for (auto &p : kvStore) {
    if (p.k == key) {
      if (p.v == val) return;
      p.v = val;
      kvCommit();
      return;
    }
  }
  kvStore.push_back({ String(key), val });
  kvCommit();
}

int kvGetInt(const char *key, int def) {
  if (!kvHas(key)) return def;
  return kvGet(key).toInt();
}

void kvSetInt(const char *key, int v) { kvSet(key, String(v)); }

bool kvGetBool(const char *key, bool def) {
  if (!kvHas(key)) return def;
  return kvGet(key) == "1";
}

void kvSetBool(const char *key, bool v) { kvSet(key, v ? "1" : "0"); }
