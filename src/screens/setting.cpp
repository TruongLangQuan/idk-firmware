#include "screens/setting.h"
#include "core/ui.h"
#include "core/input.h"
#include "system/power.h"
#include "modules/webui.h"
#include "modules/wifi.h"
#include "modules/backup.h"
#include "modules/files.h"
#include "modules/config.h"
#include "modules/ir.h"

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
    else if (setIndex == 8){
      int kc = wifiGetKnownCount();
      if (kc == 0){
        M5.Display.fillScreen(BLACK); drawStatus();
        M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(WHITE);
        M5.Display.print("No known networks");
        M5.Display.setTextColor(COLOR_DIM);
        M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        drawSetting();
        return;
      }
      int sel = 0;
      while (true){
        kc = wifiGetKnownCount();
        if (sel >= kc) sel = kc-1;
        M5.Display.fillScreen(BLACK); drawStatus();
        M5.Display.setTextColor(WHITE);
        M5.Display.setCursor(6, STATUS_H + 6);
        M5.Display.print("Known Networks:");
        int show = min(6, kc);
        for (int i=0;i<show;i++){
          int idx = i;
          int y = STATUS_H + 26 + i*18;
          String s = wifiGetKnownSSID(idx);
          if (idx == sel) M5.Display.setTextColor(GREEN); else M5.Display.setTextColor(WHITE);
          M5.Display.setCursor(6, y);
          M5.Display.print(String(idx) + ": " + s);
        }
        M5.Display.setTextColor(COLOR_DIM);
        M5.Display.setCursor(6, SCREEN_H-10);
        M5.Display.print("A=Forget  B=Next  Prev=Back");
        // wait for input
        while (true){ M5.update(); if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) break; delay(10); }
        if (M5.BtnPWR.wasPressed()) break;
        if (M5.BtnB.wasPressed()){
          sel = (sel + 1) % kc;
          continue;
        }
        if (M5.BtnA.wasPressed()){
          String s = wifiGetKnownSSID(sel);
          // confirmation
          M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(WHITE);
          M5.Display.print("Forget "); M5.Display.print(s);
          M5.Display.setCursor(6, STATUS_H + 40); M5.Display.print("M5=Yes  Prev=No");
          bool confirmed = false;
          while (true){ M5.update(); if (M5.BtnA.wasPressed()){ confirmed = true; break; } if (M5.BtnPWR.wasPressed()) break; delay(10); }
          if (confirmed){ wifiForgetKnown(sel); kc = wifiGetKnownCount(); if (kc == 0) break; if (sel >= kc) sel = kc-1; }
        }
      }
      drawSetting();
    }
    else if (setIndex == 9){
      // Backup to SPIFFS and SD (if available)
      M5.Display.fillScreen(BLACK); drawStatus();
      M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(WHITE);
      M5.Display.print("Saving backup...");
      bool okFs = backupSaveToFile("/idk-backup.json");
      bool okSd = false;
      if (!sdReady) sdReady = SD.begin(configGetSdCsPin());
      if (sdReady) okSd = backupSaveToSd("/idk-backup.json");
      M5.Display.fillScreen(BLACK); drawStatus();
      M5.Display.setCursor(6, STATUS_H + 20);
      if (okFs && okSd){ M5.Display.setTextColor(GREEN); M5.Display.print("Backup saved (FS+SD)"); }
      else if (okFs){ M5.Display.setTextColor(GREEN); M5.Display.print("Backup saved (FS)"); M5.Display.setCursor(6, STATUS_H + 36); M5.Display.setTextColor(COLOR_DIM); M5.Display.print(sdReady?"SD save failed":"SD not available"); }
      else if (okSd){ M5.Display.setTextColor(GREEN); M5.Display.print("Backup saved (SD)"); M5.Display.setCursor(6, STATUS_H + 36); M5.Display.setTextColor(COLOR_DIM); M5.Display.print("FS save failed"); }
      else { M5.Display.setTextColor(RED); M5.Display.print("Backup failed"); }
      M5.Display.setTextColor(COLOR_DIM); M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawSetting();
    }
    else if (setIndex == 10){
      // Restore from SPIFFS
      M5.Display.fillScreen(BLACK); drawStatus();
      M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(WHITE);
      M5.Display.print("Restoring from /idk-backup.json...");
      bool ok = backupLoadFromFile("/idk-backup.json");
      M5.Display.fillScreen(BLACK); drawStatus();
      M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(ok?GREEN:RED);
      M5.Display.print(ok?"Restore applied":"Restore failed");
      M5.Display.setTextColor(COLOR_DIM); M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawSetting();
    }
    else if (setIndex == 11){
      // Backup to SD: enter SD file browser to choose path
      fileSelectMode = 1; // backup save
      filesSource = FS_SD;
      if (!sdReady) sdReady = SD.begin(configGetSdCsPin());
      if (!sdReady){
        M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(RED);
        M5.Display.print("SD not mounted"); M5.Display.setTextColor(COLOR_DIM); M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        fileSelectMode = 0; drawSetting();
      } else {
        buildBrowserList(); browserIndex = 0; screen = SCR_FILES_BROWSER; drawFilesBrowser();
      }
      return;
    }
    else if (setIndex == 12){
      // Restore from SD: enter SD file browser to choose path
      fileSelectMode = 2; // backup load
      filesSource = FS_SD;
      if (!sdReady) sdReady = SD.begin(configGetSdCsPin());
      if (!sdReady){
        M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(RED);
        M5.Display.print("SD not mounted"); M5.Display.setTextColor(COLOR_DIM); M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        fileSelectMode = 0; drawSetting();
      } else {
        buildBrowserList(); browserIndex = 0; screen = SCR_FILES_BROWSER; drawFilesBrowser();
      }
      return;
    }
    else if (setIndex == 6){
      String s = String(configGetIrPin());
      if (textInput("IR Pin", s, 4, false)){
        int v = s.toInt();
        if (v >= 0 && v <= 40) {
          configSetIrPin((uint8_t)v);
          bool ok = probeIrPin((uint8_t)v);
          M5.Display.fillScreen(BLACK);
          drawStatus();
          M5.Display.setCursor(6, STATUS_H + 20);
          M5.Display.setTextColor(ok ? GREEN : RED);
          M5.Display.print(ok ? "IR init OK" : "IR init Failed");
          M5.Display.setTextColor(COLOR_DIM);
          M5.Display.setCursor(6, SCREEN_H-10);
          M5.Display.print("Prev=Back");
          while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        }
      }
      drawSetting();
    } else if (setIndex == 7){
      String s = String(configGetSdCsPin());
      if (textInput("SD CS Pin", s, 4, false)){
        int v = s.toInt();
        if (v >= 0 && v <= 40) {
          configSetSdCsPin((uint8_t)v);
          // try mounting immediately to validate
          bool ok = SD.begin(configGetSdCsPin());
          sdReady = ok;
          M5.Display.fillScreen(BLACK);
          drawStatus();
          M5.Display.setCursor(6, STATUS_H + 20);
          M5.Display.setTextColor(ok ? GREEN : RED);
          M5.Display.print(ok ? "SD mount OK" : "SD mount Failed");
          M5.Display.setTextColor(COLOR_DIM);
          M5.Display.setCursor(6, SCREEN_H-10);
          M5.Display.print("Prev=Back");
          while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        }
      }
      drawSetting();
    }
  }
}
