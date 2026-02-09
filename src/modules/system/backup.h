#pragma once
#include <Arduino.h>

String backupCreateJson();
bool backupRestoreFromJson(const String &json);
bool backupSaveToFile(const String &path);
bool backupLoadFromFile(const String &path);
bool backupSaveToSd(const String &path);
bool backupLoadFromSd(const String &path);
