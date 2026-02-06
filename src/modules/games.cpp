#include "modules/games.h"
#include "core/ui.h"
#include <math.h>

struct Tetromino { int shape[4][4]; uint16_t color; };
static const Tetromino TETS[] = {
  {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, CYAN},
  {{{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, YELLOW},
  {{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, MAGENTA},
  {{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, ORANGE},
  {{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, BLUE},
  {{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, GREEN},
  {{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, RED}
};

static void rotateShape(int s[4][4]){
  int tmp[4][4];
  for (int y=0;y<4;y++) for (int x=0;x<4;x++) tmp[y][x] = s[3-x][y];
  for (int y=0;y<4;y++) for (int x=0;x<4;x++) s[y][x] = tmp[y][x];
}

void runTetris(){
  const int W = 10, H = 16;
  uint16_t grid[H][W];
  for (int y=0;y<H;y++) for(int x=0;x<W;x++) grid[y][x] = 0;

  int cur = random(0,7);
  int sx = 3, sy = 0;
  int shape[4][4];
  memcpy(shape, TETS[cur].shape, sizeof(shape));
  uint32_t lastDrop = millis();
  uint32_t dropInterval = 500;
  bool rotateLatch = false;

  auto canPlace = [&](int nx, int ny, int s[4][4]){
    for (int y=0;y<4;y++) for (int x=0;x<4;x++){
      if (!s[y][x]) continue;
      int gx = nx + x, gy = ny + y;
      if (gx < 0 || gx >= W || gy < 0 || gy >= H) return false;
      if (grid[gy][gx]) return false;
    }
    return true;
  };

  auto lockPiece = [&](){
    for (int y=0;y<4;y++) for (int x=0;x<4;x++){
      if (!shape[y][x]) continue;
      int gx = sx + x, gy = sy + y;
      if (gy>=0 && gy<H && gx>=0 && gx<W) grid[gy][gx] = TETS[cur].color;
    }
    for (int y=H-1; y>=0; y--){
      bool full = true;
      for (int x=0;x<W;x++) if (!grid[y][x]) { full=false; break; }
      if (full){
        for (int yy=y; yy>0; yy--) for (int x=0;x<W;x++) grid[yy][x] = grid[yy-1][x];
        for (int x=0;x<W;x++) grid[0][x] = 0;
        y++;
      }
    }
  };

  auto spawn = [&](){
    cur = random(0,7);
    memcpy(shape, TETS[cur].shape, sizeof(shape));
    sx = 3; sy = 0;
  };

  while (true){
    M5.update();
    if (M5.BtnPWR.isPressed() && M5.BtnA.isPressed()) break;

    if (M5.BtnPWR.wasPressed()){
      if (canPlace(sx-1, sy, shape)) sx--;
    }
    if (M5.BtnB.wasPressed()){
      if (canPlace(sx+1, sy, shape)) sx++;
    }

    bool both = M5.BtnPWR.isPressed() && M5.BtnB.isPressed();
    if (both && !rotateLatch){
      int tmp[4][4]; memcpy(tmp, shape, sizeof(tmp));
      rotateShape(tmp);
      if (canPlace(sx, sy, tmp)) memcpy(shape, tmp, sizeof(tmp));
      rotateLatch = true;
    }
    if (!both) rotateLatch = false;

    uint32_t now = millis();
    uint32_t interval = M5.BtnA.isPressed() ? 80 : dropInterval;
    if (now - lastDrop > interval){
      lastDrop = now;
      if (canPlace(sx, sy+1, shape)) sy++;
      else {
        lockPiece();
        spawn();
        if (!canPlace(sx, sy, shape)) break;
      }
    }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    int cell = 8;
    int ox = 10;
    int oy = STATUS_H + 4;
    for (int y=0;y<H;y++) for (int x=0;x<W;x++){
      if (grid[y][x]) M5.Display.fillRect(ox + x*cell, oy + y*cell, cell-1, cell-1, grid[y][x]);
    }
    for (int y=0;y<4;y++) for (int x=0;x<4;x++){
      if (!shape[y][x]) continue;
      int px = ox + (sx + x)*cell;
      int py = oy + (sy + y)*cell;
      M5.Display.fillRect(px, py, cell-1, cell-1, TETS[cur].color);
    }
    delay(10);
  }
}

void runFlappy(){
  float birdY = 60;
  float vel = 0;
  int pipeX = SCREEN_W;
  int gapY = 60;
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnA.wasPressed()) vel = -2.8f;

    vel += 0.15f;
    birdY += vel;
    pipeX -= 2;
    if (pipeX < -20){
      pipeX = SCREEN_W;
      gapY = 40 + random(0,50);
    }

    if (birdY < STATUS_H || birdY > SCREEN_H-4) break;
    if (pipeX < 20 && pipeX > 0){
      if (birdY < gapY-18 || birdY > gapY+18) break;
    }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.fillCircle(20, (int)birdY, 3, YELLOW);
    M5.Display.fillRect(pipeX, STATUS_H, 10, gapY-18-STATUS_H, GREEN);
    M5.Display.fillRect(pipeX, gapY+18, 10, SCREEN_H-(gapY+18), GREEN);
    delay(16);
  }
}

void runSlot(){
  const char* symbols[] = {"A","B","C","7","*"};
  int s1=0,s2=1,s3=2;
  bool spinning = false;
  int stopStage = 0;
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnA.wasPressed()){
      spinning = true;
      stopStage = 0;
    }
    if (M5.BtnB.wasPressed() && spinning){
      stopStage++;
      if (stopStage >= 3) spinning = false;
    }
    if (spinning){
      if (stopStage < 1) s1 = (s1+1)%5;
      if (stopStage < 2) s2 = (s2+1)%5;
      if (stopStage < 3) s3 = (s3+1)%5;
    }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(40, 50); M5.Display.print(symbols[s1]);
    M5.Display.setCursor(100, 50); M5.Display.print(symbols[s2]);
    M5.Display.setCursor(160, 50); M5.Display.print(symbols[s3]);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.print("M5=Spin  Next=Stop  Prev=Exit");
    delay(80);
  }
  M5.Display.setTextSize(1);
}

void runDino(){
  float dinoY = SCREEN_H-20;
  float vel = 0;
  int cactusX = SCREEN_W;
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnA.wasPressed() && dinoY >= SCREEN_H-20) vel = -3.5f;
    vel += 0.2f;
    dinoY += vel;
    if (dinoY > SCREEN_H-20){ dinoY = SCREEN_H-20; vel = 0; }
    cactusX -= 3;
    if (cactusX < -10) cactusX = SCREEN_W;

    if (cactusX < 24 && cactusX > 10 && dinoY > SCREEN_H-30) break;

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.drawFastHLine(0, SCREEN_H-10, SCREEN_W, WHITE);
    M5.Display.fillRect(12, (int)dinoY, 10, 10, WHITE);
    M5.Display.fillRect(cactusX, SCREEN_H-20, 6, 10, GREEN);
    delay(16);
  }
}
