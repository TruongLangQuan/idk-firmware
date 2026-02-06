#include "modules/ir.h"

void parseIRFile(const String &path){
  cmdCount = 0;
  File f = SPIFFS.open(path, "r");
  if (!f) return;
  while(f.available() && cmdCount < MAX_CMDS){
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;
    if (line.indexOf('=') > 0) {
      int eq = line.indexOf('=');
      String name = line.substring(0, eq);
      String val = line.substring(eq+1);
      name.trim(); val.trim();
      uint32_t v = (uint32_t) strtoul(val.c_str(), NULL, 0);
      cmdName[cmdCount] = name;
      cmdHex[cmdCount] = v;
      cmdCount++;
    } else if (line.startsWith("RAW:") || line.startsWith("raw:")) {
      String raw = line.substring(line.indexOf(':')+1);
      raw.trim();
      cmdName[cmdCount] = "RAW:" + raw;
      cmdHex[cmdCount] = 0;
      cmdCount++;
    }
  }
  f.close();
}

void sendIRCommand(int idx){
  if (idx < 0 || idx >= cmdCount) return;
  if (cmdHex[idx] != 0) {
    irsend.sendNEC(cmdHex[idx], 32);
  } else {
    String rawLine = cmdName[idx].substring(4);
    std::vector<uint16_t> timings;
    int start = 0;
    while (start < (int)rawLine.length()) {
      int comma = rawLine.indexOf(',', start);
      if (comma == -1) comma = rawLine.length();
      String token = rawLine.substring(start, comma);
      token.trim();
      int v = token.toInt();
      if (v > 0) timings.push_back((uint16_t)v);
      start = comma + 1;
    }
    if (timings.size() > 0) {
      irsend.sendRaw(timings.data(), timings.size(), 38);
    }
  }
}

void spamAllIRCommands(){
  for (int i=0;i<cmdCount;i++){
    sendIRCommand(i);
    delay(120);
  }
}
