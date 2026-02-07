#include "screens/setting.h"
#include "core/ui.h"
#include "system/power.h"
#include "modules/webui.h"

void screenSettingUpdate(){
  if (M5.BtnB.wasPressed()) { setIndex = (setIndex + 1) % SET_COUNT; drawSetting(); }
  if (M5.BtnPWR.wasPressed()) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (setIndex == 0){
      dimIndex = (dimIndex + 1) % 5;
      lastActivity = millis();
      isDimmed = false;
      setBacklight(BRIGHT_NORMAL);
      drawSetting();
    } else if (setIndex == 1){
      if (!webuiEnabled){
        webuiShowInfo("WebUI Disabled");
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        drawSetting();
        return;
      }
      if (WiFi.status() == WL_CONNECTED) webuiStartSTA();
      else webuiStartAP();
      webuiShowInfo("WebUI Started");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawSetting();
    } else if (setIndex == 2){
      webuiEnabled = !webuiEnabled;
      webuiPersistEnabled(webuiEnabled);
      if (!webuiEnabled) webuiStop();
      drawSetting();
    } else if (setIndex == 3){
      M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6,50); M5.Display.print("Restarting...");
      delay(300); ESP.restart();
    } else if (setIndex == 4){
      M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6,50); M5.Display.print("Powering off...");
      delay(200); M5.Power.powerOff();
    } else if (setIndex == 5){
      M5.Display.fillScreen(BLACK);
      drawStatus();
      M5.Display.setCursor(6, STATUS_H + 12);
      M5.Display.setTextColor(WHITE);
      M5.Display.printf("Chip: ESP32\n");
      M5.Display.printf("CPU: %d MHz\n", ESP.getCpuFreqMHz());
      M5.Display.printf("Flash: %d MB\n", ESP.getFlashChipSize()/1024/1024);
      M5.Display.printf("Heap: %d KB\n", ESP.getFreeHeap()/1024);
      size_t total = SPIFFS.totalBytes();
      size_t used  = SPIFFS.usedBytes();
      M5.Display.printf("SPIFFS: %d/%d KB\n", used/1024, total/1024);
      M5.Display.setCursor(6, SCREEN_H-10);
      M5.Display.setTextColor(COLOR_DIM);
      M5.Display.print("Prev=Back");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawSetting();
    }
  }
}
