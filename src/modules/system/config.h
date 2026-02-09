#pragma once
#include <stdint.h>

void configInit();
uint8_t configGetIrPin();
void configSetIrPin(uint8_t pin);
uint8_t configGetSdCsPin();
void configSetSdCsPin(uint8_t pin);
