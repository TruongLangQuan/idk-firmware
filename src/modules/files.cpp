#include "modules/files.h"

void buildBrowserList(){
  browserCount = 0;
  fs::FS *fs = nullptr;
  if (filesSource == FS_LITTLEFS) fs = &SPIFFS;
  else if (filesSource == FS_SD && sdReady) fs = &SD;
  if (!fs) return;
  File root = fs->open("/");
  if (!root) return;
  File f = root.openNextFile();
  while (f && browserCount < MAX_FILES){
    String n = String(f.name());
    browserFiles[browserCount++] = n;
    f = root.openNextFile();
  }
}

static bool deletePathInternal(fs::FS &fs, const String &path){
  File f = fs.open(path);
  if (!f) return false;
  if (!f.isDirectory()){
    f.close();
    return fs.remove(path);
  }
  File c = f.openNextFile();
  while (c){
    String name = String(c.name());
    if (c.isDirectory()){
      deletePathInternal(fs, name);
    } else {
      fs.remove(name);
    }
    c = f.openNextFile();
  }
  f.close();
  return fs.rmdir(path);
}

bool deletePathFS(fs::FS &fs, const String &path){
  return deletePathInternal(fs, path);
}
