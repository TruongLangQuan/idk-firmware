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

static int utf8CharLen(uint8_t c){
  if ((c & 0x80) == 0) return 1;
  if ((c & 0xE0) == 0xC0) return 2;
  if ((c & 0xF0) == 0xE0) return 3;
  if ((c & 0xF8) == 0xF0) return 4;
  return 1;
}

static void wrapLineMono(const String &line, int maxChars, std::vector<String> &out){
  if (line.length() == 0){
    out.push_back("");
    return;
  }

  int i = 0;
  String current = "";
  int count = 0;
  while (i < (int)line.length()){
    int clen = utf8CharLen((uint8_t)line[i]);
    String ch = line.substring(i, i + clen);
    if (count >= maxChars && current.length()){
      current.trim();
      out.push_back(current);
      current = "";
      count = 0;
    }
    current += ch;
    count++;
    i += clen;
  }

  if (current.length()){
    current.trim();
    out.push_back(current);
  }
}

void showTXT(fs::FS &fs, const char* path){
  std::vector<String> rawLines;
  loadLines(fs, path, rawLines);
  int top = 0;
  bool viFontLoaded = false;
  static SPIFFSFontWrapper fontWrap;
  if (SPIFFS.exists("/fonts/vi12.vlw")){
    fontWrap.open("/fonts/vi12.vlw");
    viFontLoaded = M5.Display.loadFont(&fontWrap);
  }

  int contentY = STATUS_H + 14;
  int lineH = M5.Display.fontHeight();
  if (lineH < 12) lineH = 12;
  int maxWidth = SCREEN_W - 12;
  int charW = M5.Display.textWidth("W");
  if (charW <= 0) charW = 6;
  int maxChars = maxWidth / charW;
  if (maxChars < 8) maxChars = 8;
  int maxLines = (SCREEN_H - 12 - contentY) / lineH;
  if (maxLines < 1) maxLines = 1;

  std::vector<String> lines;
  lines.reserve(rawLines.size() * 2);
  for (size_t i=0;i<rawLines.size();i++){
    wrapLineMono(rawLines[i], maxChars, lines);
    if (lines.size() > 600) break; // avoid heavy render on large files
  }

  auto draw = [&](){
    if (!viFontLoaded) M5.Display.setTextFont(0);
    M5.Display.setTextWrap(false, false);
    M5.Display.fillScreen(COLOR_BG);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 2);
    String name = String(path);
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.print(name);

    // clear full content area to avoid overdraw
    M5.Display.fillRect(0, contentY, SCREEN_W, SCREEN_H - contentY, COLOR_BG);
    int y = contentY + 2;
    M5.Display.setTextColor(WHITE);
    for (int i=0;i<maxLines;i++){
      int idx = top + i;
      if (idx >= (int)lines.size()) break;
      M5.Display.setCursor(6, y + i*lineH);
      M5.Display.print(lines[idx]);
    }
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.print("Prev=Down  Next=Right  M5=Back");
    if (!viFontLoaded) M5.Display.setTextFont(0);
  };

  draw();
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()){
      if (top + maxLines < (int)lines.size()) { top++; draw(); }
    }
    if (M5.BtnB.wasPressed()){
      if (top + maxLines < (int)lines.size()) {
        top += maxLines;
        if (top >= (int)lines.size()) top = (int)lines.size() - 1;
        draw();
      }
    }
    if (M5.BtnA.wasPressed()) break;
    delay(5);
  }
  if (viFontLoaded) { M5.Display.unloadFont(); fontWrap.close(); }
  M5.Display.setTextFont(0);
}
