#include "app/state.h"

// ===== Globals =====
Screen screen = SCR_MENU;

const char* MENU_ITEMS[] = {"Wifi","Clock","Img","Gif","Games","Test","IR","Files","Setting"};
const int MENU_COUNT = 9;
int menuIndex = 0;

String imgFiles[MAX_FILES]; int imgCount = 0; int imgIndex = 0;
String gifFiles[MAX_FILES]; int gifCount = 0; int gifIndex = 0;
String irFiles[MAX_FILES];  int irCount = 0;  int irIndex = 0;
String txtFiles[MAX_FILES]; int txtCount = 0;

String wifiSSID[MAX_WIFI];
int wifiRSSI[MAX_WIFI];
bool wifiSecured[MAX_WIFI];
int wifiCount = 0;
int wifiIndex = 0;

String cmdName[MAX_CMDS];
uint32_t cmdHex[MAX_CMDS];
int cmdCount = 0;
int cmdIndex = 0;
int cmdScroll = 0;
IRsend irsend(IR_PIN);

AnimatedGIF gif;
File gifFile;
PNG png;
File pngFile;

FileSource filesSource = FS_LITTLEFS;
bool sdReady = false;
String browserFiles[MAX_FILES];
int browserCount = 0;
int browserIndex = 0;

const char* TEST_ITEMS[] = {"Text Display","Function Plot","3D Cube"};
const int TEST_COUNT = 3;
int testIndex = 0;

const char* GAME_ITEMS[] = {"Tetris","Flappy Bird","Slot Machine","Dino"};
const int GAME_COUNT = 4;
int gameIndex = 0;

const char* DIM_TEXT[] = {"Disable","10s","20s","30s","1m"};
const uint32_t DIM_MS[] = {0,10000,20000,30000,60000};
int dimIndex = 0;
uint32_t lastActivity = 0;
bool isDimmed = false;
uint8_t BRIGHT_NORMAL = 255;
uint8_t BRIGHT_DIM = 40;

int setIndex = 0;
const char* SET_ITEMS[] = {"Dim Time","Restart","Power Off","Device Info"};
const int SET_COUNT = 4;

