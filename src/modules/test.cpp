#include "modules/test.h"
#include "core/ui.h"
#include <tinyexpr.h>
#include <math.h>
#include <ctype.h>

static String normalizeExpr(const String &in){
  String out = "";
  char prev = 0;
  for (int i=0;i<in.length();i++){
    char c = in[i];
    if (isspace((unsigned char)c)) continue;
    bool prevIsNum = (prev >= '0' && prev <= '9') || prev == '.';
    bool prevIsVar = (prev == 'x' || prev == 'X');
    bool prevIsClose = (prev == ')');
    bool curIsVar = (c == 'x' || c == 'X');
    bool curIsOpen = (c == '(');
    bool curIsNum = (c >= '0' && c <= '9') || c == '.';
    if ((prevIsNum || prevIsVar || prevIsClose) && (curIsVar || curIsOpen)){
      out += '*';
    } else if ((prevIsVar || prevIsClose) && curIsNum){
      out += '*';
    }
    out += c;
    prev = c;
  }
  return out;
}

void drawPlot(const String &expr){
  M5.Display.fillScreen(BLACK);
  drawStatus();

  for (int x=0;x<SCREEN_W;x+=24) M5.Display.drawFastVLine(x, STATUS_H, SCREEN_H-STATUS_H, 0x39E7);
  for (int y=STATUS_H;y<SCREEN_H;y+=22) M5.Display.drawFastHLine(0, y, SCREEN_W, 0x39E7);

  int cx = SCREEN_W/2;
  int cy = STATUS_H + (SCREEN_H-STATUS_H)/2;
  M5.Display.drawFastVLine(cx, STATUS_H, SCREEN_H-STATUS_H, WHITE);
  M5.Display.drawFastHLine(0, cy, SCREEN_W, WHITE);

  // Axis labels and ticks
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(SCREEN_W-8, cy-10);
  M5.Display.print("x");
  M5.Display.setCursor(cx+4, STATUS_H+2);
  M5.Display.print("y");

  const int ticks[] = {-10, -5, 0, 5, 10};
  for (int i=0;i<5;i++){
    int v = ticks[i];
    int px = (int)((v + 10) * (SCREEN_W-1) / 20.0);
    M5.Display.setCursor(px-4, cy+4);
    M5.Display.printf("%d", v);
    int py = cy - (int)(v * ((SCREEN_H-STATUS_H-1) / 20.0));
    M5.Display.setCursor(cx+4, py-4);
    M5.Display.printf("%d", v);
  }

  double xval = 0;
  te_variable vars[] = { {"x", &xval} };
  int err = 0;
  String fixed = normalizeExpr(expr);
  te_expr *e = te_compile(fixed.c_str(), vars, 1, &err);
  if (!e){
    M5.Display.setTextColor(RED);
    M5.Display.setCursor(6, STATUS_H + 6);
    M5.Display.print("Parse error");
    return;
  }

  for (int px=0; px<SCREEN_W; px++){
    double x = ((double)px / (SCREEN_W-1)) * 20.0 - 10.0;
    xval = x;
    double y = te_eval(e);
    if (y < -10 || y > 10) continue;
    int py = cy - (int)(y * ((SCREEN_H-STATUS_H-1) / 20.0));
    if (py >= STATUS_H && py < SCREEN_H) M5.Display.drawPixel(px, py, YELLOW);
  }
  te_free(e);
}

void runCube(){
  float angle = 0.0f;
  int shape = 0; // 0 cube, 1 tetra, 2 octa, 3 tesseract, 4 4D simplex, 5 4D cross
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnB.wasPressed()) { shape = (shape + 1) % 6; }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 2);
    if (shape == 0) M5.Display.print("Cube");
    else if (shape == 1) M5.Display.print("Tetra");
    else if (shape == 2) M5.Display.print("Octa");
    else if (shape == 3) M5.Display.print("4D Cube");
    else if (shape == 4) M5.Display.print("4D Simplex");
    else M5.Display.print("4D Cross");

    int cx = SCREEN_W/2;
    int cy = STATUS_H + (SCREEN_H-STATUS_H)/2;
    float s = 22.0f;

    auto drawLines = [&](int count, int pts[][2], const int edges[][2], int edgeCount){
      for (int i=0;i<edgeCount;i++){
        int a = edges[i][0], b = edges[i][1];
        M5.Display.drawLine(pts[a][0], pts[a][1], pts[b][0], pts[b][1], WHITE);
      }
    };

    if (shape >= 3){
      // 4D tesseract
      float pts4[16][4];
      int vcount = 0;
      if (shape == 3){
        for (int xi=0; xi<2; xi++){
          for (int yi=0; yi<2; yi++){
            for (int zi=0; zi<2; zi++){
              for (int wi=0; wi<2; wi++){
                pts4[vcount][0] = (xi? s : -s);
                pts4[vcount][1] = (yi? s : -s);
                pts4[vcount][2] = (zi? s : -s);
                pts4[vcount][3] = (wi? s : -s);
                vcount++;
              }
            }
          }
        }
      } else if (shape == 4){
        // 4D simplex (5 vertices)
        float a = s;
        float verts[5][4] = {
          { a, 0, 0, -a/2 },
          { -a, 0, 0, -a/2 },
          { 0, a, 0, -a/2 },
          { 0, -a, 0, -a/2 },
          { 0, 0, a, a }
        };
        for (int i=0;i<5;i++){ for (int j=0;j<4;j++) pts4[i][j]=verts[i][j]; }
        vcount = 5;
      } else {
        // 4D cross polytope (8 vertices)
        float verts[8][4] = {
          { s,0,0,0 },{ -s,0,0,0 },{ 0,s,0,0 },{ 0,-s,0,0 },
          { 0,0,s,0 },{ 0,0,-s,0 },{ 0,0,0,s },{ 0,0,0,-s }
        };
        for (int i=0;i<8;i++){ for (int j=0;j<4;j++) pts4[i][j]=verts[i][j]; }
        vcount = 8;
      }
      float ca = cos(angle), sa = sin(angle);
      float cb = cos(angle*0.7f), sb = sin(angle*0.7f);
      for (int i=0;i<vcount;i++){
        float x = pts4[i][0], y = pts4[i][1], z = pts4[i][2], w = pts4[i][3];
        // rotate in XW
        float x1 = x*ca - w*sa;
        float w1 = x*sa + w*ca;
        // rotate in YW
        float y1 = y*cb - w1*sb;
        float w2 = y*sb + w1*cb;
        pts4[i][0] = x1; pts4[i][1] = y1; pts4[i][2] = z; pts4[i][3] = w2;
      }
      int proj[16][2];
      for (int i=0;i<vcount;i++){
        float x = pts4[i][0], y = pts4[i][1], z = pts4[i][2], w = pts4[i][3];
        float d4 = 60.0f;
        float f4 = d4 / (d4 - w);
        x *= f4; y *= f4; z *= f4;
        float d3 = 80.0f;
        float f3 = d3 / (d3 - z);
        proj[i][0] = cx + (int)(x * f3);
        proj[i][1] = cy + (int)(y * f3);
      }
      // edges
      if (shape == 3){
        for (int a=0; a<vcount; a++){
          for (int b=a+1; b<vcount; b++){
            int diff = a ^ b;
            if ((diff & (diff-1)) == 0){
              M5.Display.drawLine(proj[a][0], proj[a][1], proj[b][0], proj[b][1], WHITE);
            }
          }
        }
      } else if (shape == 4){
        for (int a=0;a<vcount;a++){
          for (int b=a+1;b<vcount;b++){
            M5.Display.drawLine(proj[a][0], proj[a][1], proj[b][0], proj[b][1], WHITE);
          }
        }
      } else {
        for (int a=0;a<vcount;a++){
          for (int b=a+1;b<vcount;b++){
            if (a/2 != b/2){
              M5.Display.drawLine(proj[a][0], proj[a][1], proj[b][0], proj[b][1], WHITE);
            }
          }
        }
      }
    } else {
      // 3D shapes
      float pts3[8][3];
      int vcount = 0;
      if (shape == 0){
        float cube[8][3] = {
          {-s,-s,-s},{s,-s,-s},{s,s,-s},{-s,s,-s},
          {-s,-s,s},{s,-s,s},{s,s,s},{-s,s,s}
        };
        memcpy(pts3, cube, sizeof(cube));
        vcount = 8;
      } else if (shape == 1){
        float tet[4][3] = {
          {s,s,s},{-s,-s,s},{-s,s,-s},{s,-s,-s}
        };
        memcpy(pts3, tet, sizeof(tet));
        vcount = 4;
      } else {
        float oct[6][3] = {
          {0,0,s},{0,0,-s},{0,s,0},{0,-s,0},{s,0,0},{-s,0,0}
        };
        memcpy(pts3, oct, sizeof(oct));
        vcount = 6;
      }

      float ca = cos(angle), sa = sin(angle);
      float cb = cos(angle*0.7f), sb = sin(angle*0.7f);
      for (int i=0;i<vcount;i++){
        float x = pts3[i][0];
        float y = pts3[i][1];
        float z = pts3[i][2];
        float x1 = x*ca + z*sa;
        float z1 = -x*sa + z*ca;
        float y1 = y*cb - z1*sb;
        float z2 = y*sb + z1*cb;
        pts3[i][0] = x1; pts3[i][1] = y1; pts3[i][2] = z2;
      }
      int proj[8][2];
      for (int i=0;i<vcount;i++){
        float z = pts3[i][2] + 80.0f;
        proj[i][0] = cx + (int)(pts3[i][0] * 80.0f / z);
        proj[i][1] = cy + (int)(pts3[i][1] * 80.0f / z);
      }

      if (shape == 0){
        const int edges[][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        drawLines(vcount, proj, edges, 12);
      } else if (shape == 1){
        const int edges[][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        drawLines(vcount, proj, edges, 6);
      } else {
        const int edges[][2] = {{0,2},{0,3},{0,4},{0,5},{1,2},{1,3},{1,4},{1,5},{2,4},{2,5},{3,4},{3,5}};
        drawLines(vcount, proj, edges, 12);
      }
    }

    angle += 0.04f;
    delay(16);
  }
}

void runUniverse(){
  int mode = 0; // 0 black hole,1 neutron,2 nebula,3 galaxy,4 star system
  float angle = 0.0f;
  static bool seeded = false;
  static int16_t sx[60], sy[60];
  static uint8_t sr[60];
  if (!seeded){
    seeded = true;
    for (int i=0;i<60;i++){
      sx[i] = rand() % SCREEN_W;
      sy[i] = STATUS_H + (rand() % (SCREEN_H-STATUS_H));
      sr[i] = rand() % 3;
    }
  }
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnB.wasPressed()) { mode = (mode + 1) % 5; }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 2);
    if (mode==0) M5.Display.print("Black Hole");
    else if (mode==1) M5.Display.print("Neutron Star");
    else if (mode==2) M5.Display.print("Nebula");
    else if (mode==3) M5.Display.print("Galaxy");
    else M5.Display.print("Star System");

    int cx = SCREEN_W/2;
    int cy = STATUS_H + (SCREEN_H-STATUS_H)/2;

    // starfield
    for (int i=0;i<60;i++){
      int x = (sx[i] + (int)(angle*10*(sr[i]+1))) % SCREEN_W;
      int y = sy[i];
      uint16_t c = (sr[i]==0)?WHITE:(sr[i]==1?0x7BEF:0x39E7);
      M5.Display.drawPixel(x, y, c);
    }

    if (mode==0){
      // black hole
      for (int r=30;r>6;r-=2){
        uint16_t c = (r%6==0)?0x7BEF:0x39E7;
        M5.Display.drawCircle(cx, cy, r, c);
      }
      M5.Display.fillCircle(cx, cy, 12, BLACK);
      for (int i=0;i<12;i++){
        float a = angle*0.7f + i*0.5f;
        int x = cx + (int)(cos(a)*34);
        int y = cy + (int)(sin(a)*12);
        M5.Display.drawPixel(x, y, YELLOW);
      }
    } else if (mode==1){
      // neutron star
      M5.Display.fillCircle(cx, cy, 8, 0xFFFF);
      for (int i=0;i<10;i++){
        int x = cx + (int)(cos(angle + i*0.6f)*24);
        int y = cy + (int)(sin(angle + i*0.6f)*24);
        M5.Display.drawLine(cx, cy, x, y, 0xBDF7);
      }
    } else if (mode==2){
      // nebula
      for (int r=34;r>6;r-=3){
        uint16_t c = (r%9==0)?0x07FF:((r%6==0)?0xF81F:0x5B6F);
        M5.Display.drawCircle(cx, cy, r, c);
      }
    } else if (mode==3){
      // galaxy spiral
      for (int i=0;i<60;i++){
        float a = i*0.25f + angle;
        float r = i*0.6f;
        int x = cx + (int)(cos(a)*r);
        int y = cy + (int)(sin(a)*r*0.6f);
        M5.Display.drawPixel(x, y, WHITE);
      }
      M5.Display.fillCircle(cx, cy, 4, YELLOW);
    } else {
      // star system
      M5.Display.fillCircle(cx, cy, 6, YELLOW);
      for (int i=0;i<3;i++){
        float a = angle*(0.6f+i*0.2f);
        int r = 16 + i*8;
        int x = cx + (int)(cos(a)*r);
        int y = cy + (int)(sin(a)*r);
        uint16_t c = (i==0)?0x07E0:(i==1?0xF800:0x001F);
        M5.Display.fillCircle(x, y, 2+i%2, c);
      }
    }

    angle += 0.03f;
    delay(16);
  }
}

void runIllusion(){
  int mode = 0; // 0 grid,1 cafe wall,2 muller-lyer,3 spiral
  float t = 0.0f;
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnB.wasPressed()) { mode = (mode + 1) % 4; }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 2);
    if (mode==0) M5.Display.print("Hermann Grid");
    else if (mode==1) M5.Display.print("Cafe Wall");
    else if (mode==2) M5.Display.print("Muller-Lyer");
    else M5.Display.print("Spiral");

    int y0 = STATUS_H + 12;
    if (mode==0){
      for (int y=y0; y<SCREEN_H; y+=18){
        for (int x=0; x<SCREEN_W; x+=18){
          M5.Display.fillRect(x, y, 16, 16, WHITE);
        }
      }
    } else if (mode==1){
      for (int y=y0; y<SCREEN_H; y+=14){
        for (int x=0; x<SCREEN_W; x+=28){
          int ox = (y/14)%2 ? 6 : 0;
          M5.Display.fillRect(x+ox, y, 14, 8, WHITE);
        }
        M5.Display.drawFastHLine(0, y+8, SCREEN_W, 0x7BEF);
      }
    } else if (mode==2){
      int cx = SCREEN_W/2;
      int cy = y0 + (SCREEN_H - y0)/2;
      M5.Display.drawLine(20, cy-20, 100, cy-20, WHITE);
      M5.Display.drawLine(140, cy+20, 220, cy+20, WHITE);
      // arrows
      M5.Display.drawLine(20, cy-20, 30, cy-30, WHITE);
      M5.Display.drawLine(20, cy-20, 30, cy-10, WHITE);
      M5.Display.drawLine(100, cy-20, 90, cy-30, WHITE);
      M5.Display.drawLine(100, cy-20, 90, cy-10, WHITE);
      M5.Display.drawLine(140, cy+20, 150, cy+10, WHITE);
      M5.Display.drawLine(140, cy+20, 150, cy+30, WHITE);
      M5.Display.drawLine(220, cy+20, 210, cy+10, WHITE);
      M5.Display.drawLine(220, cy+20, 210, cy+30, WHITE);
    } else {
      int cx = SCREEN_W/2;
      int cy = y0 + (SCREEN_H - y0)/2;
      for (int i=0;i<120;i++){
        float a = i*0.2f + t;
        float r = i*0.5f;
        int x = cx + (int)(cos(a)*r);
        int y = cy + (int)(sin(a)*r);
        M5.Display.drawPixel(x, y, (i%2)?WHITE:0x7BEF);
      }
      t += 0.02f;
    }
    delay(16);
  }
}
