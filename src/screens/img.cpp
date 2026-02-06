#include "screens/img.h"
#include "core/ui.h"
#include "modules/media.h"

void screenImgListUpdate(){
  if (M5.BtnB.wasPressed()) { if (imgCount>0) { imgIndex = (imgIndex + 1) % imgCount; drawList("Images", imgFiles, imgCount, imgIndex);} }
  if (M5.BtnPWR.wasPressed()) { if (imgCount>0) { imgIndex = (imgIndex - 1 + imgCount) % imgCount; drawList("Images", imgFiles, imgCount, imgIndex);} }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed() && imgCount>0){
    String path = imgFiles[imgIndex];
    screen = SCR_IMG_VIEW;
    showPNG(path.c_str());
    while (true){ M5.update(); if (M5.BtnPWR.wasPressed()) break; delay(10); }
    screen = SCR_IMG_LIST; drawList("Images", imgFiles, imgCount, imgIndex);
  }
}
