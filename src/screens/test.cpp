#include "screens/test.h"
#include "core/ui.h"
#include "core/input.h"
#include "modules/test.h"

void screenTestUpdate(){
  if (M5.BtnB.wasPressed()) { testIndex = (testIndex + 1) % TEST_COUNT; drawListCstr("Test", TEST_ITEMS, TEST_COUNT, testIndex); }
  if (M5.BtnPWR.wasPressed()) { testIndex = (testIndex - 1 + TEST_COUNT) % TEST_COUNT; drawListCstr("Test", TEST_ITEMS, TEST_COUNT, testIndex); }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (testIndex == 0){
      String txt = "";
      if (textInput("Input Text", txt, 64, false)){
        M5.Display.fillScreen(BLACK);
        drawStatus();
        M5.Display.setTextColor(WHITE);
        M5.Display.setCursor(6, STATUS_H + 20);
        M5.Display.print(txt);
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      }
    } else if (testIndex == 1){
      String expr = "sin(x)";
      if (textInput("f(x)=", expr, 32, false)){
        drawPlot(expr);
        while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
      }
    } else if (testIndex == 2){
      runCube();
    } else if (testIndex == 3){
      runUniverse();
    } else if (testIndex == 4){
      runIllusion();
    } else if (testIndex == 5){
      runDraw();
    }
    drawListCstr("Test", TEST_ITEMS, TEST_COUNT, testIndex);
  }
}
