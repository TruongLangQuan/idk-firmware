#include "modules/media.h"
#include "core/ui.h"

// --- SPIFFS scanning ---
static void scanDir(fs::FS &fs, const char* path){
  File dir = fs.open(path);
  if (!dir || !dir.isDirectory()) return;
  File f = dir.openNextFile();
  while (f) {
    String name = String(path) + "/" + String(f.name());
    if (name.endsWith(".png") && imgCount < MAX_FILES) imgFiles[imgCount++] = name;
    else if (name.endsWith(".gif") && gifCount < MAX_FILES) gifFiles[gifCount++] = name;
    else if (name.endsWith(".ir")  && irCount  < MAX_FILES) irFiles[irCount++] = name;
    else if (name.endsWith(".txt") && txtCount < MAX_FILES) txtFiles[txtCount++] = name;
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
      String n = String(f.name());
      if (n.endsWith(".png") && imgCount < MAX_FILES) imgFiles[imgCount++] = n;
      else if (n.endsWith(".gif") && gifCount < MAX_FILES) gifFiles[gifCount++] = n;
      else if (n.endsWith(".ir") && irCount < MAX_FILES) irFiles[irCount++] = n;
      else if (n.endsWith(".txt") && txtCount < MAX_FILES) txtFiles[txtCount++] = n;
      f = root.openNextFile();
    }
  }
}

// ===== PNG =====
static void *PngOpen(const char *filename, int32_t *size){
  if (!SPIFFS.exists(filename)) return nullptr;
  pngFile = SPIFFS.open(filename, "r");
  if (!pngFile) return nullptr;
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

// ===== GIF =====
static void *GifOpenFile(const char *szFilename, int32_t *pFileSize){
  if (!SPIFFS.exists(szFilename)) return NULL;
  gifFile = SPIFFS.open(szFilename, "r");
  if (!gifFile) return NULL;
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
  int16_t x = pDraw->iX;
  int16_t y = pDraw->iY;
  int16_t w = pDraw->iWidth;
  int16_t h = pDraw->iHeight;
  int16_t maxW = M5.Display.width();
  int16_t maxH = M5.Display.height();
  if (x < 0 || y < 0) return;
  if (x + w > maxW) w = maxW - x;
  if (y + h > maxH) h = maxH - y;
  if (w <= 0 || h <= 0) return;
  uint16_t *pixels = (uint16_t*)pDraw->pPixels;
  M5.Display.pushImage(x, y, w, h, pixels);
}

void playGIFLoop(const char* path){
  while(true){
    gif.begin(LITTLE_ENDIAN_PIXELS);
    int r = gif.open(path, GifOpenFile, GifCloseFile, GifReadFile, GifSeekFile, GifDraw);
    if(!r){
      M5.Display.fillScreen(BLACK);
      M5.Display.setTextColor(RED);
      M5.Display.setCursor(6, 40);
      M5.Display.print("GIF open failed");
      delay(800);
      return;
    }
    M5.Display.startWrite();
    while (gif.playFrame(true, NULL) > 0) {
      M5.update();
      if (M5.BtnPWR.wasPressed()) {
        gif.close();
        M5.Display.endWrite();
        return;
      }
    }
    gif.close();
    M5.Display.endWrite();
  }
}
