#include "screens/clock.h"
#include "core/ui.h"
#include "modules/clock.h"

void screenClockUpdate(){
  if (M5.BtnPWR.wasPressed()) { screen = SCR_MENU; drawMenu(); return; }
  static uint32_t lastClock = 0;
  if (millis() - lastClock > 1000){ drawClock(); lastClock = millis(); }
}
