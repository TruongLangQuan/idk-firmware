#include "screens/files.h"
#include "core/ui.h"
#include "modules/files.h"
#include "modules/media.h"

void screenFilesListUpdate(){
  if (M5.BtnB.wasPressed()) { filesSource = (FileSource)((filesSource + 1) % 3); drawFilesList(); }
  if (M5.BtnPWR.wasPressed()) { filesSource = (FileSource)((filesSource - 1 + 3) % 3); drawFilesList(); }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (filesSource == FS_WEBUI){
      M5.Display.fillScreen(BLACK);
      drawStatus();
      M5.Display.setTextColor(WHITE);
      M5.Display.setCursor(6, STATUS_H + 20);
      M5.Display.print("WebUI TBD (Bruce)");
      while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      drawFilesList();
    } else {
      if (filesSource == FS_SD && !sdReady){
        sdReady = SD.begin();
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
  if (M5.BtnA.wasPressed() && browserCount>0){
    String name = browserFiles[browserIndex];
    if (filesSource == FS_LITTLEFS){
      if (name.endsWith(".png")) showPNG(name.c_str());
      else if (name.endsWith(".gif")) playGIFLoop(name.c_str());
      else {
        M5.Display.fillScreen(BLACK);
        drawStatus();
        M5.Display.setTextColor(WHITE);
        M5.Display.setCursor(6, STATUS_H + 20);
        M5.Display.print(name);
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
