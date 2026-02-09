#include "screens/gif.h"
#include "core/ui.h"
#include "modules/media/media.h"

void screenGifListUpdate(){
  if (M5.BtnB.wasPressed()) { if (gifCount>0) { gifIndex = (gifIndex + 1) % gifCount; drawList("Gifs", gifFiles, gifCount, gifIndex);} }
  if (M5.BtnPWR.wasPressed()) { if (gifCount>0) { gifIndex = (gifIndex - 1 + gifCount) % gifCount; drawList("Gifs", gifFiles, gifCount, gifIndex);} }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed() && gifCount>0){
    String path = gifFiles[gifIndex];
    screen = SCR_GIF_VIEW;
    playGIFLoop(path.c_str());
    screen = SCR_GIF_LIST; drawList("Gifs", gifFiles, gifCount, gifIndex);
  }
}
