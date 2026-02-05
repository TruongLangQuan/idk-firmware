// src/main.ino
// Fixed all-in-one firmware for M5StickC Plus2
// - Menu: Gif / IR / TXT / Setting
// - GIF playback from SPIFFS (loop) using AnimatedGIF (callbacks)
// - IR support: parse Name=0xHEX and RAW:comma,timings ; send + Spam All
// - TXT reader from SPIFFS (Next = down, Prev = exit)
// - Device Info in Settings
// - Dim time + backlight control
// - Power off using M5.Power.powerOff()
// - Buttons: BtnA=Select, BtnB=Next, BtnPWR=Prev

#include <M5Unified.h>
#include <SPIFFS.h>
#include <AnimatedGIF.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <vector>

#define IR_PIN 19
#define BACKLIGHT_PIN 27
#define STATUS_BAR_H 18

// Screens
enum Screen {
  SCR_MENU,
  SCR_GIF_LIST,
  SCR_GIF_VIEW,
  SCR_IR_LIST,
  SCR_IR_CMD_LIST,
  SCR_TXT_LIST,
  SCR_TXT_VIEW,
  SCR_SETTING,
  SCR_DEVICE_INFO
};
Screen screen = SCR_MENU;

// Menu
const char* MENU_ITEMS[] = {"Gif", "IR", "TXT", "Setting"};
const int MENU_COUNT = 4;
int menuIndex = 0;

// GIF
#define MAX_GIFS 64
String gifList[MAX_GIFS];
int gifCount = 0;
int gifIndex = 0;
AnimatedGIF gif;
File gifFile;

// IR files & commands
#define MAX_IR_FILES 64
String irFiles[MAX_IR_FILES];
int irFileCount = 0;
int irFileIndex = 0;

#define MAX_CMDS 128
String cmdName[MAX_CMDS];
uint32_t cmdHex[MAX_CMDS];
int cmdCount = 0;
int cmdIndex = 0;
int cmdScroll = 0;
IRsend irsend(IR_PIN);

// TXT
#define MAX_TXT_FILES 64
String txtFiles[MAX_TXT_FILES];
int txtFileCount = 0;
int txtFileIndex = 0;

#define MAX_LINES 600
String txtLines[MAX_LINES];
int txtLineCount = 0;
int txtScroll = 0;

// Settings / dim
const char* SET_ITEMS[] = {"Dim Time", "Restart", "Power Off", "Device Info"};
const int SET_COUNT = 4;
int setIndex = 0;

const char* DIM_TEXT[] = {"Disable","10s","20s","30s","1m"};
const uint32_t DIM_MS[] = {0,10000,20000,30000,60000};
int dimIndex = 0;
uint32_t lastActivity = 0;
bool isDimmed = false;
uint8_t BRIGHT_NORMAL = 255;
uint8_t BRIGHT_DIM = 40;

// Helpers
void setBacklight(uint8_t level){
  const int ch = 0;
  ledcSetup(ch, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, ch);
  ledcWrite(ch, level);
}

void drawStatus(){
  M5.Display.fillRect(0,0,240,STATUS_BAR_H,BLACK);
  M5.Display.setCursor(4,2);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Status: Ready");
  int bat = M5.Power.getBatteryLevel();
  uint16_t color = GREEN;
  if (bat <= 20) color = RED;
  else if (bat <= 50) color = YELLOW;
  M5.Display.setTextColor(color);
  M5.Display.setCursor(160,2);
  M5.Display.printf("Bat:%d%%", bat);
}

// --- SPIFFS scanning (subfolders) ---
void scanDir(const char* path) {
  File dir = SPIFFS.open(path);
  if (!dir || !dir.isDirectory()) return;
  File f = dir.openNextFile();
  while (f) {
    String name = String(path) + "/" + String(f.name());
    if (name.endsWith(".gif") && gifCount < MAX_GIFS) gifList[gifCount++] = name;
    else if (name.endsWith(".ir") && irFileCount < MAX_IR_FILES) irFiles[irFileCount++] = name;
    else if (name.endsWith(".txt") && txtFileCount < MAX_TXT_FILES) txtFiles[txtFileCount++] = name;
    f = dir.openNextFile();
  }
}

void scanSPIFFS(){
  gifCount = irFileCount = txtFileCount = 0;
  // scan dedicated folders; also scan root as fallback
  scanDir("/gifs");
  scanDir("/ir");
  scanDir("/txt");

  // fallback: also scan root to catch files placed directly
  File root = SPIFFS.open("/");
  if (root) {
    File f = root.openNextFile();
    while (f) {
      String n = String(f.name());
      if (n.endsWith(".gif") && gifCount < MAX_GIFS) gifList[gifCount++] = n;
      else if (n.endsWith(".ir") && irFileCount < MAX_IR_FILES) irFiles[irFileCount++] = n;
      else if (n.endsWith(".txt") && txtFileCount < MAX_TXT_FILES) txtFiles[txtFileCount++] = n;
      f = root.openNextFile();
    }
  }
  Serial.printf("SPIFFS scan → GIF:%d IR:%d TXT:%d\n", gifCount, irFileCount, txtFileCount);
}

// --- UI draws ---
void drawMenu(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  for(int i=0;i<MENU_COUNT;i++){
    int y = STATUS_BAR_H + 12 + i*20;
    if (i == menuIndex){
      M5.Display.fillRect(0,y-4,240,18,0x294B);
      M5.Display.setTextColor(WHITE);
      M5.Display.setCursor(8,y);
      M5.Display.printf("> %s", MENU_ITEMS[i]);
    } else {
      M5.Display.setTextColor(0xC618);
      M5.Display.setCursor(8,y);
      M5.Display.printf("  %s", MENU_ITEMS[i]);
    }
  }
}

void drawGifList(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  for(int i=0;i<gifCount;i++){
    int y = STATUS_BAR_H + 12 + i*16;
    if (i == gifIndex){
      M5.Display.fillRect(0,y-4,240,14,0x294B);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(0xC618);
    }
    String name = gifList[i];
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.setCursor(8,y);
    M5.Display.print(name);
  }
}

void drawIRList(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  for(int i=0;i<irFileCount;i++){
    int y = STATUS_BAR_H + 12 + i*16;
    if (i == irFileIndex){
      M5.Display.fillRect(0,y-4,240,14,0x294B);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(0xC618);
    }
    String name = irFiles[i];
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.setCursor(8,y);
    M5.Display.print(name);
  }
}

void drawTXTList(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  for(int i=0;i<txtFileCount;i++){
    int y = STATUS_BAR_H + 12 + i*16;
    if (i == txtFileIndex){
      M5.Display.fillRect(0,y-4,240,14,0x294B);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(0xC618);
    }
    String name = txtFiles[i];
    if (name.startsWith("/")) name = name.substring(1);
    M5.Display.setCursor(8,y);
    M5.Display.print(name);
  }
}

void drawSetting(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  for(int i=0;i<SET_COUNT;i++){
    int y = STATUS_BAR_H + 12 + i*18;
    if (i == setIndex){
      M5.Display.fillRect(0,y-4,240,16,0x294B);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(0xC618);
    }
    M5.Display.setCursor(8,y);
    M5.Display.print(SET_ITEMS[i]);
    if (i == 0){
      M5.Display.setCursor(150,y);
      M5.Display.print(DIM_TEXT[dimIndex]);
    }
  }
}

void drawDeviceInfo(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  M5.Display.setCursor(6, STATUS_BAR_H + 12);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(1);
  M5.Display.printf("Chip: ESP32\n");
  M5.Display.printf("CPU: %d MHz\n", ESP.getCpuFreqMHz());
  M5.Display.printf("Flash: %d MB\n", ESP.getFlashChipSize()/1024/1024);
  M5.Display.printf("Heap free: %d KB\n", ESP.getFreeHeap()/1024);
  size_t total = SPIFFS.totalBytes();
  size_t used  = SPIFFS.usedBytes();
  M5.Display.printf("SPIFFS: %d/%d KB\n", used/1024, total/1024);
  M5.Display.printf("Uptime: %lus\n", millis()/1000);
}

// --- AnimatedGIF callbacks ---
void *GifOpenFile(const char *szFilename, int32_t *pFileSize){
  if (!SPIFFS.exists(szFilename)) return NULL;
  gifFile = SPIFFS.open(szFilename, "r");
  if (!gifFile) return NULL;
  *pFileSize = gifFile.size();
  gifFile.seek(0);
  return &gifFile;
}

void GifCloseFile(void *pHandle){
  File *f = (File*)pHandle;
  if (f) f->close();
}

int32_t GifReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen){
  gifFile.seek(pFile->iPos);
  int32_t r = gifFile.read(pBuf, iLen);
  pFile->iPos += r;
  return r;
}

int32_t GifSeekFile(GIFFILE *pFile, int32_t iPosition){
  if (iPosition < 0) iPosition = 0;
  if (iPosition >= pFile->iSize) iPosition = pFile->iSize - 1;
  pFile->iPos = iPosition;
  gifFile.seek(pFile->iPos);
  return iPosition;
}

void GifDraw(GIFDRAW *pDraw){
  uint16_t *pixels = (uint16_t*)pDraw->pPixels;
  // draw under status bar
  M5.Lcd.pushImage(pDraw->iX, pDraw->iY + STATUS_BAR_H, pDraw->iWidth, pDraw->iHeight, pixels);
}

// play GIF loop (re-open on EOF) — keeps calling M5.update() so buttons work
void playGIFLoop(const char* path){
  while(true){
    gif.begin(LITTLE_ENDIAN_PIXELS);
    int r = gif.open(path, GifOpenFile, GifCloseFile, GifReadFile, GifSeekFile, GifDraw);
    if(!r){
      Serial.print("GIF open failed: "); Serial.println(path);
      M5.Display.fillScreen(BLACK);
      drawStatus();
      M5.Display.setCursor(6, 40);
      M5.Display.setTextColor(RED);
      M5.Display.print("GIF open failed");
      delay(800);
      return;
    }
    M5.Lcd.startWrite();
    while (gif.playFrame(true, NULL) > 0) {
      M5.update();
      if (M5.BtnPWR.wasPressed()) {
        gif.close();
        M5.Lcd.endWrite();
        return;
      }
    }
    gif.close();
    M5.Lcd.endWrite();
    // reopen to loop
  }
}

// --- IR parsing & sending ---
void parseIRFile(const String &path){
  cmdCount = 0;
  File f = SPIFFS.open(path, "r");
  if (!f) {
    Serial.println("IR open fail");
    return;
  }
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
  Serial.printf("Parsed IR commands: %d\n", cmdCount);
}

void sendIRCommand(int idx){
  if (idx < 0 || idx >= cmdCount) return;
  if (cmdHex[idx] != 0) {
    irsend.sendNEC(cmdHex[idx], 32);
    Serial.printf("Sent NEC 0x%08X\n", cmdHex[idx]);
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
      Serial.printf("Sent RAW len=%d\n", (int)timings.size());
    }
  }
}

void spamAllIRCommands(){
  for (int i=0;i<cmdCount;i++){
    sendIRCommand(i);
    delay(120);
  }
}

// --- TXT reader ---
void loadTXTFile(const String &path){
  txtLineCount = 0;
  txtScroll = 0;
  String p = path;
  if (!p.startsWith("/")) p = "/" + p;
  File f = SPIFFS.open(p, "r");
  if (!f) {
    Serial.println("TXT open fail: " + p);
    return;
  }
  while(f.available() && txtLineCount < MAX_LINES){
    String ln = f.readStringUntil('\n');
    ln.replace("\r", "");
    ln.trim();
    txtLines[txtLineCount++] = ln;
  }
  f.close();
  Serial.printf("Loaded TXT lines=%d\n", txtLineCount);
}

void drawTXTView(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  int y = STATUS_BAR_H + 8;
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(1);
  int linesPerPage = 6;
  for (int i=0;i<linesPerPage;i++){
    int li = txtScroll + i;
    if (li >= txtLineCount) break;
    M5.Display.setCursor(6, y);
    M5.Display.print(txtLines[li]);
    y += 16;
  }
  M5.Display.setCursor(6, 135-12);
  M5.Display.setTextColor(0xC618);
  M5.Display.print("Prev=Exit  Next=Down");
}

// --- IR command list draw with scrolling ---
void drawIRCmdList(){
  M5.Display.fillScreen(BLACK);
  drawStatus();
  int maxShow = 8;
  for(int i=0;i<maxShow;i++){
    int idx = cmdScroll + i;
    if(idx >= cmdCount) break;
    int y = STATUS_BAR_H + 12 + i*14;
    if(idx == cmdIndex){
      M5.Display.fillRect(0,y-4,240,12,0x294B);
      M5.Display.setTextColor(WHITE);
    } else {
      M5.Display.setTextColor(0xC618);
    }
    M5.Display.setCursor(8,y);
    if(cmdName[idx].startsWith("RAW:")) M5.Display.print("RAW");
    else M5.Display.print(cmdName[idx]);
  }
  M5.Display.setCursor(6, 135-12);
  M5.Display.setTextColor(0xC618);
  M5.Display.print("A=Send  B=Next  PWR=Back");
}

// --- setup / loop ---
void setup(){
  Serial.begin(115200);
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Power.begin();
  irsend.begin();
  M5.Display.setRotation(3);
  M5.Display.setTextFont(0);
  setBacklight(BRIGHT_NORMAL);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed!");
  }

  scanSPIFFS();

  // fallback scan of subfolders already done
  drawMenu();
  lastActivity = millis();
}

void loop(){
  M5.update();

  // reset idle timer on any press
  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()){
    lastActivity = millis();
    if(isDimmed){
      setBacklight(BRIGHT_NORMAL);
      isDimmed=false;
    }
  }

  // update battery every 5s
  static uint32_t t = 0;
  if (millis() - t > 5000){
    drawStatus();
    t = millis();
  }

  // auto dim
  if (dimIndex != 0 && !isDimmed && (millis() - lastActivity >= DIM_MS[dimIndex])){
    setBacklight(BRIGHT_DIM);
    isDimmed = true;
  }

  // handle screens
  switch(screen){
    case SCR_MENU:
      if (M5.BtnB.wasPressed()){
        menuIndex = (menuIndex + 1) % MENU_COUNT;
        drawMenu();
      }
      if (M5.BtnPWR.wasPressed()){
        menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT;
        drawMenu();
      }
      if (M5.BtnA.wasPressed()){
        if (menuIndex == 0){ // GIF
          if (gifCount > 0){
            screen = SCR_GIF_LIST; gifIndex = 0; drawGifList();
          } else {
            M5.Display.fillScreen(BLACK); drawStatus();
            M5.Display.setCursor(6, 50); M5.Display.setTextColor(RED); M5.Display.print("No GIFs");
          }
        } else if (menuIndex == 1){ // IR
          if (irFileCount > 0){
            screen = SCR_IR_LIST; irFileIndex = 0; drawIRList();
          } else {
            M5.Display.fillScreen(BLACK); drawStatus();
            M5.Display.setCursor(6, 50); M5.Display.setTextColor(RED); M5.Display.print("No IR files");
          }
        } else if (menuIndex == 2){ // TXT
          if (txtFileCount > 0){
            screen = SCR_TXT_LIST; txtFileIndex = 0; drawTXTList();
          } else {
            M5.Display.fillScreen(BLACK); drawStatus();
            M5.Display.setCursor(6, 50); M5.Display.setTextColor(RED); M5.Display.print("No TXT files");
          }
        } else if (menuIndex == 3){ // Setting
          screen = SCR_SETTING; setIndex = 0; drawSetting();
        }
      }
      break;

    case SCR_GIF_LIST:
      if (M5.BtnB.wasPressed()){
        gifIndex = (gifIndex + 1) % gifCount; drawGifList();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_MENU; drawMenu();
      }
      if (M5.BtnA.wasPressed()){
        String path = gifList[gifIndex];
        screen = SCR_GIF_VIEW;
        playGIFLoop(path.c_str());
        screen = SCR_GIF_LIST; drawGifList();
      }
      break;

    case SCR_IR_LIST:
      if (M5.BtnB.wasPressed()){
        irFileIndex = (irFileIndex + 1) % irFileCount; drawIRList();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_MENU; drawMenu();
      }
      if (M5.BtnA.wasPressed()){
        parseIRFile(irFiles[irFileIndex]);
        cmdIndex = 0; cmdScroll = 0;
        screen = SCR_IR_CMD_LIST;
        drawIRCmdList();
      }
      break;

    case SCR_IR_CMD_LIST:
      if (M5.BtnB.wasPressed()){
        cmdIndex = (cmdIndex + 1) % cmdCount;
        if (cmdIndex >= cmdScroll + 8) cmdScroll++;
        drawIRCmdList();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_IR_LIST; drawIRList();
      }
      if (M5.BtnA.wasPressed()){
        sendIRCommand(cmdIndex);
      }
      if (M5.BtnA.pressedFor(800)){
        spamAllIRCommands();
        drawIRCmdList();
      }
      break;

    case SCR_TXT_LIST:
      if (M5.BtnB.wasPressed()){
        txtFileIndex = (txtFileIndex + 1) % txtFileCount; drawTXTList();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_MENU; drawMenu();
      }
      if (M5.BtnA.wasPressed()){
        loadTXTFile(txtFiles[txtFileIndex]);
        screen = SCR_TXT_VIEW;
        drawTXTView();
      }
      break;

    case SCR_TXT_VIEW:
      if (M5.BtnB.wasPressed()){
        if (txtScroll + 6 < txtLineCount) txtScroll++;
        drawTXTView();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_TXT_LIST; drawTXTList();
      }
      break;

    case SCR_SETTING:
      if (M5.BtnB.wasPressed()){
        setIndex = (setIndex + 1) % SET_COUNT; drawSetting();
      }
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_MENU; drawMenu();
      }
      if (M5.BtnA.wasPressed()){
        if (setIndex == 0){
          dimIndex = (dimIndex + 1) % 5;
          lastActivity = millis();
          isDimmed = false;
          setBacklight(BRIGHT_NORMAL);
          drawSetting();
        } else if (setIndex == 1){
          M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6,50); M5.Display.print("Restarting...");
          delay(300);
          ESP.restart();
        } else if (setIndex == 2){
          M5.Display.fillScreen(BLACK); drawStatus(); M5.Display.setCursor(6,50); M5.Display.print("Powering off...");
          delay(200);
          M5.Power.powerOff();
        } else if (setIndex == 3){
          screen = SCR_DEVICE_INFO; drawDeviceInfo();
        }
      }
      break;

    case SCR_DEVICE_INFO:
      if (M5.BtnPWR.wasPressed()){
        screen = SCR_SETTING; drawSetting();
      }
      break;

    default:
      break;
  }

  delay(10);
}
