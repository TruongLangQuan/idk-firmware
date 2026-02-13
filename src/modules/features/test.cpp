#include "modules/features/test.h"
#include "core/ui.h"
#include <tinyexpr.h>
#include <math.h>
#include <ctype.h>
#include <cstring>

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

  // Remove old grid, will draw new one below

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

  // Axis range -50 to 50
  const int axisMin = -50;
  const int axisMax = 50;
  const int axisRange = axisMax - axisMin;
  
  // Grid lines - every 10 units for better visibility
  for (int x=axisMin; x<=axisMax; x+=10){
    int px = (int)((x - axisMin) * (SCREEN_W-1) / (double)axisRange);
    if (px >= 0 && px < SCREEN_W){
      // Draw lighter grid lines for minor, darker for major
      uint16_t gridColor = (x % 50 == 0) ? 0x39E7 : 0x18C3; // Darker for major lines (every 50)
      M5.Display.drawFastVLine(px, STATUS_H, SCREEN_H-STATUS_H, gridColor);
    }
  }
  for (int y=axisMin; y<=axisMax; y+=10){
    int py = cy - (int)(y * ((SCREEN_H-STATUS_H-1) / (double)axisRange));
    if (py >= STATUS_H && py < SCREEN_H){
      // Draw lighter grid lines for minor, darker for major
      uint16_t gridColor = (y % 50 == 0) ? 0x39E7 : 0x18C3; // Darker for major lines (every 50)
      M5.Display.drawFastHLine(0, py, SCREEN_W, gridColor);
    }
  }
  
  // No axis labels (removed as requested)

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

  // Plot with improved resolution and range
  double yRange = SCREEN_H - STATUS_H - 1;
  int prevPy = -1;
  for (int px=0; px<SCREEN_W; px++){
    double x = ((double)px / (SCREEN_W-1)) * axisRange + axisMin;
    xval = x;
    double y = te_eval(e);
    if (y < axisMin || y > axisMax) {
      prevPy = -1;
      continue;
    }
    // Convert y value to screen coordinate
    // cy is at y=0, so y values map: y=axisMax -> top, y=axisMin -> bottom
    int py = cy - (int)(y * (yRange / (double)axisRange));
    if (py >= STATUS_H && py < SCREEN_H) {
      // Draw pixel
      M5.Display.drawPixel(px, py, YELLOW);
      // Draw line from previous point if valid
      if (prevPy >= STATUS_H && prevPy < SCREEN_H) {
        M5.Display.drawLine(px-1, prevPy, px, py, YELLOW);
      }
      prevPy = py;
    } else {
      prevPy = -1;
    }
  }
  te_free(e);
}

void runCube(){
  float angle = 0.0f;
  int shape = 0; // 0 cube, 1 tetra, 2 octa, 3 prism, 4 pyramid, 5 icosahedron, 6 dodecahedron, 7 torus, 8 sphere, 9 cylinder, 10 cone, 11 4D Cube, 12 4D Simplex, 13 4D Cross
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnB.wasPressed()) { shape = (shape + 1) % 14; }

    M5.Display.fillScreen(BLACK);
    drawStatus();
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 2);
    if (shape == 0) M5.Display.print("Cube");
    else if (shape == 1) M5.Display.print("Tetra");
    else if (shape == 2) M5.Display.print("Octa");
    else if (shape == 3) M5.Display.print("Prism");
    else if (shape == 4) M5.Display.print("Pyramid");
    else if (shape == 5) M5.Display.print("Icosa");
    else if (shape == 6) M5.Display.print("Dodeca");
    else if (shape == 7) M5.Display.print("Torus");
    else if (shape == 8) M5.Display.print("Sphere");
    else if (shape == 9) M5.Display.print("Cylinder");
    else if (shape == 10) M5.Display.print("Cone");
    else if (shape == 11) M5.Display.print("4D Cube");
    else if (shape == 12) M5.Display.print("4D Simplex");
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

    if (shape >= 11){
      // 4D tesseract
      float pts4[16][4];
      int vcount = 0;
      if (shape == 11){
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
      } else if (shape == 12){
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
      if (shape == 11){
        for (int a=0; a<vcount; a++){
          for (int b=a+1; b<vcount; b++){
            int diff = a ^ b;
            if ((diff & (diff-1)) == 0){
              M5.Display.drawLine(proj[a][0], proj[a][1], proj[b][0], proj[b][1], WHITE);
            }
          }
        }
      } else if (shape == 12){
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
      float pts3[12][3]; // Reduced size to save RAM
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
      } else if (shape == 2) {
        float oct[6][3] = {
          {0,0,s},{0,0,-s},{0,s,0},{0,-s,0},{s,0,0},{-s,0,0}
        };
        memcpy(pts3, oct, sizeof(oct));
        vcount = 6;
      } else if (shape == 3) {
        // triangular prism (6 vertices)
        float prism[6][3] = {
          {-s,-s, -s},{s,-s,-s},{0,s,-s},
          {-s,-s,s},{s,-s,s},{0,s,s}
        };
        memcpy(pts3, prism, sizeof(prism));
        vcount = 6;
      } else if (shape == 4) {
        // square pyramid (5 vertices)
        float pyr[5][3] = {
          {-s,-s,-s},{s,-s,-s},{s,s,-s},{-s,s,-s},{0,0,s}
        };
        for (int i=0;i<5;i++) for (int j=0;j<3;j++) pts3[i][j] = pyr[i][j];
        vcount = 5;
      } else if (shape == 5) {
        // icosahedron simplified (8 vertices instead of 12)
        float icosa[8][3] = {
          {-s, s, 0}, {s, s, 0}, {-s, -s, 0}, {s, -s, 0},
          {0, -s, s}, {0, s, s}, {0, -s, -s}, {0, s, -s}
        };
        for (int i=0;i<8;i++) for (int j=0;j<3;j++) pts3[i][j] = icosa[i][j];
        vcount = 8;
      } else if (shape == 6) {
        // dodecahedron simplified (12 vertices instead of 20)
        float t = (1.0f + sqrtf(5.0f)) / 2.0f;
        float dodeca[12][3] = {
          {s, s, s}, {s, s, -s}, {s, -s, s}, {s, -s, -s},
          {-s, s, s}, {-s, s, -s}, {-s, -s, s}, {-s, -s, -s},
          {0, s/t, t*s}, {0, -s/t, t*s}, {t*s, 0, s/t}, {-t*s, 0, s/t}
        };
        for (int i=0;i<12;i++) for (int j=0;j<3;j++) pts3[i][j] = dodeca[i][j];
        vcount = 12;
      } else if (shape == 7) {
        // torus simplified (8 vertices)
        int segments = 4;
        for (int i=0;i<segments;i++){
          float a1 = i * 2.0f * M_PI / segments;
          for (int j=0;j<2;j++){
            float a2 = j * 2.0f * M_PI / 2.0f;
            float r = s * 0.6f;
            float R = s * 1.2f;
            pts3[i*2+j][0] = (R + r*cosf(a2)) * cosf(a1);
            pts3[i*2+j][1] = (R + r*cosf(a2)) * sinf(a1);
            pts3[i*2+j][2] = r * sinf(a2);
          }
        }
        vcount = segments * 2;
      } else if (shape == 8) {
        // sphere approximation simplified (8 vertices)
        int segments = 4;
        for (int i=0;i<segments;i++){
          float lat = (i - segments/2.0f) * M_PI / segments;
          for (int j=0;j<2;j++){
            float lon = j * M_PI;
            pts3[i*2+j][0] = s * cosf(lat) * cosf(lon);
            pts3[i*2+j][1] = s * cosf(lat) * sinf(lon);
            pts3[i*2+j][2] = s * sinf(lat);
          }
        }
        vcount = segments * 2;
      } else if (shape == 9) {
        // cylinder simplified (8 vertices)
        int segments = 4;
        for (int i=0;i<segments;i++){
          float a = i * 2.0f * M_PI / segments;
          pts3[i*2][0] = s * cosf(a);
          pts3[i*2][1] = s * sinf(a);
          pts3[i*2][2] = -s;
          pts3[i*2+1][0] = s * cosf(a);
          pts3[i*2+1][1] = s * sinf(a);
          pts3[i*2+1][2] = s;
        }
        vcount = segments * 2;
      } else if (shape == 10) {
        // cone simplified (5 vertices)
        pts3[0][0] = 0; pts3[0][1] = 0; pts3[0][2] = s*1.5f; // apex
        int segments = 4;
        for (int i=0;i<segments;i++){
          float a = i * 2.0f * M_PI / segments;
          pts3[i+1][0] = s * cosf(a);
          pts3[i+1][1] = s * sinf(a);
          pts3[i+1][2] = -s;
        }
        vcount = segments + 1;
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
      int proj[12][2];
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
      } else if (shape == 2){
        const int edges[][2] = {{0,2},{0,3},{0,4},{0,5},{1,2},{1,3},{1,4},{1,5},{2,4},{2,5},{3,4},{3,5}};
        drawLines(vcount, proj, edges, 12);
      } else if (shape == 3){
        const int edges[][2] = {{0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5}};
        drawLines(vcount, proj, edges, 9);
      } else if (shape == 4){
        const int edges[][2] = {{0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4}};
        drawLines(vcount, proj, edges, 8);
      } else if (shape == 5){
        // icosahedron edges (simplified - connect nearby vertices)
        for (int i=0;i<vcount;i++){
          for (int j=i+1;j<vcount;j++){
            float dist = sqrtf(powf(pts3[i][0]-pts3[j][0],2) + powf(pts3[i][1]-pts3[j][1],2) + powf(pts3[i][2]-pts3[j][2],2));
            if (dist < s*1.8f) M5.Display.drawLine(proj[i][0], proj[i][1], proj[j][0], proj[j][1], WHITE);
          }
        }
      } else if (shape == 6){
        // dodecahedron edges (connect nearby vertices)
        for (int i=0;i<vcount;i++){
          for (int j=i+1;j<vcount;j++){
            float dist = sqrtf(powf(pts3[i][0]-pts3[j][0],2) + powf(pts3[i][1]-pts3[j][1],2) + powf(pts3[i][2]-pts3[j][2],2));
            if (dist < s*2.2f) M5.Display.drawLine(proj[i][0], proj[i][1], proj[j][0], proj[j][1], WHITE);
          }
        }
      } else if (shape == 7){
        // torus edges
        for (int i=0;i<vcount-1;i++){
          M5.Display.drawLine(proj[i][0], proj[i][1], proj[i+1][0], proj[i+1][1], WHITE);
        }
        M5.Display.drawLine(proj[vcount-1][0], proj[vcount-1][1], proj[0][0], proj[0][1], WHITE);
      } else if (shape == 8){
        // sphere edges (connect nearby vertices)
        for (int i=0;i<vcount;i++){
          for (int j=i+1;j<vcount;j++){
            float dist = sqrtf(powf(pts3[i][0]-pts3[j][0],2) + powf(pts3[i][1]-pts3[j][1],2) + powf(pts3[i][2]-pts3[j][2],2));
            if (dist < s*1.5f) M5.Display.drawLine(proj[i][0], proj[i][1], proj[j][0], proj[j][1], WHITE);
          }
        }
      } else if (shape == 9){
        // cylinder edges
        for (int i=0;i<vcount/2-1;i++){
          M5.Display.drawLine(proj[i*2][0], proj[i*2][1], proj[(i+1)*2][0], proj[(i+1)*2][1], WHITE);
          M5.Display.drawLine(proj[i*2+1][0], proj[i*2+1][1], proj[(i+1)*2+1][0], proj[(i+1)*2+1][1], WHITE);
          M5.Display.drawLine(proj[i*2][0], proj[i*2][1], proj[i*2+1][0], proj[i*2+1][1], WHITE);
        }
        M5.Display.drawLine(proj[(vcount/2-1)*2][0], proj[(vcount/2-1)*2][1], proj[0][0], proj[0][1], WHITE);
        M5.Display.drawLine(proj[(vcount/2-1)*2+1][0], proj[(vcount/2-1)*2+1][1], proj[1][0], proj[1][1], WHITE);
      } else if (shape == 10){
        // cone edges
        for (int i=1;i<vcount;i++){
          M5.Display.drawLine(proj[0][0], proj[0][1], proj[i][0], proj[i][1], WHITE);
          if (i < vcount-1) M5.Display.drawLine(proj[i][0], proj[i][1], proj[i+1][0], proj[i+1][1], WHITE);
        }
        M5.Display.drawLine(proj[vcount-1][0], proj[vcount-1][1], proj[1][0], proj[1][1], WHITE);
      }
    }

    angle += 0.04f;
    delay(16);
  }
}

void runUniverse(){
  int mode = 0; // 0 black hole,1 neutron,2 nebula,3 galaxy,4 star system
  float angle = 0.0f;

  auto project3D = [&](float x, float y, float z, int cx, int cy, int &sx, int &sy){
    float dz = z + 96.0f;
    sx = cx + (int)(x * 92.0f / dz);
    sy = cy + (int)(y * 92.0f / dz);
  };

  auto rotY = [&](float &x, float &z, float a){
    float ca = cos(a), sa = sin(a);
    float nx = x * ca + z * sa;
    float nz = -x * sa + z * ca;
    x = nx; z = nz;
  };

  auto rotX = [&](float &y, float &z, float a){
    float ca = cos(a), sa = sin(a);
    float ny = y * ca - z * sa;
    float nz = y * sa + z * ca;
    y = ny; z = nz;
  };

  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()) break;
    if (M5.BtnB.wasPressed()) mode = (mode + 1) % 5;

    M5.Display.fillScreen(BLACK);
    drawStatus();

    for (int y=STATUS_H + 8; y<SCREEN_H; y+=12) M5.Display.drawFastHLine(0, y, SCREEN_W, 0x1082);
    for (int x=0; x<SCREEN_W; x+=12) M5.Display.drawFastVLine(x, STATUS_H, SCREEN_H-STATUS_H, 0x1082);
    for (int y=STATUS_H + 14; y<SCREEN_H; y+=24) M5.Display.drawFastHLine(0, y, SCREEN_W, 0x2104);
    for (int x=0; x<SCREEN_W; x+=24) M5.Display.drawFastVLine(x, STATUS_H, SCREEN_H-STATUS_H, 0x2104);

    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, STATUS_H + 2);
    if (mode==0) M5.Display.print("Black Hole");
    else if (mode==1) M5.Display.print("Neutron Star");
    else if (mode==2) M5.Display.print("Nebula");
    else if (mode==3) M5.Display.print("Galaxy");
    else M5.Display.print("Star System");

    int cx = SCREEN_W / 2;
    int cy = STATUS_H + (SCREEN_H - STATUS_H) / 2;

    for (int i=0; i<56; i++){
      float a = i * 0.28f + angle * 0.18f;
      float r = 18.0f + (i % 14) * 6.5f;
      float x = cos(a) * r;
      float y = sin(a * 1.25f) * (8.0f + (i % 6) * 2.5f);
      float z = 18.0f + (i % 16) * 4.8f;
      int sx, sy;
      project3D(x, y, z, cx, cy, sx, sy);
      if (sx>=0 && sx<SCREEN_W && sy>=STATUS_H && sy<SCREEN_H){
        if ((i & 3) == 0) M5.Display.fillCircle(sx, sy, 1, WHITE);
        else M5.Display.drawPixel(sx, sy, 0xCE79);
      }
    }

    if (mode == 0){
      int seg = 48;
      int prevA[2] = {0,0}, prevB[2] = {0,0}, prevC[2] = {0,0};
      for (int i=0;i<=seg;i++){
        float t = (2.0f * M_PI * i) / seg;
        float x = cos(t) * 28.0f;
        float y = sin(t) * 8.5f;
        float z = sin(t + angle) * 9.0f;
        rotY(x, z, angle * 0.8f);
        rotX(y, z, angle * 0.22f);
        int p0x, p0y;
        project3D(x, y, z, cx, cy, p0x, p0y);
        if (i > 0) M5.Display.drawLine(prevA[0], prevA[1], p0x, p0y, 0xEF7D);
        prevA[0] = p0x; prevA[1] = p0y;

        float x2 = cos(t + M_PI) * 26.0f;
        float y2 = sin(t + M_PI) * 8.0f;
        float z2 = sin(t + M_PI + angle) * 10.0f;
        rotY(x2, z2, angle * 0.8f);
        rotX(y2, z2, angle * 0.22f);
        int p1x, p1y;
        project3D(x2, y2, z2, cx, cy, p1x, p1y);
        if (i > 0) M5.Display.drawLine(prevB[0], prevB[1], p1x, p1y, 0xDEFB);
        prevB[0] = p1x; prevB[1] = p1y;

        float x3 = cos(t + angle * 0.7f) * 17.0f;
        float y3 = sin(t + angle * 0.7f) * 4.5f;
        float z3 = cos(t * 1.5f + angle) * 6.0f;
        rotY(x3, z3, angle * 1.1f);
        int p2x, p2y;
        project3D(x3, y3, z3, cx, cy, p2x, p2y);
        if (i > 0) M5.Display.drawLine(prevC[0], prevC[1], p2x, p2y, 0x7BEF);
        prevC[0] = p2x; prevC[1] = p2y;
      }
      for (int j=0;j<18;j++){
        float a = angle * 1.4f + j * (2.0f * M_PI / 18.0f);
        int px = cx + (int)(cos(a) * 34.0f);
        int py = cy + (int)(sin(a) * 6.0f);
        M5.Display.drawPixel(px, py, 0xFFE0);
      }
      M5.Display.drawCircle(cx, cy, 8, WHITE);
      M5.Display.fillCircle(cx, cy, 6, BLACK);
    } else if (mode == 1){
      int seg = 18;
      for (int i=0;i<seg;i++){
        float a = i * (2.0f * M_PI / seg) + angle;
        float x = cos(a) * 23.0f;
        float y = sin(a) * 23.0f;
        float z = sin(a * 2.2f) * 9.0f;
        int sx, sy;
        project3D(x, y, z, cx, cy, sx, sy);
        M5.Display.drawLine(cx, cy, sx, sy, 0xCE79);
      }
      for (int lat=-2; lat<=2; lat++){
        int prev[2] = {0,0};
        for (int i=0;i<=24;i++){
          float t = (2.0f * M_PI * i) / 24.0f;
          float x = cos(t) * (10.0f - fabs(lat) * 1.8f);
          float y = sin(t) * (10.0f - fabs(lat) * 1.8f);
          float z = lat * 3.2f;
          rotY(x, z, angle * 0.7f);
          rotX(y, z, angle * 0.45f);
          int sx, sy;
          project3D(x, y, z, cx, cy, sx, sy);
          if (i > 0) M5.Display.drawLine(prev[0], prev[1], sx, sy, WHITE);
          prev[0] = sx; prev[1] = sy;
        }
      }
      M5.Display.drawCircle(cx, cy, 10, WHITE);
    } else if (mode == 2){
      int prevCloud[3][2] = {{0,0},{0,0},{0,0}};
      for (int i=0;i<66;i++){
        float a = i * 0.32f + angle * 0.55f;
        float r = 9.0f + (i % 12) * 2.3f;
        float x = cos(a) * r;
        float y = sin(a * 1.45f) * (r * 0.55f);
        float z = cos(a * 0.9f) * 11.0f;
        rotY(x, z, angle * 0.22f);
        int sx, sy;
        project3D(x, y, z, cx, cy, sx, sy);
        if (sx>=0 && sx<SCREEN_W && sy>=STATUS_H && sy<SCREEN_H) M5.Display.drawPixel(sx, sy, 0xE73C);
        int lane = i % 3;
        if (i >= 3) M5.Display.drawLine(prevCloud[lane][0], prevCloud[lane][1], sx, sy, 0xC638);
        prevCloud[lane][0] = sx;
        prevCloud[lane][1] = sy;
      }
    } else if (mode == 3){
      for (int arm=0; arm<3; arm++){
        int prev[2] = {0,0};
        for (int i=0;i<52;i++){
          float a = i * 0.28f + angle + arm * 2.094f;
          float r = 1.5f + i * 0.68f;
          float x = cos(a) * r;
          float y = sin(a) * r * 0.42f;
          float z = sin(a * 0.72f + arm) * 6.5f;
          int sx, sy;
          project3D(x, y, z, cx, cy, sx, sy);
          if (i > 0) M5.Display.drawLine(prev[0], prev[1], sx, sy, (arm==0)?WHITE:(arm==1?0xD6BA:0xBDF7));
          prev[0] = sx; prev[1] = sy;
          if ((i % 8) == 0) M5.Display.drawPixel(sx, sy, WHITE);
        }
      }
      M5.Display.drawCircle(cx, cy, 4, WHITE);
    } else {
      M5.Display.drawCircle(cx, cy, 7, WHITE);
      M5.Display.fillCircle(cx, cy, 2, WHITE);
      for (int ring=0; ring<3; ring++){
        float rr = 14.0f + ring * 10.0f;
        int seg = 32;
        int prev[2] = {0,0};
        for (int i=0;i<=seg;i++){
          float a = i * (2.0f * M_PI / seg) + angle * (0.5f + ring*0.18f);
          float x = cos(a) * rr;
          float y = sin(a) * rr * 0.6f;
          float z = sin(a + ring) * 6.0f;
          int sx, sy;
          project3D(x, y, z, cx, cy, sx, sy);
          if (i > 0) M5.Display.drawLine(prev[0], prev[1], sx, sy, 0xD69A);
          prev[0] = sx; prev[1] = sy;
        }
        float pa = angle * (0.8f + ring*0.23f);
        float px = cos(pa) * rr;
        float py = sin(pa) * rr * 0.6f;
        float pz = sin(pa + ring) * 6.0f;
        int psx, psy;
        project3D(px, py, pz, cx, cy, psx, psy);
        M5.Display.fillCircle(psx, psy, 2, WHITE);
        if (ring == 2){
          int rs = 10;
          int prv[2] = {0,0};
          for (int i=0;i<=rs;i++){
            float a = i * (2.0f * M_PI / rs) + angle * 0.35f;
            float x = px + cos(a) * 3.5f;
            float y = py + sin(a) * 1.8f;
            float z = pz + sin(a * 1.2f) * 1.0f;
            int rx, ry;
            project3D(x, y, z, cx, cy, rx, ry);
            if (i > 0) M5.Display.drawLine(prv[0], prv[1], rx, ry, WHITE);
            prv[0] = rx; prv[1] = ry;
          }
        }
      }
    }

    angle += 0.04f;
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
      for (int i=0;i<80;i++){
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

void runDraw(){
  // Draw mode: pixel editor - use even smaller resolution to save RAM
  // Use 80x45 resolution (1/3 resolution) to save memory
  const int DRAW_W = SCREEN_W / 3;
  const int DRAW_H = SCREEN_H / 3;
  static uint8_t canvas[(DRAW_W * DRAW_H + 7) / 8];
  static bool initialized = false;
  if (!initialized){
    memset(canvas, 0, sizeof(canvas));
    initialized = true;
  }
  
  int px = DRAW_W/2;
  int py = DRAW_H/2;
  bool exitDraw = false;
  
  auto setPixel = [&](int x, int y, bool on){
    if (x < 0 || x >= DRAW_W || y < 0 || y >= DRAW_H) return;
    int idx = y * DRAW_W + x;
    int byteIdx = idx / 8;
    int bitIdx = idx % 8;
    if (on) canvas[byteIdx] |= (1 << bitIdx);
    else canvas[byteIdx] &= ~(1 << bitIdx);
  };
  
  auto getPixel = [&](int x, int y) -> bool {
    if (x < 0 || x >= DRAW_W || y < 0 || y >= DRAW_H) return false;
    int idx = y * DRAW_W + x;
    int byteIdx = idx / 8;
    int bitIdx = idx % 8;
    return (canvas[byteIdx] & (1 << bitIdx)) != 0;
  };
  
  while (!exitDraw){
    M5.update();
    
    // Exit: M5 + Prev pressed together
    if (M5.BtnA.isPressed() && M5.BtnPWR.isPressed()){
      exitDraw = true;
      // Reset canvas
      memset(canvas, 0, sizeof(canvas));
      break;
    }
    
    // Place pixel: M5 pressed (alone)
    if (M5.BtnA.wasPressed() && !M5.BtnB.isPressed()){
      setPixel(px, py, true);
    }
    
    // Erase pixel: M5 + Next pressed together
    if (M5.BtnA.isPressed() && M5.BtnB.wasPressed() && !M5.BtnA.wasPressed()){
      setPixel(px, py, false);
    }
    
    // Handle Next button (BtnB)
    static bool nextWasPressed = false;
    static uint32_t nextPressTime = 0;
    static uint32_t lastMoveRight = 0;
    static uint32_t lastMoveLeft = 0;
    
    if (M5.BtnB.wasPressed()){
      nextWasPressed = true;
      nextPressTime = millis();
      // Move right on press
      px++;
      if (px >= DRAW_W) px = DRAW_W - 1;
    }
    
    if (M5.BtnB.isPressed() && nextWasPressed){
      // If held for more than 200ms, start moving left
      if (millis() - nextPressTime > 200){
        if (millis() - lastMoveLeft > 100){
          px--;
          if (px < 0) px = 0;
          lastMoveLeft = millis();
        }
      }
    } else {
      nextWasPressed = false;
    }
    
    // Handle Prev button (BtnPWR)
    static bool prevWasPressed = false;
    static uint32_t prevPressTime = 0;
    static uint32_t lastMoveDown = 0;
    static uint32_t lastMoveUp = 0;
    
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()){
      prevWasPressed = true;
      prevPressTime = millis();
      // Move down on press
      py++;
      if (py >= DRAW_H) py = DRAW_H - 1;
    }
    
    if (M5.BtnPWR.isPressed() && !M5.BtnA.isPressed() && prevWasPressed){
      // If held for more than 200ms, start moving up
      if (millis() - prevPressTime > 200){
        if (millis() - lastMoveUp > 100){
          py--;
          if (py < 0) py = 0;
          lastMoveUp = millis();
        }
      }
    } else {
      prevWasPressed = false;
    }
    
    // Draw canvas - scale up from reduced resolution
    M5.Display.fillScreen(BLACK);
    for (int x=0;x<DRAW_W;x++){
      for (int y=0;y<DRAW_H;y++){
        if (getPixel(x, y)){
          // Scale up 3x3 pixels
          int sx = x * 3;
          int sy = y * 3;
          M5.Display.fillRect(sx, sy, 3, 3, WHITE);
        }
      }
    }
    
    // Draw cursor - scale up
    if (px >= 0 && px < DRAW_W && py >= 0 && py < DRAW_H){
      int sx = px * 3;
      int sy = py * 3;
      M5.Display.drawRect(sx-1, sy-1, 5, 5, CYAN);
    }
    
    delay(10);
  }
}
