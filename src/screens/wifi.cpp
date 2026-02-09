#include "screens/wifi.h"
#include "core/ui.h"
#include "modules/network/wifi.h"

void screenWifiUpdate(){
  if (M5.BtnB.wasPressed()) { if (wifiCount>0) { wifiIndex = (wifiIndex + 1) % wifiCount; drawWifiList(); } }
  if (M5.BtnPWR.wasPressed()) { if (wifiCount>0) { wifiIndex = (wifiIndex - 1 + wifiCount) % wifiCount; drawWifiList(); } }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()) { wifiConnectTo(wifiIndex); drawWifiList(); }
}
