#include "screens/games.h"
#include "core/ui.h"
#include "modules/games.h"

void screenGamesUpdate(){
  if (M5.BtnB.wasPressed()) { gameIndex = (gameIndex + 1) % GAME_COUNT; drawListCstr("Games", GAME_ITEMS, GAME_COUNT, gameIndex); }
  if (M5.BtnPWR.wasPressed()) { gameIndex = (gameIndex - 1 + GAME_COUNT) % GAME_COUNT; drawListCstr("Games", GAME_ITEMS, GAME_COUNT, gameIndex); }
  if (M5.BtnPWR.pressedFor(800)) { screen = SCR_MENU; drawMenu(); }
  if (M5.BtnA.wasPressed()){
    if (gameIndex == 0) runTetris();
    else if (gameIndex == 1) runFlappy();
    else if (gameIndex == 2) runSlot();
    else if (gameIndex == 3) runDino();
    drawListCstr("Games", GAME_ITEMS, GAME_COUNT, gameIndex);
  }
}
