#include "modules/media/media.h"
#include "core/ui.h"
#include "core/log.h"

// --- SPIFFS scanning ---
static String baseNameOf(const String &path){
  int s = path.lastIndexOf('/');
  if (s >= 0) return path.substring(s + 1);
  return path;
}

static bool existsByBase(String *arr, int count, const String &base){
  for (int i=0;i<count;i++){
    String n = baseNameOf(arr[i]);
    if (n.equalsIgnoreCase(base)) return true;
  }
  return false;
}

static void addUnique(String *arr, int &count, const String &name){
  for (int i=0;i<count;i++){
    if (arr[i] == name) return;
  }
  // De-dup by base name to avoid showing the same file twice
  String base = baseNameOf(name);
  if (existsByBase(arr, count, base)) return;
  if (count < MAX_FILES) arr[count++] = name;
}

static String normPath(const String &in){
  String s = in;
  if (!s.startsWith("/")) s = "/" + s;
  while (s.indexOf("//") >= 0) s.replace("//", "/");
  return s;
}

static void scanDir(fs::FS &fs, const char* path){
  File dir = fs.open(path);
  if (!dir || !dir.isDirectory()) return;
  File f = dir.openNextFile();
  while (f) {
    String n = String(f.name());
    String name = n.startsWith("/") ? n : (String(path) + "/" + n);
    name = normPath(name);
    String lname = name;
    lname.toLowerCase();
    if (lname.endsWith(".png") || lname.endsWith(".jpg") || lname.endsWith(".jpeg")) addUnique(imgFiles, imgCount, name);
    else if (name.endsWith(".gif")) addUnique(gifFiles, gifCount, name);
    else if (name.endsWith(".ir"))  addUnique(irFiles, irCount, name);
    else if (name.endsWith(".txt")) addUnique(txtFiles, txtCount, name);
    f = dir.openNextFile();
  }
}

void scanSPIFFS(){
  imgCount = gifCount = irCount = txtCount = 0;
  scanDir(SPIFFS, "/img");
  scanDir(SPIFFS, "/gifs");
  scanDir(SPIFFS, "/ir");
  scanDir(SPIFFS, "/txt");
  File root = SPIFFS.open("/");
  if (root) {
    File f = root.openNextFile();
    while (f) {
      String n = normPath(String(f.name()));
      // Skip entries already found in subfolders (avoid duplicates like gifs/boot.gif + boot.gif)
      String base = n;
      int s = base.lastIndexOf('/');
      if (s >= 0) base = base.substring(s+1);
      if (existsByBase(imgFiles, imgCount, base) ||
          existsByBase(gifFiles, gifCount, base) ||
          existsByBase(irFiles, irCount, base) ||
          existsByBase(txtFiles, txtCount, base)) {
        f = root.openNextFile();
        continue;
      }
      if (n.endsWith(".png")) addUnique(imgFiles, imgCount, n);
      else if (n.endsWith(".gif")) addUnique(gifFiles, gifCount, n);
      else if (n.endsWith(".jpg") || n.endsWith(".jpeg")) addUnique(imgFiles, imgCount, n);
      else if (n.endsWith(".ir")) addUnique(irFiles, irCount, n);
      else if (n.endsWith(".txt")) addUnique(txtFiles, txtCount, n);
      f = root.openNextFile();
    }
  }
}

// ===== PNG =====
static void *PngOpen(const char *filename, int32_t *size){
  if (!SPIFFS.exists(filename)) { LOGE("PNG not found: %s", filename); return nullptr; }
  pngFile = SPIFFS.open(filename, "r");
  if (!pngFile) { LOGE("PNG open failed: %s", filename); return nullptr; }
  *size = pngFile.size();
  return &pngFile;
}
static void PngClose(void *handle){
  File *f = (File*)handle;
  if (f) f->close();
}
static int32_t PngRead(PNGFILE *handle, uint8_t *buf, int32_t len){
  File *f = (File*)handle->fHandle;
  int32_t r = f->read(buf, len);
  return r;
}
static int32_t PngSeek(PNGFILE *handle, int32_t pos){
  File *f = (File*)handle->fHandle;
  f->seek(pos);
  return pos;
}
static int PngDraw(PNGDRAW *pDraw){
  uint16_t *line = (uint16_t*)pDraw->pPixels;
  M5.Display.pushImage(0, pDraw->y, pDraw->iWidth, 1, line);
  return 1;
}

void showPNG(const char* path){
  M5.Display.fillScreen(BLACK);
  png.open(path, PngOpen, PngClose, PngRead, PngSeek, PngDraw);
  png.decode(NULL, 0);
  png.close();
}

void showJPG(const char* path){
  M5.Display.fillScreen(BLACK);
  int y = STATUS_H;
  int h = SCREEN_H - STATUS_H;
  bool ok = M5.Display.drawJpgFile(path, 0, y, SCREEN_W, h);
  if (!ok){
    M5.Display.setTextColor(RED);
    M5.Display.setCursor(6, STATUS_H + 20);
    M5.Display.print("JPG open failed");
  }
}

// ===== GIF =====
static void *GifOpenFile(const char *szFilename, int32_t *pFileSize){
  if (!SPIFFS.exists(szFilename)) { LOGE("GIF not found: %s", szFilename); return NULL; }
  gifFile = SPIFFS.open(szFilename, "r");
  if (!gifFile) { LOGE("GIF open failed: %s", szFilename); return NULL; }
  *pFileSize = gifFile.size();
  gifFile.seek(0);
  return &gifFile;
}
static void GifCloseFile(void *pHandle){
  File *f = (File*)pHandle;
  if (f) f->close();
}
static int32_t GifReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen){
  gifFile.seek(pFile->iPos);
  int32_t r = gifFile.read(pBuf, iLen);
  pFile->iPos += r;
  return r;
}
static int32_t GifSeekFile(GIFFILE *pFile, int32_t iPosition){
  if (iPosition < 0) iPosition = 0;
  if (iPosition >= pFile->iSize) iPosition = pFile->iSize - 1;
  pFile->iPos = iPosition;
  gifFile.seek(pFile->iPos);
  return iPosition;
}
static void GifDraw(GIFDRAW *pDraw){
  static uint16_t *lineBuf = nullptr;
  if (!lineBuf){
    lineBuf = (uint16_t*)malloc(SCREEN_W * sizeof(uint16_t));
    if (!lineBuf) { LOGE("GIF lineBuf alloc failed"); return; }
  }
  int x = pDraw->iX;
  int y = pDraw->iY + pDraw->y;
  int w = pDraw->iWidth;
  if (x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return;
  if (x + w > SCREEN_W) w = SCREEN_W - x;
  if (w <= 0) return;

  uint8_t *s = pDraw->pPixels;
  uint16_t *palette = pDraw->pPalette;

  if (pDraw->ucDisposalMethod == 2){
    for (int i=0;i<w;i++){
      if (s[i] == pDraw->ucTransparent) s[i] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  if (pDraw->ucHasTransparency){
    int xPos = 0;
    while (xPos < w){
      while (xPos < w && s[xPos] == pDraw->ucTransparent) xPos++;
      int runStart = xPos;
      while (xPos < w && s[xPos] != pDraw->ucTransparent){
        lineBuf[xPos] = palette[s[xPos]];
        xPos++;
      }
      int runLen = xPos - runStart;
      if (runLen > 0){
        M5.Display.pushImage(x + runStart, y, runLen, 1, &lineBuf[runStart]);
      }
    }
  } else {
    for (int i=0;i<w;i++) lineBuf[i] = palette[s[i]];
    M5.Display.pushImage(x, y, w, 1, lineBuf);
  }
}

void playGIFLoop(const char* path){
  gif.begin(GIF_PALETTE_RGB565_LE);
  while(true){
    int r = gif.open(path, GifOpenFile, GifCloseFile, GifReadFile, GifSeekFile, GifDraw);
    if(!r){ LOGE("gif.open returned error for %s (r=%d)", path, r);
      M5.Display.fillScreen(BLACK);
      M5.Display.setTextColor(RED);
      M5.Display.setCursor(6, 40);
      M5.Display.print("GIF open failed");
      delay(800);
      return;
    }
    while (gif.playFrame(true, NULL) > 0) {
      M5.update();
      if (M5.BtnPWR.wasPressed()) {
        gif.close();
        return;
      }
    }
    gif.close();
  }
}
