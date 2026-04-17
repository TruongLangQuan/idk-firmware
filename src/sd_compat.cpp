#include "sd_compat.h"
#include <SdFat.h>
#include <SPI.h>

#ifndef SDCARD_SPI_MHZ
#define SDCARD_SPI_MHZ 25
#endif
#ifndef SDCARD_SPI_FALLBACK1_MHZ
#define SDCARD_SPI_FALLBACK1_MHZ 12
#endif
#ifndef SDCARD_SPI_FALLBACK2_MHZ
#define SDCARD_SPI_FALLBACK2_MHZ 4
#endif

static SdFs s_sd;
static bool s_sdReady = false;
static uint8_t s_lastCs = 0xFF;

#ifdef SDCARD_SPI_BUS
static SPIClass s_sdSpi(SDCARD_SPI_BUS);
static SPIClass* s_spi = &s_sdSpi;
#else
static SPIClass* s_spi = &SPI;
#endif

struct SdFileHandle {
  FsFile file;
};

struct SdDirHandle {
  FsFile dir;
  FsFile entry;
  String base;
};

static String joinPath(const String &base, const String &name){
  if (base.length() == 0 || base == "/") return "/" + name;
  if (base.endsWith("/")) return base + name;
  return base + "/" + name;
}

bool sdCompatBegin(uint8_t csPin){
  if (s_sdReady && s_lastCs == csPin) return true;
  s_sdReady = false;
  s_lastCs = csPin;

  const uint8_t speeds[] = {SDCARD_SPI_MHZ, SDCARD_SPI_FALLBACK1_MHZ, SDCARD_SPI_FALLBACK2_MHZ};
  for (uint8_t mhz : speeds){
    if (s_sd.begin(SdSpiConfig(csPin, SHARED_SPI, SD_SCK_MHZ(mhz), s_spi))){
      s_sdReady = true;
      return true;
    }
  }
  return false;
}

bool sdCompatReady(){
  return s_sdReady;
}

uint64_t sdCompatTotalBytes(){
  if (!s_sdReady) return 0;
  auto *vol = s_sd.vol();
  if (!vol) return 0;
  uint64_t clusters = vol->clusterCount();
  uint64_t sectors = clusters * (uint64_t)vol->sectorsPerCluster();
  return sectors * 512ULL;
}

uint64_t sdCompatUsedBytes(){
  if (!s_sdReady) return 0;
  auto *vol = s_sd.vol();
  if (!vol) return 0;
  uint64_t clusters = vol->clusterCount();
  uint64_t freeClusters = vol->freeClusterCount();
  uint64_t usedClusters = (clusters > freeClusters) ? (clusters - freeClusters) : 0;
  uint64_t sectors = usedClusters * (uint64_t)vol->sectorsPerCluster();
  return sectors * 512ULL;
}

uint32_t sdCompatCardErrorCode(){
  if (!s_sd.card()) return 0;
  return s_sd.card()->errorCode();
}

uint32_t sdCompatCardErrorData(){
  if (!s_sd.card()) return 0;
  return s_sd.card()->errorData();
}

bool sdCompatExists(const String &path){
  if (!s_sdReady) return false;
  return s_sd.exists(path.c_str());
}

bool sdCompatMkdir(const String &path){
  if (!s_sdReady) return false;
  return s_sd.mkdir(path.c_str());
}

bool sdCompatMkdirs(const String &path){
  if (!s_sdReady) return false;
  if (path.length() == 0 || path == "/") return true;
  if (sdCompatExists(path)) return true;
  int pos = 1;
  while (true){
    pos = path.indexOf('/', pos);
    String sub = (pos < 0) ? path : path.substring(0, pos);
    if (!sdCompatExists(sub)) sdCompatMkdir(sub);
    if (pos < 0) break;
    pos++;
  }
  return sdCompatExists(path);
}

bool sdCompatRmdir(const String &path){
  if (!s_sdReady) return false;
  return s_sd.rmdir(path.c_str());
}

bool sdCompatRemove(const String &path){
  if (!s_sdReady) return false;
  return s_sd.remove(path.c_str());
}

bool sdCompatRename(const String &from, const String &to){
  if (!s_sdReady) return false;
  return s_sd.rename(from.c_str(), to.c_str());
}

bool sdCompatIsDirPath(const String &path){
  if (!s_sdReady) return false;
  FsFile f = s_sd.open(path.c_str());
  if (!f) return false;
  bool isDir = f.isDir();
  f.close();
  return isDir;
}

uint64_t sdCompatDirSize(const String &path){
  if (!s_sdReady) return 0;
  FsFile dir = s_sd.open(path.c_str());
  if (!dir || !dir.isDir()) return 0;
  uint64_t sum = 0;
  FsFile entry;
  while (entry.openNext(&dir, O_RDONLY)){
    char name[256];
    name[0] = 0;
    entry.getName(name, sizeof(name));
    if (entry.isDir()){
      sum += sdCompatDirSize(joinPath(path, name));
    } else {
      sum += entry.fileSize();
    }
    entry.close();
  }
  dir.close();
  return sum;
}

bool sdCompatDeletePath(const String &path){
  if (!s_sdReady) return false;
  FsFile dir = s_sd.open(path.c_str());
  if (!dir) return false;
  if (!dir.isDir()){
    dir.close();
    return s_sd.remove(path.c_str());
  }
  FsFile entry;
  while (entry.openNext(&dir, O_RDONLY)){
    char name[256];
    name[0] = 0;
    entry.getName(name, sizeof(name));
    String child = joinPath(path, name);
    if (entry.isDir()){
      sdCompatDeletePath(child);
    } else {
      s_sd.remove(child.c_str());
    }
    entry.close();
  }
  dir.close();
  return s_sd.rmdir(path.c_str());
}

SdDirHandle* sdCompatOpenDir(const String &path){
  if (!s_sdReady) return nullptr;
  SdDirHandle *h = new SdDirHandle();
  if (!h->dir.open(path.c_str()) || !h->dir.isDir()){
    h->dir.close();
    delete h;
    return nullptr;
  }
  h->base = path;
  if (h->base.length() > 1 && h->base.endsWith("/")) h->base.remove(h->base.length()-1);
  return h;
}

bool sdCompatDirNext(SdDirHandle *dir, SdDirEntry &out){
  if (!dir) return false;
  dir->entry.close();
  if (!dir->entry.openNext(&dir->dir, O_RDONLY)) return false;
  char name[256];
  name[0] = 0;
  dir->entry.getName(name, sizeof(name));
  String full = joinPath(dir->base, name);
  out.name = full;
  out.isDir = dir->entry.isDir();
  out.size = out.isDir ? 0 : dir->entry.fileSize();
  return true;
}

void sdCompatDirRewind(SdDirHandle *dir){
  if (!dir) return;
  dir->entry.close();
  dir->dir.rewind();
}

void sdCompatDirClose(SdDirHandle *dir){
  if (!dir) return;
  dir->entry.close();
  dir->dir.close();
  delete dir;
}

SdFileHandle* sdCompatOpenRead(const String &path){
  if (!s_sdReady) return nullptr;
  SdFileHandle *h = new SdFileHandle();
  if (!h->file.open(path.c_str(), O_RDONLY)){
    delete h;
    return nullptr;
  }
  return h;
}

SdFileHandle* sdCompatOpenWrite(const String &path, bool truncate){
  if (!s_sdReady) return nullptr;
  SdFileHandle *h = new SdFileHandle();
  oflag_t flags = O_RDWR | O_CREAT;
  if (truncate) flags |= O_TRUNC;
  if (!h->file.open(path.c_str(), flags)){
    delete h;
    return nullptr;
  }
  return h;
}

bool sdCompatIsDir(SdFileHandle *file){
  if (!file) return false;
  return file->file.isDir();
}

uint64_t sdCompatFileSize(SdFileHandle *file){
  if (!file) return 0;
  return file->file.fileSize();
}

bool sdCompatAvailable(SdFileHandle *file){
  if (!file) return false;
  return file->file.available() > 0;
}

int sdCompatRead(SdFileHandle *file, uint8_t *buf, size_t len){
  if (!file) return -1;
  return file->file.read(buf, len);
}

int sdCompatReadByte(SdFileHandle *file){
  if (!file) return -1;
  return file->file.read();
}

size_t sdCompatWrite(SdFileHandle *file, const uint8_t *buf, size_t len){
  if (!file) return 0;
  return file->file.write(buf, len);
}

size_t sdCompatPrint(SdFileHandle *file, const String &s){
  if (!file) return 0;
  return file->file.print(s);
}

bool sdCompatSeek(SdFileHandle *file, uint32_t pos){
  if (!file) return false;
  return file->file.seekSet(pos);
}

uint32_t sdCompatPosition(SdFileHandle *file){
  if (!file) return 0;
  return file->file.position();
}

String sdCompatReadStringUntil(SdFileHandle *file, char delim){
  String out;
  if (!file) return out;
  while (true){
    int c = file->file.read();
    if (c < 0) break;
    if ((char)c == delim) break;
    out += (char)c;
  }
  return out;
}

Stream* sdCompatStream(SdFileHandle *file){
  if (!file) return nullptr;
  return static_cast<Stream*>(&file->file);
}

void sdCompatClose(SdFileHandle *file){
  if (!file) return;
  file->file.close();
  delete file;
}
