#include "modules/test.h"
#include "core/ui.h"
#include <tinyexpr.h>
#include <math.h>

void drawPlot(const String &expr){
  M5.Display.fillScreen(BLACK);
  drawStatus();

  for (int x=0;x<SCREEN_W;x+=24) M5.Display.drawFastVLine(x, STATUS_H, SCREEN_H-STATUS_H, 0x39E7);
  for (int y=STATUS_H;y<SCREEN_H;y+=22) M5.Display.drawFastHLine(0, y, SCREEN_W, 0x39E7);

  int cx = SCREEN_W/2;
  int cy = STATUS_H + (SCREEN_H-STATUS_H)/2;
  M5.Display.drawFastVLine(cx, STATUS_H, SCREEN_H-STATUS_H, WHITE);
  M5.Display.drawFastHLine(0, cy, SCREEN_W, WHITE);

  double xval = 0;
  te_variable vars[] = { {"x", &xval} };
  int err = 0;
  te_expr *e = te_compile(expr.c_str(), vars, 1, &err);
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
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;

    M5.Display.fillScreen(BLACK);
    drawStatus();

    float s = 20.0f;
    float pts[8][3] = {
      {-s,-s,-s},{s,-s,-s},{s,s,-s},{-s,s,-s},
      {-s,-s,s},{s,-s,s},{s,s,s},{-s,s,s}
    };

    float ca = cos(angle), sa = sin(angle);
    float cb = cos(angle*0.7f), sb = sin(angle*0.7f);
    for (int i=0;i<8;i++){
      float x = pts[i][0];
      float y = pts[i][1];
      float z = pts[i][2];
      float x1 = x*ca + z*sa;
      float z1 = -x*sa + z*ca;
      float y1 = y*cb - z1*sb;
      float z2 = y*sb + z1*cb;
      pts[i][0] = x1; pts[i][1] = y1; pts[i][2] = z2;
    }

    int cx = SCREEN_W/2;
    int cy = STATUS_H + (SCREEN_H-STATUS_H)/2;
    int proj[8][2];
    for (int i=0;i<8;i++){
      float z = pts[i][2] + 80.0f;
      proj[i][0] = cx + (int)(pts[i][0] * 80.0f / z);
      proj[i][1] = cy + (int)(pts[i][1] * 80.0f / z);
    }

    auto line = [&](int a, int b){
      M5.Display.drawLine(proj[a][0], proj[a][1], proj[b][0], proj[b][1], WHITE);
    };
    line(0,1); line(1,2); line(2,3); line(3,0);
    line(4,5); line(5,6); line(6,7); line(7,4);
    line(0,4); line(1,5); line(2,6); line(3,7);

    angle += 0.04f;
    delay(16);
  }
}
