#include "screens/menu.h"
#include "core/ui.h"
#include "modules/network/wifi.h"
#include "modules/media/media.h"
#include "modules/features/clock.h"

void screenMenuUpdate(){
  if (M5.BtnB.wasPressed()) { menuIndex = (menuIndex + 1) % MENU_COUNT; drawMenu(); }
  if (M5.BtnPWR.wasPressed()) { menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (menuIndex == 0){ scanWifi(); wifiIndex = 0; screen = SCR_WIFI_LIST; drawWifiList(); }
    else if (menuIndex == 1){ screen = SCR_CLOCK; drawClock(false); }
    else if (menuIndex == 2){ imgIndex = 0; screen = SCR_IMG_LIST; drawList("Images", imgFiles, imgCount, imgIndex); }
    else if (menuIndex == 3){ gifIndex = 0; screen = SCR_GIF_LIST; drawList("Gifs", gifFiles, gifCount, gifIndex); }
    else if (menuIndex == 4){ screen = SCR_TXT_LIST; drawList("Text", txtFiles, txtCount, 0); }
    else if (menuIndex == 5){ gameIndex = 0; screen = SCR_GAMES_LIST; drawListCstr("Games", GAME_ITEMS, GAME_COUNT, gameIndex); }
    else if (menuIndex == 6){ testIndex = 0; screen = SCR_TEST_LIST; drawListCstr("Test", TEST_ITEMS, TEST_COUNT, testIndex); }
    else if (menuIndex == 7){ irIndex = 0; screen = SCR_IR_LIST; drawList("IR", irFiles, irCount, irIndex); }
    else if (menuIndex == 8){ screen = SCR_FILES_LIST; filesSource = FS_LITTLEFS; drawFilesList(); }
    else if (menuIndex == 9){ setIndex = 0; screen = SCR_SETTING; drawSetting(); }
    else if (menuIndex == 10){ screen = SCR_CHEAT; }
  }
}
