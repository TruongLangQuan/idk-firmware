#include "screens/files.h"
#include "core/ui.h"
#include "core/input.h"
#include "modules/files.h"
#include "modules/media.h"
#include "modules/webui.h"
#include "modules/txt.h"
#include "modules/config.h"
#include "modules/backup.h"

void screenFilesListUpdate(){
  if (M5.BtnB.wasPressed()) { filesSource = (FileSource)((filesSource + 1) % 3); drawFilesList(); }
  if (M5.BtnPWR.wasPressed()) { filesSource = (FileSource)((filesSource - 1 + 3) % 3); drawFilesList(); }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (filesSource == FS_WEBUI){
      if (WiFi.status() == WL_CONNECTED) webuiStartSTA();
      else webuiStartAP();
      webuiShowInfo("WebUI Started");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawFilesList();
    } else {
      if (filesSource == FS_SD && !sdReady){
        sdReady = SD.begin(configGetSdCsPin());
      }
      if (filesSource == FS_SD && !sdReady){
        M5.Display.fillScreen(BLACK);
        drawStatus();
        M5.Display.setTextColor(RED);
        M5.Display.setCursor(6, STATUS_H + 20);
        M5.Display.print("SD not mounted");
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
        drawFilesList();
        return;
      }
      buildBrowserList();
      browserIndex = 0;
      screen = SCR_FILES_BROWSER;
      drawFilesBrowser();
    }
  }
}

void screenFilesBrowserUpdate(){
  if (M5.BtnB.wasPressed()) { if (browserCount>0) { browserIndex = (browserIndex + 1) % browserCount; drawFilesBrowser(); } }
  if (M5.BtnPWR.wasPressed()) { if (browserCount>0) { browserIndex = (browserIndex - 1 + browserCount) % browserCount; drawFilesBrowser(); } }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_FILES_LIST; drawFilesList(); }
  if (M5.BtnA.pressedFor(800) && browserCount>0){
    String name = browserFiles[browserIndex];
    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 20);
    M5.Display.print("Delete?");
    M5.Display.setCursor(6, STATUS_H + 36);
    M5.Display.print(name);
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.print("M5=Yes  Prev=No");
    while (true){
      M5.update();
      if (M5.BtnA.wasPressed()){
        fs::FS *fs = nullptr;
        if (filesSource == FS_SD && sdReady) fs = &SD;
        else if (filesSource == FS_LITTLEFS) fs = &SPIFFS;
        if (fs) deletePathFS(*fs, name);
        buildBrowserList();
        if (browserIndex >= browserCount) browserIndex = browserCount-1;
        drawFilesBrowser();
        return;
      }
      if (M5.BtnPWR.wasPressed()){
        drawFilesBrowser();
        return;
      }
      delay(10);
    }
  }
  if (M5.BtnA.wasPressed() && browserCount>0){
    String name = browserFiles[browserIndex];
    if (fileSelectMode != 0){
      // selection mode: if directory, ask for filename; otherwise use path
      fs::FS *fs = nullptr;
      if (filesSource == FS_SD && sdReady) fs = &SD;
      else if (filesSource == FS_LITTLEFS) fs = &SPIFFS;
      if (!fs){ fileSelectMode = 0; screen = SCR_SETTING; drawSetting(); return; }
      File f = fs->open(name);
      String target = name;
      if (f && f.isDirectory()){
        f.close();
        String fname = "idk-backup.json";
        if (!textInput("Filename", fname, 64, false)){
          fileSelectMode = 0; screen = SCR_SETTING; drawSetting(); return;
        }
        if (!fname.startsWith("/")) target = String(name) + "/" + fname; else target = fname;
      } else if (f){ f.close(); }
      bool ok = false;
      if (fileSelectMode == 1){
        ok = backupSaveToSd(target);
      } else if (fileSelectMode == 2){
        ok = backupLoadFromSd(target);
      }
      fileSelectMode = 0;
      M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6, STATUS_H + 20); M5.Display.setTextColor(ok?GREEN:RED);
      M5.Display.print(ok?"Operation OK":"Operation Failed");
      M5.Display.setTextColor(COLOR_DIM); M5.Display.setCursor(6, SCREEN_H-10); M5.Display.print("Prev=Back");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10);} 
      screen = SCR_SETTING; drawSetting();
      return;
    }
    if (filesSource == FS_LITTLEFS){
    String lname = name;
    lname.toLowerCase();
    if (lname.endsWith(".png")) showPNG(name.c_str());
    else if (lname.endsWith(".gif")) playGIFLoop(name.c_str());
    else if (lname.endsWith(".jpg") || lname.endsWith(".jpeg")) showJPG(name.c_str());
    else if (lname.endsWith(".txt")) showTXT(SPIFFS, name.c_str());
      else {
        M5.Display.fillScreen(BLACK);
        drawStatus();
        M5.Display.setTextColor(WHITE);
        M5.Display.setCursor(6, STATUS_H + 20);
        M5.Display.print(name);
      }
    } else if (filesSource == FS_SD && sdReady){
      String lname = name;
      lname.toLowerCase();
      if (lname.endsWith(".txt")) showTXT(SD, name.c_str());
      else if (lname.endsWith(".jpg") || lname.endsWith(".jpeg")) {
        // draw from SD explicitly by opening the file and using drawJpg(file)
        M5.Display.fillScreen(BLACK);
        drawStatus();
        int y = STATUS_H; int h = SCREEN_H - STATUS_H;
        File f = SD.open(name);
        if (f) {
          if (!M5.Display.drawJpg(&f, 0, y, SCREEN_W, h)){
            M5.Display.setTextColor(RED);
            M5.Display.setCursor(6, STATUS_H + 20);
            M5.Display.print("JPG open failed");
          }
          f.close();
        } else {
          M5.Display.setTextColor(RED);
          M5.Display.setCursor(6, STATUS_H + 20);
          M5.Display.print("JPG open failed");
        }
      }
      else {
        M5.Display.fillScreen(BLACK);
        drawStatus();
        M5.Display.setTextColor(COLOR_DIM);
        M5.Display.setCursor(6, STATUS_H + 20);
        M5.Display.print("Viewer: txt only");
      }
    } else {
      M5.Display.fillScreen(BLACK);
      drawStatus();
      M5.Display.setTextColor(COLOR_DIM);
      M5.Display.setCursor(6, STATUS_H + 20);
      M5.Display.print("Viewer: LittleFS only");
    }
    while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
    drawFilesBrowser();
  }
}
