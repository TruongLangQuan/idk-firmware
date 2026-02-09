#include "modules/txt.h"
#include "core/ui.h"
#include "core/log.h"
#include <vector>
#include <new>
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
  if (!f) { LOGE("TXT open failed: %s", path); return; }
  while (f.available() && lines.size() < 200){
    String line = f.readStringUntil('\n');
    line.replace("\r", "");
    if (ESP.getFreeHeap() < 16000) {
      LOGE("Low heap while loading txt, stop reading");
      break;
    }
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
  // Avoid heavy processing if heap is already low
  if (ESP.getFreeHeap() < 30000) {
    LOGE("Low heap before wrapLineMono, skip");
    return;
  }
  if (line.length() == 0){
    out.push_back("");
    return;
  }

  int i = 0;
  String current = "";
  current.reserve(maxChars * 3);
  int count = 0;
  while (i < (int)line.length()){
    int clen = utf8CharLen((uint8_t)line[i]);
    char chbuf[5] = {0};
    for (int k=0;k<clen && (i+k) < (int)line.length(); k++) chbuf[k] = line[i+k];
    if (count >= maxChars && current.length()){
      current.trim();
      if (ESP.getFreeHeap() < 16000) { LOGE("Low heap during wrapLineMono, stop wrapping"); return; }
      try {
        out.push_back(current);
      } catch (const std::bad_alloc &e) {
        LOGE("bad_alloc in wrapLineMono: %s", e.what());
        return;
      }
      current = "";
      count = 0;
    }
    current += chbuf;
    count++;
    i += clen;
  }

  if (current.length()){
    current.trim();
    if (ESP.getFreeHeap() < 16000) { LOGE("Low heap during wrapLineMono final push, skip"); return; }
    try {
      out.push_back(current);
    } catch (const std::bad_alloc &e) {
      LOGE("bad_alloc in wrapLineMono final push: %s", e.what());
      return;
    }
  }
}

// Helper: read the file and return a page of wrapped lines starting at `top`
static bool getTxtPage(fs::FS &fs, const char* path, int top, int maxLines, int maxChars, std::vector<String> &out){
  out.clear();
  out.reserve(maxLines + 2);
  File f = fs.open(path, "r");
  if (!f) { LOGE("TXT open failed: %s", path); return false; }

  int wrappedIndex = 0;
  while (f.available()){
    String line = f.readStringUntil('\n');
    line.replace("\r", "");
    // wrap into temporary small vector to avoid reallocating the page buffer
    std::vector<String> wrapped;
    int approx = (line.length() / maxChars) + 1;
    if (approx < 2) approx = 2;
    if ((size_t)approx > 1000) approx = 1000;
    wrapped.reserve(approx);
    wrapLineMono(line, maxChars, wrapped);

    for (size_t i=0;i<wrapped.size();i++){
      if (wrappedIndex >= top && (int)out.size() < maxLines) out.push_back(wrapped[i]);
      wrappedIndex++;
      if ((int)out.size() >= maxLines) break;
    }
    if ((int)out.size() >= maxLines) break;
    // safety: if wrappedIndex grows huge, stop
    if (wrappedIndex > 20000) break;
  }
  f.close();
  return true;
}

void showTXT(fs::FS &fs, const char* path){
  int top = 0;
  bool viFontLoaded = false;
  static SPIFFSFontWrapper fontWrap;
  if (SPIFFS.exists("/fonts/vi12.vlw")){
    if (!fontWrap.open("/fonts/vi12.vlw")){
      LOGW("Failed open vi font file");
    } else {
      viFontLoaded = M5.Display.loadFont(&fontWrap);
      if (!viFontLoaded) LOGW("Failed load vi font into display");
    }
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

  std::vector<String> page;
  getTxtPage(fs, path, top, maxLines, maxChars, page);

  auto drawPage = [&](const std::vector<String> &lines){
    if (!viFontLoaded) M5.Display.setTextFont(0);
    M5.Display.setTextWrap(false, false);
    M5.Display.fillScreen(COLOR_BG);
    drawStatus();
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(6, STATUS_H + 2);
    String name = String(path);
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.print(name);

    M5.Display.fillRect(0, contentY, SCREEN_W, SCREEN_H - contentY, COLOR_BG);
    int y = contentY + 2;
    M5.Display.setTextColor(WHITE);
    for (int i=0;i<maxLines;i++){
      if (i >= (int)lines.size()) break;
      M5.Display.setCursor(6, y + i*lineH);
      M5.Display.print(lines[i]);
    }
    M5.Display.setTextColor(COLOR_DIM);
    M5.Display.setCursor(6, SCREEN_H-10);
    M5.Display.print("Prev=Down  Next=Right  M5=Back");
    if (!viFontLoaded) M5.Display.setTextFont(0);
  };

  drawPage(page);
  while (true){
    M5.update();
    if (M5.BtnPWR.wasPressed()){
      top++;
      std::vector<String> next;
      getTxtPage(fs, path, top, maxLines, maxChars, next);
      if (next.size() > 0){ page = next; drawPage(page); }
      else top--; // no more lines
    }
    if (M5.BtnB.wasPressed()){
      top += maxLines;
      std::vector<String> next;
      getTxtPage(fs, path, top, maxLines, maxChars, next);
      if (next.size() > 0){ page = next; drawPage(page); }
      else { top -= maxLines; }
    }
    if (M5.BtnA.wasPressed()) break;
    delay(5);
  }
  if (viFontLoaded) { M5.Display.unloadFont(); fontWrap.close(); }
  M5.Display.setTextFont(0);
}
