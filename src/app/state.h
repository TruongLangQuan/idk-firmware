#pragma once

#include <M5Unified.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include <AnimatedGIF.h>
#include <PNGdec.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <vector>

// ===== Pins =====
#define IR_PIN 19
#define BACKLIGHT_PIN 27

// ===== Display / UI =====
static const int SCREEN_W = 240;
static const int SCREEN_H = 135;
static const int STATUS_H = 16;
static const uint16_t COLOR_BG = BLACK;
static const uint16_t COLOR_DIM = 0x7BEF;
static const uint16_t COLOR_HI = 0x294B;

// ===== Screens =====
enum Screen {
  SCR_MENU,
  SCR_WIFI_LIST,
  SCR_CLOCK,
  SCR_IMG_LIST,
  SCR_IMG_VIEW,
  SCR_GIF_LIST,
  SCR_GIF_VIEW,
  SCR_GAMES_LIST,
  SCR_TEST_LIST,
  SCR_IR_LIST,
  SCR_IR_CMD_LIST,
  SCR_FILES_LIST,
  SCR_FILES_BROWSER,
  SCR_SETTING,
  SCR_DEVICE_INFO
};
extern Screen screen;

// ===== Menu =====
extern const char* MENU_ITEMS[];
extern const int MENU_COUNT;
extern int menuIndex;

// ===== File lists =====
#define MAX_FILES 36
extern String imgFiles[MAX_FILES]; extern int imgCount; extern int imgIndex;
extern String gifFiles[MAX_FILES]; extern int gifCount; extern int gifIndex;
extern String irFiles[MAX_FILES];  extern int irCount;  extern int irIndex;
extern String txtFiles[MAX_FILES]; extern int txtCount;

// ===== WiFi =====
#define MAX_WIFI 16
extern String wifiSSID[MAX_WIFI];
extern int wifiRSSI[MAX_WIFI];
extern bool wifiSecured[MAX_WIFI];
extern int wifiCount;
extern int wifiIndex;

// ===== IR commands =====
#define MAX_CMDS 64
extern String cmdName[MAX_CMDS];
extern uint32_t cmdHex[MAX_CMDS];
extern int cmdCount;
extern int cmdIndex;
extern int cmdScroll;
extern IRsend irsend;

// ===== GIF/PNG =====
extern AnimatedGIF gif;
extern File gifFile;
extern PNG png;
extern File pngFile;

// ===== Files browser =====
enum FileSource {FS_WEBUI, FS_LITTLEFS, FS_SD};
extern FileSource filesSource;
extern bool sdReady;
extern String browserFiles[MAX_FILES];
extern int browserCount;
extern int browserIndex;

// ===== Test =====
extern const char* TEST_ITEMS[];
extern const int TEST_COUNT;
extern int testIndex;

// ===== Games =====
extern const char* GAME_ITEMS[];
extern const int GAME_COUNT;
extern int gameIndex;

// ===== Settings / dim =====
extern const char* DIM_TEXT[];
extern const uint32_t DIM_MS[];
extern int dimIndex;
extern uint32_t lastActivity;
extern bool isDimmed;
extern uint8_t BRIGHT_NORMAL;
extern uint8_t BRIGHT_DIM;

extern int setIndex;
extern const char* SET_ITEMS[];
extern const int SET_COUNT;
