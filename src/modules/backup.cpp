#include "modules/backup.h"
#include "modules/config.h"
#include <Preferences.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD.h>
#include "modules/config.h"

static String escapeJson(const String &s){
  String out;
  for (size_t i=0;i<s.length();i++){
    char c = s.charAt(i);
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else out += c;
  }
  return out;
}

static String unescapeJson(const String &s){
  String out;
  for (size_t i=0;i<s.length();i++){
    char c = s.charAt(i);
    if (c == '\\' && i+1 < s.length()){
      char n = s.charAt(i+1);
      if (n == '"') out += '"';
      else if (n == '\\') out += '\\';
      else if (n == 'n') out += '\n';
      else if (n == 'r') out += '\r';
      else out += n;
      i++;
    } else out += c;
  }
  return out;
}

String backupCreateJson(){
  String out = "{";
  out += "\"cfg\":{";
  out += "\"irpin\":" + String(configGetIrPin()) + ",";
  out += "\"sdcs\":" + String(configGetSdCsPin());
  out += "},";
  out += "\"wifi\":[";
  Preferences p;
  p.begin("wifi", true);
  int known = p.getInt("known", 0);
  for (int i=0;i<known;i++){
    String ss = p.getString((String("s") + String(i)).c_str(), "");
    String pw = p.getString((String("p") + String(i)).c_str(), "");
    if (i) out += ",";
    out += "{";
    out += "\"ssid\":\"" + escapeJson(ss) + "\",";
    out += "\"pass\":\"" + escapeJson(pw) + "\"";
    out += "}";
  }
  p.end();
  out += "]}";
  return out;
}

static int parseIntAfterKey(const String &json, const String &key){
  int p = json.indexOf(key);
  if (p < 0) return -1;
  p += key.length();
  // skip spaces
  while (p < json.length() && isSpace(json.charAt(p))) p++;
  int sign = 1;
  if (json.charAt(p) == '-') { sign = -1; p++; }
  long val = 0;
  bool saw = false;
  while (p < json.length() && isDigit(json.charAt(p))) { saw = true; val = val*10 + (json.charAt(p)-'0'); p++; }
  return saw ? (int)(val*sign) : -1;
}

bool backupRestoreFromJson(const String &json){
  int ir = parseIntAfterKey(json, "\"irpin\":");
  int sd = parseIntAfterKey(json, "\"sdcs\":");
  if (ir >= 0) configSetIrPin((uint8_t)ir);
  if (sd >= 0) configSetSdCsPin((uint8_t)sd);

  // restore wifi list
  Preferences p;
  p.begin("wifi", false);
  int old = p.getInt("known", 0);
  for (int i=0;i<old;i++){
    p.remove((String("s") + String(i)).c_str());
    p.remove((String("p") + String(i)).c_str());
  }
  int idx = 0;
  int pos = json.indexOf("\"wifi\":");
  if (pos >= 0){
    pos = json.indexOf('[', pos);
    while (pos >= 0){
      int sspos = json.indexOf("\"ssid\":\"", pos);
      if (sspos < 0) break;
      sspos += 9; // move to after "ssid":"
      int ssend = json.indexOf('"', sspos);
      if (ssend < 0) break;
      String ss = json.substring(sspos, ssend);
      ss = unescapeJson(ss);
      int ppos = json.indexOf("\"pass\":\"", ssend);
      if (ppos < 0) break;
      ppos += 9;
      int pend = json.indexOf('"', ppos);
      if (pend < 0) break;
      String pw = json.substring(ppos, pend);
      pw = unescapeJson(pw);
      p.putString((String("s") + String(idx)).c_str(), ss);
      p.putString((String("p") + String(idx)).c_str(), pw);
      idx++;
      pos = pend;
    }
  }
  p.putInt("known", idx);
  p.end();
  return true;
}

bool backupSaveToFile(const String &path){
  String j = backupCreateJson();
  if (!SPIFFS.begin(true)) return false;
  File f = SPIFFS.open(path, FILE_WRITE);
  if (!f) return false;
  size_t w = f.print(j);
  f.close();
  return w == j.length();
}

bool backupLoadFromFile(const String &path){
  if (!SPIFFS.begin(true)) return false;
  if (!SPIFFS.exists(path)) return false;
  File f = SPIFFS.open(path, FILE_READ);
  if (!f) return false;
  String s;
  while (f.available()) s += (char)f.read();
  f.close();
  return backupRestoreFromJson(s);
}

bool backupSaveToSd(const String &path){
  String j = backupCreateJson();
  uint8_t cs = configGetSdCsPin();
  if (!SD.begin(cs)) return false;
  File f = SD.open(path, FILE_WRITE);
  if (!f) return false;
  size_t w = f.print(j);
  f.close();
  return w == j.length();
}

bool backupLoadFromSd(const String &path){
  uint8_t cs = configGetSdCsPin();
  if (!SD.begin(cs)) return false;
  if (!SD.exists(path)) return false;
  File f = SD.open(path, FILE_READ);
  if (!f) return false;
  String s;
  while (f.available()) s += (char)f.read();
  f.close();
  return backupRestoreFromJson(s);
}
