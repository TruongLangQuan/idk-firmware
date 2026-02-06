#pragma once
#include "app/state.h"

void parseIRFile(const String &path);
void sendIRCommand(int idx);
void spamAllIRCommands();
