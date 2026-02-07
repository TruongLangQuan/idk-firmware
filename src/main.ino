// src/main.ino
// M5StickC Plus2 firmware - modularized + screen split

#include "app/state.h"
#include "core/ui.h"
#include "core/input.h"
#include "system/power.h"
#include "modules/wifi.h"
#include "modules/clock.h"
#include "modules/media.h"
#include "modules/ir.h"
#include "modules/files.h"
#include "modules/games.h"
#include "modules/test.h"
#include "modules/webui.h"
#include "modules/wifi.h"

#include "screens/menu.h"
#include "screens/wifi.h"
#include "screens/clock.h"
#include "screens/img.h"
#include "screens/gif.h"
#include "screens/txt.h"
#include "screens/games.h"
#include "screens/test.h"
#include "screens/ir.h"
#include "screens/files.h"
#include "screens/setting.h"

void setup(){
  Serial.begin(115200);
  disableCore0WDT();
  disableLoopWDT();
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Power.begin();
  irsend.begin();

  M5.Display.setRotation(3); // landscape-left (flipped if needed)
  M5.Display.setTextFont(0);
  M5.Display.setSwapBytes(true);
  setBacklight(BRIGHT_NORMAL);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed!");
  }

  sdReady = false;
  scanSPIFFS();
  setupTime();
  webuiInit();
  wifiAutoConnect();
  randomSeed(millis());

  drawMenu();
  lastActivity = millis();
}

void loop(){
  M5.update();

  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()){
    lastActivity = millis();
    if(isDimmed){ setBacklight(BRIGHT_NORMAL); isDimmed=false; }
  }

  static uint32_t t = 0;
  if (millis() - t > 5000){ drawStatus(); t = millis(); }

  if (dimIndex != 0 && !isDimmed && (millis() - lastActivity >= DIM_MS[dimIndex])){
    setBacklight(BRIGHT_DIM);
    isDimmed = true;
  }

  switch (screen){
    case SCR_MENU:
      screenMenuUpdate();
      break;
    case SCR_WIFI_LIST:
      screenWifiUpdate();
      break;
    case SCR_CLOCK:
      screenClockUpdate();
      break;
    case SCR_IMG_LIST:
      screenImgListUpdate();
      break;
    case SCR_GIF_LIST:
      screenGifListUpdate();
      break;
    case SCR_TXT_LIST:
      screenTxtListUpdate();
      break;
    case SCR_GAMES_LIST:
      screenGamesUpdate();
      break;
    case SCR_TEST_LIST:
      screenTestUpdate();
      break;
    case SCR_IR_LIST:
      screenIrListUpdate();
      break;
    case SCR_IR_CMD_LIST:
      screenIrCmdUpdate();
      break;
    case SCR_FILES_LIST:
      screenFilesListUpdate();
      break;
    case SCR_FILES_BROWSER:
      screenFilesBrowserUpdate();
      break;
    case SCR_SETTING:
      screenSettingUpdate();
      break;
    default:
      break;
  }

  webuiLoop();
  delay(1);
}
