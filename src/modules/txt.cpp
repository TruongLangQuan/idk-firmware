#include "modules/txt.h"
#include "core/ui.h"
#include <vector>
#include <lgfx/v1/misc/DataWrapper.hpp>

struct SPIFFSFontWrapper : public lgfx::v1::DataWrapper {
  File f;
  bool open(const char* path) override {
    f = SPIFFS.open(path, "r");
    return (bool)f;
  }
  int read(uint8_t *buf, uint32_t len) override { return f.read(buf, len); }
  void skip(int32_t offset) override { f.seek(f.position() + offset); }
  bool seek(uint32_t offset) override { return f.seek(offset); }
  void close(void) override { if (f) f.close(); }
  int32_t tell(void) override { return (int32_t)f.position(); }
};

static void loadLines(fs::FS &fs, const char* path, std::vector<String> &lines){
  lines.clear();
  File f = fs.open(path, "r");
  if (!f) return;
  while (f.available() && lines.size() < 200){
    String line = f.readStringUntil('\n');
    line.replace("\r", "");
    lines.push_back(line);
  }
  f.close();
}

void showTXT(fs::FS &fs, const char* path){
  std::vector<String> lines;
  loadLines(fs, path, lines);
  int top = 0;
  const int maxLines = 7;
  bool viFontLoaded = false;
  static SPIFFSFontWrapper fontWrap;
  if (SPIFFS.exists("/fonts/vi12.vlw")){
    fontWrap.open("/fonts/vi12.vlw");
    viFontLoaded = M5.Display.loadFont(&fontWrap);
  }

  auto draw = [&](){
    if (!viFontLoaded) M5.Display.setTextFont(0);
    M5.Display.fillScreen(COLOR_BG);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 2);
    String name = String(path);
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.print(name);
    int y = STATUS_H + 16;
    int lineH = 14;
    // clear full content area to avoid overdraw
    int y0 = STATUS_H + 14;
    M5.Display.fillRect(0, y0, SCREEN_W, SCREEN_H - y0, COLOR_BG);
    M5.Display.setTextColor(WHITE);
    for (int i=0;i<maxLines;i++){
      int idx = top + i;
      if (idx >= (int)lines.size()) break;
      M5.Display.setCursor(6, y + i*lineH);
      M5.Display.print(lines[idx]);
    }
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.print("Next/Prev scroll  Prev hold=Back");
    if (!viFontLoaded) M5.Display.setTextFont(0);
  };

  draw();
  while (true){
    M5.update();
    if (M5.BtnB.wasPressed()){
      if (top + maxLines < (int)lines.size()) { top++; draw(); }
    }
    if (M5.BtnPWR.wasPressed()){
      if (top > 0) { top--; draw(); }
    }
    if (M5.BtnPWR.pressedFor(800)) break;
    delay(5);
  }
  if (viFontLoaded) { M5.Display.unloadFont(); fontWrap.close(); }
  M5.Display.setTextFont(0);
}
