#pragma once
#include "app/state.h"

void scanWifi();
void wifiConnectTo(int idx);
void wifiAutoConnect();
int wifiGetKnownCount();
String wifiGetKnownSSID(int idx);
void wifiForgetKnown(int idx);
