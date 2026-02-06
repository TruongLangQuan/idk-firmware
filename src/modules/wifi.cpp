#include "modules/wifi.h"
#include "core/ui.h"
#include "core/input.h"

void scanWifi(){
  wifiCount = 0;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  int n = WiFi.scanNetworks();
  if (n < 0) return;
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
    M5.Display.print(WiFi.localIP());
  } else {
    M5.Display.setTextColor(RED);
    M5.Display.print("Failed");
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
