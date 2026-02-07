#include "core/input.h"
#include "core/ui.h"

bool textInput(const char* title, String &out, int maxLen, bool mask){
  const char* row1 = "1234567890-=";
  const char* row2 = "qwertyuiop[]";
  const char* row3 = "asdfghjkl;'";
  const char* row4 = "zxcvbnm,./";
  const char* row1s = "!@#$%^&*()_+";
  const char* row2s = "QWERTYUIOP{}";
  const char* row3s = "ASDFGHJKL:\"";
  const char* row4s = "ZXCVBNM<>?";

  const char* topKeys[] = {"OK","CAP","DEL","SPACE","BACK"};
  const int topCount = 5;

  int cursor = 0;
  bool caps = false;
  bool symbol = false;
  String value = out;

  auto keyAt = [&](int idx, String &label, bool &isTop){
    if (idx < topCount){
      label = topKeys[idx];
      isTop = true;
      return;
    }
    isTop = false;
    int k = idx - topCount;
    const char* r1 = symbol ? row1s : row1;
    const char* r2 = symbol ? row2s : (caps ? row2s : row2);
    const char* r3 = symbol ? row3s : (caps ? row3s : row3);
    const char* r4 = symbol ? row4s : (caps ? row4s : row4);
    int len1 = strlen(r1);
    int len2 = strlen(r2);
    int len3 = strlen(r3);
    int len4 = strlen(r4);
    if (k < len1){ label = String(r1[k]); return; }
    k -= len1;
    if (k < len2){ label = String(r2[k]); return; }
    k -= len2;
    if (k < len3){ label = String(r3[k]); return; }
    k -= len3;
    if (k < len4){ label = String(r4[k]); return; }
    label = "";
  };

  auto keyCount = [&](){
    return topCount + (int)strlen(row1) + (int)strlen(row2) + (int)strlen(row3) + (int)strlen(row4);
  };

  auto draw = [&](){
    M5.Display.fillScreen(COLOR_BG);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 2);
    M5.Display.print(title);

    // input box
    M5.Display.drawRect(4, STATUS_H + 14, SCREEN_W-8, 18, COLOR_DIM);
    M5.Display.setCursor(8, STATUS_H + 18);
    String shown = value;
    if (mask) {
      shown = "";
      for (int i=0;i<value.length();i++) shown += "*";
    }
    M5.Display.print(shown);
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(190, STATUS_H + 2);
    M5.Display.printf("%d/%d", (int)value.length(), maxLen);

    int y0 = STATUS_H + 34;
    int rowH = 16;
    int keyW = 20;
    int padX = 4;

    // draw top row with more spacing
    int x = padX;
    for (int i=0;i<topCount;i++){
      int w = 30; // OK
      if (i==1) w = 36;       // CAP
      if (i==2) w = 36;       // DEL
      if (i==3) w = 70;       // SPACE
      if (i==4) w = 46;       // BACK
      int idx = i;
      bool sel = (cursor == idx);
      M5.Display.drawRect(x, y0, w, 12, sel ? WHITE : COLOR_DIM);
      M5.Display.setTextColor(sel ? WHITE : COLOR_DIM);
      M5.Display.setCursor(x+4, y0+2);
      M5.Display.print(topKeys[i]);
      x += w + 4;
    }

    // rows
    const char* r1 = symbol ? row1s : row1;
    const char* r2 = symbol ? row2s : (caps ? row2s : row2);
    const char* r3 = symbol ? row3s : (caps ? row3s : row3);
    const char* r4 = symbol ? row4s : (caps ? row4s : row4);
    const char* rows[] = {r1,r2,r3,r4};
    int rowLens[] = {(int)strlen(r1),(int)strlen(r2),(int)strlen(r3),(int)strlen(r4)};

    int base = topCount;
    for (int r=0;r<4;r++){
      int y = y0 + 16 + r*rowH;
      for (int c=0;c<rowLens[r];c++){
        int idx = base + c;
        bool sel = (cursor == idx);
        int x = padX + c*keyW;
        M5.Display.drawRect(x, y, keyW-2, 12, sel ? WHITE : COLOR_DIM);
        M5.Display.setTextColor(sel ? WHITE : COLOR_DIM);
        M5.Display.setCursor(x+5, y+2);
        M5.Display.print(rows[r][c]);
      }
      base += rowLens[r];
    }

    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.print("Prev:Down Hold:Up  Next:Right Hold:Left");
  };

  int total = keyCount();
  draw();
  bool pwrDown = false;
  bool pwrLong = false;
  uint32_t pwrStart = 0;
  bool bDown = false;
  bool bLong = false;
  uint32_t bStart = 0;
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()){
      pwrDown = true;
      pwrLong = false;
      pwrStart = millis();
    }
    if (pwrDown && !pwrLong && (millis() - pwrStart > 500)){
      cursor = (cursor - 1 + total) % total;
      pwrLong = true;
      draw();
    }
    if (M5.BtnPWR.wasReleased()){
      if (!pwrLong){
        cursor = (cursor + 1) % total;
        draw();
      }
      pwrDown = false;
    }

    if (M5.BtnB.wasPressed()){
      bDown = true;
      bLong = false;
      bStart = millis();
    }
    if (bDown && !bLong && (millis() - bStart > 500)){
      cursor = (cursor - 1 + total) % total;
      bLong = true;
      draw();
    }
    if (M5.BtnB.wasReleased()){
      if (!bLong){
        cursor = (cursor + 1) % total;
        draw();
      }
      bDown = false;
    }
    if (M5.BtnA.wasPressed()){
      if (cursor < topCount){
        if (cursor == 0){ out = value; return true; }          // OK
        if (cursor == 1){                                       // CAP: cycle lower -> CAPS -> SYM
          if (!caps && !symbol) { caps = true; }
          else if (caps && !symbol) { caps = false; symbol = true; }
          else { symbol = false; }
          draw();
        }
        if (cursor == 2){ if (value.length()) value.remove(value.length()-1); draw(); } // DEL
        if (cursor == 3){ if ((int)value.length() < maxLen) value += " "; draw(); }    // SPACE
        if (cursor == 4){ return false; }                       // BACK
      } else {
        String label; bool isTop = false;
        keyAt(cursor, label, isTop);
        if (!label.isEmpty()){
          if ((int)value.length() < maxLen) value += label;
        }
        draw();
      }
    }
    delay(5);
  }
}
