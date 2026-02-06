#pragma once
#include "app/state.h"

void drawStatus();
void drawMenu();
void drawList(const char* title, String *items, int count, int index);
void drawListCstr(const char* title, const char** items, int count, int index);
void drawSetting();
void drawIRCmdList();
void drawWifiList();
void drawFilesList();
void drawFilesBrowser();
