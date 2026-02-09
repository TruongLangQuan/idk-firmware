#pragma once
#include "app/state.h"

void webuiStartSTA();
void webuiStartAP();
void webuiStop();
void webuiLoop();
String webuiGetUrl();
void webuiShowInfo(const char* title);
bool webuiPromptOpen();
void webuiInit();
void webuiPersistEnabled(bool enabled);
