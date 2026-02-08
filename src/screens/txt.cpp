#include "screens/txt.h"
#include "core/ui.h"
#include "modules/txt.h"

static int txtIndex = 0;

void screenTxtListUpdate(){
  if (txtCount == 0){
    M5.Display.fillScreen(COLOR_BG);
    drawStatus();
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 20);
    M5.Display.print("No TXT files");
    if (M5.BtnPWR.wasPressed()) { screen = SCR_MENU; drawMenu(); }
    return;
  }

  if (M5.BtnB.wasPressed()) { txtIndex = (txtIndex + 1) % txtCount; drawList("Text", txtFiles, txtCount, txtIndex); }
  if (M5.BtnPWR.wasPressed()) { txtIndex = (txtIndex + 1) % txtCount; drawList("Text", txtFiles, txtCount, txtIndex); }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    showTXT(SPIFFS, txtFiles[txtIndex].c_str());
    drawList("Text", txtFiles, txtCount, txtIndex);
  }
}
