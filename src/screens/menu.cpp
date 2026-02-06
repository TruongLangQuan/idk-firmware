#include "screens/menu.h"
#include "core/ui.h"
#include "modules/wifi.h"
#include "modules/media.h"
#include "modules/clock.h"

void screenMenuUpdate(){
  if (M5.BtnB.wasPressed()) { menuIndex = (menuIndex + 1) % MENU_COUNT; drawMenu(); }
  if (M5.BtnPWR.wasPressed()) { menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (menuIndex == 0){ scanWifi(); wifiIndex = 0; screen = SCR_WIFI_LIST; drawWifiList(); }
    else if (menuIndex == 1){ screen = SCR_CLOCK; drawClock(); }
    else if (menuIndex == 2){ imgIndex = 0; screen = SCR_IMG_LIST; drawList("Images", imgFiles, imgCount, imgIndex); }
    else if (menuIndex == 3){ gifIndex = 0; screen = SCR_GIF_LIST; drawList("Gifs", gifFiles, gifCount, gifIndex); }
    else if (menuIndex == 4){ gameIndex = 0; screen = SCR_GAMES_LIST; drawListCstr("Games", GAME_ITEMS, GAME_COUNT, gameIndex); }
    else if (menuIndex == 5){ testIndex = 0; screen = SCR_TEST_LIST; drawListCstr("Test", TEST_ITEMS, TEST_COUNT, testIndex); }
    else if (menuIndex == 6){ irIndex = 0; screen = SCR_IR_LIST; drawList("IR", irFiles, irCount, irIndex); }
    else if (menuIndex == 7){ screen = SCR_FILES_LIST; filesSource = FS_LITTLEFS; drawFilesList(); }
    else if (menuIndex == 8){ setIndex = 0; screen = SCR_SETTING; drawSetting(); }
  }
}
