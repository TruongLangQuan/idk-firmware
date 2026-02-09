#include "modules/network/wifi.h"
#include "core/ui.h"
#include "core/input.h"
#include "modules/network/webui.h"
#include "modules/features/clock.h"
#include "core/log.h"
#include <Preferences.h>

static Preferences wifiPrefs;
static const char* WIFI_NS = "wifi";
static const char* WIFI_KEY_SSID = "ssid";
static const char* WIFI_KEY_PASS = "pass";
static const char* WIFI_KEY_KNOWN = "known"; // count of known networks

static int getKnownCount(){
  wifiPrefs.begin(WIFI_NS, true);
  int c = wifiPrefs.getInt(WIFI_KEY_KNOWN, 0);
  wifiPrefs.end();
  return c;
}

static String knownKeyS(int idx){
  char buf[16]; snprintf(buf, sizeof(buf), "s%d", idx); return String(buf);
}
static String knownKeyP(int idx){
  char buf[16]; snprintf(buf, sizeof(buf), "p%d", idx); return String(buf);
}

static void addKnownNetwork(const String &ssid, const String &pass){
  wifiPrefs.begin(WIFI_NS, false);
  int c = wifiPrefs.getInt(WIFI_KEY_KNOWN, 0);
  // check existing
  for (int i=0;i<c;i++){
    String s = wifiPrefs.getString(knownKeyS(i).c_str(), "");
    if (s.length() && s == ssid){
      wifiPrefs.putString(knownKeyP(i).c_str(), pass);
      wifiPrefs.putString(WIFI_KEY_SSID, ssid);
      wifiPrefs.putString(WIFI_KEY_PASS, pass);
      wifiPrefs.end();
      return;
    }
  }
  // append if space (limit to MAX_WIFI)
  if (c < MAX_WIFI){
    wifiPrefs.putString(knownKeyS(c).c_str(), ssid);
    wifiPrefs.putString(knownKeyP(c).c_str(), pass);
    wifiPrefs.putInt(WIFI_KEY_KNOWN, c+1);
  }
  // also keep last-used keys
  wifiPrefs.putString(WIFI_KEY_SSID, ssid);
  wifiPrefs.putString(WIFI_KEY_PASS, pass);
  wifiPrefs.end();
}

void scanWifi(){
  wifiCount = 0;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  int n = WiFi.scanNetworks();
  if (n < 0) { LOGW("WiFi scan failed (n=%d)", n); return; }
  for (int i=0;i<n && wifiCount<MAX_WIFI;i++){
    wifiSSID[wifiCount] = WiFi.SSID(i);
    wifiRSSI[wifiCount] = WiFi.RSSI(i);
    wifiSecured[wifiCount] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    wifiCount++;
  }
}

void wifiConnectTo(int idx){
  if (idx < 0 || idx >= wifiCount) return;
  String ssid = wifiSSID[idx];
  String pass = "";
  if (wifiSecured[idx]){
    bool ok = textInput("WiFi Password", pass, 32, true);
    if (!ok) return;
  }

  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 20);
  M5.Display.print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    delay(100);
  }

  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setCursor(6, STATUS_H + 20);
  if (WiFi.status() == WL_CONNECTED){
    M5.Display.setTextColor(GREEN);
    M5.Display.print("Connected");
    M5.Display.setCursor(6, STATUS_H + 36);
    M5.Display.setTextColor(WHITE);
    IPAddress ip = WiFi.localIP();
    M5.Display.print(ip);
    LOGI("WiFi connected %s", ip.toString().c_str());
    addKnownNetwork(ssid, pass);
    setupTime();
    uint32_t t0 = millis();
    while (!timeValid() && millis() - t0 < 10000){
      delay(50);
    }
    if (webuiPromptOpen()){
      webuiStartSTA();
      webuiShowInfo("WebUI Started");
    }
  } else {
    M5.Display.setTextColor(RED);
    M5.Display.print("Failed");
    LOGE("WiFi connect failed to %s", ssid.c_str());
  }
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("Prev=Back");

  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    delay(10);
  }
}

void wifiAutoConnect(){
  // Try to auto-connect by scanning for known networks saved in Preferences.
  int known = getKnownCount();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  int n = WiFi.scanNetworks();
  if (n < 0) return;
  // scan results: try to match any known SSID (exact or prefix if stored ssid ends with '*')
  wifiPrefs.begin(WIFI_NS, true);
  for (int i=0;i<n;i++){
    String found = WiFi.SSID(i);
    for (int k=0;k<known;k++){
      String stored = wifiPrefs.getString(knownKeyS(k).c_str(), "");
      String pass = wifiPrefs.getString(knownKeyP(k).c_str(), "");
      if (stored.length() == 0) continue;
      bool match = false;
      if (stored.endsWith("*")){
        String prefix = stored.substring(0, stored.length()-1);
        if (found.startsWith(prefix)) match = true;
      } else {
        if (found == stored) match = true;
      }
      if (match){
        wifiPrefs.end();
        WiFi.begin(stored.c_str(), pass.c_str());
        uint32_t start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 6000){ delay(50); }
        if (WiFi.status() == WL_CONNECTED){
          setupTime();
          uint32_t t0 = millis();
          while (!timeValid() && millis() - t0 < 10000){ delay(50); }
          return;
        }
        // else continue trying other known entries
      }
    }
  }
  wifiPrefs.end();
  // fallback: legacy single-entry keys
  wifiPrefs.begin(WIFI_NS, true);
  String ssid = wifiPrefs.getString(WIFI_KEY_SSID, "");
  String pass = wifiPrefs.getString(WIFI_KEY_PASS, "");
  wifiPrefs.end();
  if (ssid.length() == 0) return;
  WiFi.begin(ssid.c_str(), pass.c_str());
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 6000){ delay(50); }
  if (WiFi.status() == WL_CONNECTED){
    setupTime();
    uint32_t t0 = millis();
    while (!timeValid() && millis() - t0 < 10000){ delay(50); }
  }
}

// Exposed helpers for UI
int wifiGetKnownCount(){
  return getKnownCount();
}

String wifiGetKnownSSID(int idx){
  wifiPrefs.begin(WIFI_NS, true);
  int c = wifiPrefs.getInt(WIFI_KEY_KNOWN, 0);
  String out = "";
  if (idx >= 0 && idx < c){
    out = wifiPrefs.getString(knownKeyS(idx).c_str(), "");
  }
  wifiPrefs.end();
  return out;
}

void wifiForgetKnown(int idx){
  wifiPrefs.begin(WIFI_NS, false);
  int c = wifiPrefs.getInt(WIFI_KEY_KNOWN, 0);
  if (idx < 0 || idx >= c){ wifiPrefs.end(); return; }
  // remove by shifting later entries down
  for (int i=idx;i<c-1;i++){
    String s = wifiPrefs.getString(knownKeyS(i+1).c_str(), "");
    String p = wifiPrefs.getString(knownKeyP(i+1).c_str(), "");
    wifiPrefs.putString(knownKeyS(i).c_str(), s);
    wifiPrefs.putString(knownKeyP(i).c_str(), p);
  }
  // remove last
  wifiPrefs.remove(knownKeyS(c-1).c_str());
  wifiPrefs.remove(knownKeyP(c-1).c_str());
  wifiPrefs.putInt(WIFI_KEY_KNOWN, c-1);
  // adjust last-used keys if they matched removed SSID
  String last = wifiPrefs.getString(WIFI_KEY_SSID, "");
  if (last.length() == 0){
    if (c-1 > 0){
      String first = wifiPrefs.getString(knownKeyS(0).c_str(), "");
      String pass = wifiPrefs.getString(knownKeyP(0).c_str(), "");
      wifiPrefs.putString(WIFI_KEY_SSID, first);
      wifiPrefs.putString(WIFI_KEY_PASS, pass);
    } else {
      wifiPrefs.remove(WIFI_KEY_SSID);
      wifiPrefs.remove(WIFI_KEY_PASS);
    }
  }
  wifiPrefs.end();
}
