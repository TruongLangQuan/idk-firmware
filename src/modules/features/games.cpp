#include "modules/features/games.h"
#include "core/ui.h"
#include <math.h>
#include <Preferences.h>

static Preferences scorePrefs;
static const char* SCORE_NS = "scores";

static int getScore(const char* key){
  scorePrefs.begin(SCORE_NS, true);
  int v = scorePrefs.getInt(key, 0);
  scorePrefs.end();
  return v;
}

static void setScore(const char* key, int v){
  scorePrefs.begin(SCORE_NS, false);
  scorePrefs.putInt(key, v);
  scorePrefs.end();
}

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
  int prevRot = M5.Display.getRotation();
  M5.Display.setRotation(0); // portrait for tetris
  delay(20);
  int dispW = M5.Display.width();
  int dispH = M5.Display.height();
  const int statusH = 16;

  const int W = 10, H = 20;
  uint16_t grid[H][W];
  for (int y=0;y<H;y++) for(int x=0;x<W;x++) grid[y][x] = 0;

  int cur = random(0,7);
  int next = random(0,7); // Store next piece separately
  int sx = 3, sy = 0;
  int shape[4][4];
  memcpy(shape, TETS[cur].shape, sizeof(shape));
  uint32_t lastDrop = millis();
  int score = 0;
  int lines = 0;
  int level = 1;
  uint32_t dropInterval = 500;
  int high = getScore("tetris_h");
  bool rotateLatch = false;
  bool gameOver = false;
  bool exitGame = false;

  auto darken = [&](uint16_t c, uint8_t shift){
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5) & 0x3F;
    uint8_t b = c & 0x1F;
    r = (r > shift) ? (r - shift) : 0;
    g = (g > shift) ? (g - (shift * 2)) : 0;
    b = (b > shift) ? (b - shift) : 0;
    return (uint16_t)((r << 11) | (g << 5) | b);
  };

  auto drawStatusT = [&](){
    M5.Display.fillRect(0,0,dispW,statusH,BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(2,2);
    M5.Display.print("Bat:");
    int bat = M5.Power.getBatteryLevel();
    uint16_t c = (bat<=20)?RED:((bat<=50)?YELLOW:GREEN);
    M5.Display.setTextColor(c);
    M5.Display.printf("%d%%", bat);
  };

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
    int cleared = 0;
    for (int y=H-1; y>=0; y--){
      bool full = true;
      for (int x=0;x<W;x++) if (!grid[y][x]) { full=false; break; }
      if (full){
        for (int yy=y; yy>0; yy--) for (int x=0;x<W;x++) grid[yy][x] = grid[yy-1][x];
        for (int x=0;x<W;x++) grid[0][x] = 0;
        y++;
        cleared++;
      }
    }
    if (cleared > 0){
      lines += cleared;
      level = 1 + lines/10;
      score += cleared * 100 * level;
      dropInterval = 500 - (level-1)*30;
      if (dropInterval < 120) dropInterval = 120;
    }
  };

  auto spawn = [&](){
    cur = next; // Use the stored next piece
    next = random(0,7); // Generate new next piece
    memcpy(shape, TETS[cur].shape, sizeof(shape));
    sx = 3; sy = 0;
  };

  while (true){
    M5.update();
    if (M5.BtnPWR.isPressed() && M5.BtnA.isPressed()) { exitGame = true; break; }

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
        if (!canPlace(sx, sy, shape)) { gameOver = true; break; }
      }
    }

    M5.Display.fillScreen(BLACK);
    drawStatusT();

    int cell = 10;
    int boardW = W * cell;
    int boardH = H * cell;
    int ox = 2;
    int oy = statusH + 4;
    if (oy + boardH > dispH - 4){
      cell = 9;
      boardW = W * cell;
      boardH = H * cell;
    }

    // board frame + grid
    M5.Display.drawRect(ox-2, oy-2, boardW+4, boardH+4, 0x39E7);
    for (int y=0;y<=H;y++){
      int yy = oy + y*cell;
      M5.Display.drawFastHLine(ox, yy, boardW, 0x18C3);
    }
    for (int x=0;x<=W;x++){
      int xx = ox + x*cell;
      M5.Display.drawFastVLine(xx, oy, boardH, 0x18C3);
    }

    // stats panel
    int panelX = ox + boardW + 6;
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(panelX, statusH + 6);
    // Fix score overflow by limiting display width
    char scoreStr[16];
    snprintf(scoreStr, sizeof(scoreStr), "S:%d", score);
    if (panelX + strlen(scoreStr) * 6 > dispW) {
      // Truncate score if too long
      snprintf(scoreStr, sizeof(scoreStr), "S:%dK", score / 1000);
    }
    M5.Display.print(scoreStr);
    M5.Display.setCursor(panelX, statusH + 18);
    M5.Display.printf("L:%d", level);
    M5.Display.setCursor(panelX, statusH + 30);
    char highStr[16];
    snprintf(highStr, sizeof(highStr), "H:%d", high);
    if (panelX + strlen(highStr) * 6 > dispW) {
      snprintf(highStr, sizeof(highStr), "H:%dK", high / 1000);
    }
    M5.Display.print(highStr);

    // next piece preview - use stored next variable
    int px0 = panelX;
    int py0 = statusH + 46;
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(panelX, statusH + 42);
    M5.Display.print("NEXT");
    for (int y=0;y<4;y++) for (int x=0;x<4;x++){
      if (!TETS[next].shape[y][x]) continue;
      int px = px0 + x*(cell-3);
      int py = py0 + y*(cell-3);
      uint16_t c = TETS[next].color;
      M5.Display.fillRect(px, py, cell-4, cell-4, c);
      M5.Display.drawRect(px, py, cell-4, cell-4, darken(c, 4));
    }

    // blocks
    for (int y=0;y<H;y++) for (int x=0;x<W;x++){
      if (!grid[y][x]) continue;
      int px = ox + x*cell + 1;
      int py = oy + y*cell + 1;
      uint16_t c = grid[y][x];
      M5.Display.fillRect(px, py, cell-2, cell-2, c);
      M5.Display.drawRect(px, py, cell-2, cell-2, darken(c, 4));
      M5.Display.drawFastHLine(px+1, py+1, cell-4, 0xFFFF);
    }
    for (int y=0;y<4;y++) for (int x=0;x<4;x++){
      if (!shape[y][x]) continue;
      int px = ox + (sx + x)*cell + 1;
      int py = oy + (sy + y)*cell + 1;
      uint16_t c = TETS[cur].color;
      M5.Display.fillRect(px, py, cell-2, cell-2, c);
      M5.Display.drawRect(px, py, cell-2, cell-2, darken(c, 4));
      M5.Display.drawFastHLine(px+1, py+1, cell-4, 0xFFFF);
    }
    delay(10);
  }

  setScore("tetris_last", score);
  if (gameOver && score > high) setScore("tetris_h", score);
  M5.Display.setRotation(prevRot);
}

void runFlappy(){
  float birdY = 60;
  float vel = 0;
  int pipeX = SCREEN_W;
  int gapY = 60;
  int score = 0;
  int level = 1;
  int high = getScore("flappy_h");
  bool passed = false;
  bool gameOver = false;
  bool exitGame = false;

  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) { exitGame = true; break; }
    if (M5.BtnA.wasPressed()) vel = -2.8f;

    vel += 0.15f;
    birdY += vel;
    float speed = 2.0f + (level-1)*0.4f;
    pipeX -= (int)speed;
    if (pipeX < -20){
      pipeX = SCREEN_W;
      gapY = 40 + random(0,50);
      passed = false;
    }

    if (!passed && pipeX < 18){
      passed = true;
      score += 1;
      if (score % 5 == 0) level++;
    }

    if (birdY < STATUS_H || birdY > SCREEN_H-4) { gameOver = true; break; }
    if (pipeX < 20 && pipeX > 0){
      if (birdY < gapY-18 || birdY > gapY+18) { gameOver = true; break; }
    }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(150, STATUS_H + 4);
    M5.Display.printf("S:%d", score);
    M5.Display.setCursor(150, STATUS_H + 16);
    M5.Display.printf("L:%d", level);
    M5.Display.setCursor(150, STATUS_H + 28);
    M5.Display.printf("H:%d", high);

    // Improved bird graphics - larger with wing animation
    int birdSize = 5;
    M5.Display.fillCircle(20, (int)birdY, birdSize, YELLOW);
    M5.Display.fillCircle(18, (int)birdY-1, 2, WHITE); // eye
    // Wing
    int wingOffset = (int)(sin(millis()*0.1f) * 2);
    M5.Display.fillTriangle(22, (int)birdY, 25, (int)birdY-wingOffset, 25, (int)birdY+wingOffset, 0xFFE0);
    
    // Improved pipe graphics with gradient
    M5.Display.fillRect(pipeX, STATUS_H, 12, gapY-18-STATUS_H, GREEN);
    M5.Display.fillRect(pipeX, gapY+18, 12, SCREEN_H-(gapY+18), GREEN);
    // Pipe borders
    M5.Display.drawRect(pipeX, STATUS_H, 12, gapY-18-STATUS_H, 0x07E0);
    M5.Display.drawRect(pipeX, gapY+18, 12, SCREEN_H-(gapY+18), 0x07E0);
    // Pipe caps
    M5.Display.fillRect(pipeX-2, gapY-20, 16, 4, 0x07E0);
    M5.Display.fillRect(pipeX-2, gapY+16, 16, 4, 0x07E0);
    delay(16);
  }

  setScore("flappy_last", score);
  if (gameOver && score > high) setScore("flappy_h", score);
}

void runSlot(){
  const char* symbols[] = {"A","B","C","7","*"};
  int s1=0,s2=1,s3=2;
  bool spinning = false;
  int stopStage = 0;
  int score = 0;
  int level = 1;
  int high = getScore("slot_h");
  bool exitGame = false;

  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) { exitGame = true; break; }
    if (M5.BtnA.wasPressed()){
      spinning = true;
      stopStage = 0;
    }
    if (M5.BtnB.wasPressed() && spinning){
      stopStage++;
      if (stopStage >= 3){
        spinning = false;
        if (s1==s2 && s2==s3) score += 10 * level;
        else score = score > 0 ? score-1 : 0;
        level = 1 + score/50;
        if (score > high) setScore("slot_h", score);
      }
    }
    if (spinning){
      int speed = 1 + level/2;
      if (stopStage < 1) s1 = (s1+speed)%5;
      if (stopStage < 2) s2 = (s2+speed)%5;
      if (stopStage < 3) s3 = (s3+speed)%5;
    }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 4);
    M5.Display.printf("S:%d  L:%d  H:%d", score, level, high);

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

  setScore("slot_last", score);
  M5.Display.setTextSize(1);
}

void runDino(){
  float dinoY = SCREEN_H-20;
  float vel = 0;
  int cactusX = SCREEN_W;
  int score = 0;
  int level = 1;
  int high = getScore("dino_h");
  bool gameOver = false;
  bool exitGame = false;
  uint32_t lastTick = millis();

  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) { exitGame = true; break; }
    if (M5.BtnA.wasPressed() && dinoY >= SCREEN_H-20) vel = -3.5f;

    vel += 0.2f;
    dinoY += vel;
    if (dinoY > SCREEN_H-20){ dinoY = SCREEN_H-20; vel = 0; }

    float speed = 3.0f + (level-1)*0.5f;
    cactusX -= (int)speed;
    if (cactusX < -10) cactusX = SCREEN_W;

    uint32_t now = millis();
    if (now - lastTick > 200){
      score += 1;
      lastTick = now;
      if (score % 20 == 0) level++;
    }

    if (cactusX < 24 && cactusX > 10 && dinoY > SCREEN_H-30) { gameOver = true; break; }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 4);
    M5.Display.printf("S:%d  L:%d  H:%d", score, level, high);

    // Improved ground graphics
    M5.Display.fillRect(0, SCREEN_H-12, SCREEN_W, 12, 0x7BEF); // Ground color
    M5.Display.drawFastHLine(0, SCREEN_H-12, SCREEN_W, WHITE);
    
    // Improved dino graphics - larger and more detailed
    int dinoW = 14;
    int dinoH = 14;
    M5.Display.fillRect(12, (int)dinoY-dinoH, dinoW, dinoH, WHITE);
    // Dino head
    M5.Display.fillRect(12, (int)dinoY-dinoH-4, 8, 4, WHITE);
    // Dino eye
    M5.Display.fillRect(16, (int)dinoY-dinoH-2, 2, 2, BLACK);
    // Dino leg animation
    int legOffset = (int)(sin(millis()*0.2f) * 2);
    M5.Display.fillRect(14, (int)dinoY, 3, 4, WHITE);
    M5.Display.fillRect(20, (int)dinoY+legOffset, 3, 4, WHITE);
    
    // Improved cactus graphics - larger with details
    int cactusW = 10;
    int cactusH = 15;
    M5.Display.fillRect(cactusX, SCREEN_H-20-cactusH, cactusW, cactusH, GREEN);
    // Cactus branches
    M5.Display.fillRect(cactusX-3, SCREEN_H-20-cactusH+5, 4, 6, GREEN);
    M5.Display.fillRect(cactusX+cactusW-1, SCREEN_H-20-cactusH+8, 4, 6, GREEN);
    // Cactus details
    M5.Display.drawRect(cactusX, SCREEN_H-20-cactusH, cactusW, cactusH, 0x07E0);
    delay(16);
  }

  setScore("dino_last", score);
  if (gameOver && score > high) setScore("dino_h", score);
}
