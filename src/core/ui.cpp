#include "core/ui.h"
#include "modules/config.h"

void drawStatus(){
  M5.Display.fillRect(0,0,SCREEN_W,STATUS_H,BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(4,2);
  M5.Display.print("Bat:");
  int bat = M5.Power.getBatteryLevel();
  uint16_t color = GREEN;
  if (bat <= 20) color = RED; else if (bat <= 50) color = YELLOW;
  M5.Display.setTextColor(color);
  M5.Display.printf("%d%%", bat);

  // Storage usage (SPIFFS) next to battery
  size_t total = SPIFFS.totalBytes();
  size_t used = SPIFFS.usedBytes();
  int totalMB = (int)((total + (1024*1024 - 1)) / (1024*1024));
  int usedMB = (int)((used + (1024*1024 - 1)) / (1024*1024));
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(80, 2);
  M5.Display.printf("%dMB/%dMB", usedMB, totalMB);

  // Max flash size
  int flashMB = (int)(ESP.getFlashChipSize() / (1024 * 1024));
  M5.Display.setCursor(160, 2);
  M5.Display.printf("MAX:%dMB", flashMB);

  // WiFi icon on the right when connected
  if (WiFi.status() == WL_CONNECTED){
    int x = SCREEN_W - 20;
    int y = 4;
    M5.Display.setTextColor(WHITE);
    M5.Display.fillRect(x + 0, y + 6, 3, 2, WHITE);
    M5.Display.fillRect(x + 5, y + 4, 3, 4, WHITE);
    M5.Display.fillRect(x + 10, y + 2, 3, 6, WHITE);
  }

  // SD card icon when mounted (sdReady)
  if (sdReady){
    int sx = SCREEN_W - 40;
    int sy = 2;
    // simple SD card shape: outer rect + small notch
    M5.Display.drawRect(sx, sy, 12, 12, WHITE);
    // notch (top-right cut)
    M5.Display.fillRect(sx + 8, sy, 4, 4, COLOR_BG);
    // small inner square to denote card window
    M5.Display.fillRect(sx + 3, sy + 4, 6, 6, WHITE);
    // optional label 'SD'
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(BLACK);
    M5.Display.setCursor(sx + 2, sy + 6);
    M5.Display.print("SD");
  }
}

void drawMenu(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  int maxShow = 8;
  int first = 0;
  if (menuIndex >= maxShow) first = menuIndex - maxShow + 1;
  for(int i=0;i<maxShow;i++){
    int idx = first + i;
    if (idx >= MENU_COUNT) break;
    int y = STATUS_H + 8 + i*12;
    if (idx == menuIndex){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    M5.Display.setCursor(8,y);
    M5.Display.print(MENU_ITEMS[idx]);
  }
}

void drawList(const char* title, String *items, int count, int index){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 2);
  M5.Display.print(title);
  int startY = STATUS_H + 14;
  int maxShow = 9;
  int first = 0;
  if (index >= maxShow) first = index - maxShow + 1;
  for (int i=0;i<maxShow;i++){
    int idx = first + i;
    if (idx >= count) break;
    int y = startY + i*12;
    if (idx == index){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    String name = items[idx];
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.setCursor(8,y);
    M5.Display.print(name);
  }
}

void drawListCstr(const char* title, const char** items, int count, int index){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 2);
  M5.Display.print(title);
  int startY = STATUS_H + 14;
  int maxShow = 9;
  int first = 0;
  if (index >= maxShow) first = index - maxShow + 1;
  for (int i=0;i<maxShow;i++){
    int idx = first + i;
    if (idx >= count) break;
    int y = startY + i*12;
    if (idx == index){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    M5.Display.setCursor(8,y);
    M5.Display.print(items[idx]);
  }
}

void drawSetting(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  for(int i=0;i<SET_COUNT;i++){
    int y = STATUS_H + 12 + i*16;
    if (i == setIndex){
      M5.Display.fillRect(0,y-2,SCREEN_W,12,COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    M5.Display.setCursor(8,y);
    M5.Display.print(SET_ITEMS[i]);
    if (i == 0){
      M5.Display.setCursor(150,y);
      M5.Display.print(DIM_TEXT[dimIndex]);
    }
    if (i == 2){
      M5.Display.setCursor(150,y);
      M5.Display.print(webuiEnabled ? "On" : "Off");
    }
    if (i == 6){
      M5.Display.setCursor(150,y);
      M5.Display.print(configGetIrPin());
    }
    if (i == 7){
      M5.Display.setCursor(150,y);
      M5.Display.print(configGetSdCsPin());
    }
  }
}

void drawIRCmdList(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  int maxShow = 8;
  for(int i=0;i<maxShow;i++){
    int idx = cmdScroll + i;
    if(idx >= cmdCount) break;
    int y = STATUS_H + 12 + i*14;
    if(idx == cmdIndex){
      M5.Display.fillRect(0,y-2,SCREEN_W,12,COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    M5.Display.setCursor(8,y);
    if(cmdName[idx].startsWith("RAW:")) M5.Display.print("RAW");
    else M5.Display.print(cmdName[idx]);
  }
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("M5=Send  Next=Down  Prev=Back");
}

void drawWifiList(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 2);
  M5.Display.print("WiFi Scan");
  int startY = STATUS_H + 14;
  int maxShow = 8;
  int first = 0;
  if (wifiIndex >= maxShow) first = wifiIndex - maxShow + 1;
  for (int i=0;i<maxShow;i++){
    int idx = first + i;
    if (idx >= wifiCount) break;
    int y = startY + i*14;
    if (idx == wifiIndex){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    String ssid = wifiSSID[idx];
    if (ssid.length() > 16) ssid = ssid.substring(0,16);
    M5.Display.setCursor(8,y);
    M5.Display.printf("%s (%ddB)", ssid.c_str(), wifiRSSI[idx]);
  }
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("M5=Select  Prev=Back");
}

void drawFilesList(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 2);
  M5.Display.print("Files");
  const char* sources[] = {"WebUI","LittleFS","SDCard"};
  for (int i=0;i<3;i++){
    int y = STATUS_H + 16 + i*14;
    if (i == (int)filesSource){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    M5.Display.setCursor(8,y);
    M5.Display.print(sources[i]);
  }
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("M5=Open  Prev=Back");
}

void drawFilesBrowser(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(6, STATUS_H + 2);
  M5.Display.print(filesSource == FS_SD ? "SD" : "LittleFS");
  int startY = STATUS_H + 14;
  int maxShow = 8;
  int first = 0;
  if (browserIndex >= maxShow) first = browserIndex - maxShow + 1;
  for (int i=0;i<maxShow;i++){
    int idx = first + i;
    if (idx >= browserCount) break;
    int y = startY + i*14;
    if (idx == browserIndex){
      M5.Display.fillRect(0, y-2, SCREEN_W, 12, COLOR_HI);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(COLOR_DIM);
    }
    String name = browserFiles[idx];
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.setCursor(8,y);
    M5.Display.print(name);
  }
  M5.Display.setTextColor(COLOR_DIM);
  M5.Display.setCursor(6, SCREEN_H-10);
  M5.Display.print("M5=Open  Prev=Back");
}
