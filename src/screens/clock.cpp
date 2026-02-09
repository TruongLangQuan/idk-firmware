#include "screens/clock.h"
#include "core/ui.h"
#include "modules/features/clock.h"

void screenClockUpdate(){
  if (M5.BtnPWR.wasPressed()) { 
    screen = SCR_MENU; 
    drawMenu(); // This will add status bar back
    return; 
  }
  static uint32_t lastClock = 0;
  if (millis() - lastClock > 1000){ 
    drawClock(false); // Don't show status bar
    lastClock = millis(); 
  }
}
