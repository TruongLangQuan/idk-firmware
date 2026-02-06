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
