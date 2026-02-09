#include "modules/system/config.h"
#include <Preferences.h>
#include "modules/features/ir.h"

static Preferences cfgPrefs;
static const char* CFG_NS = "cfg";
static const char* KEY_IR_PIN = "irpin";
static const char* KEY_SD_CS = "sdcs";

static uint8_t s_ir_pin = IR_PIN;
static uint8_t s_sd_cs = 4; // sensible default for many boards

void configInit(){
  cfgPrefs.begin(CFG_NS, true);
  s_ir_pin = cfgPrefs.getUChar(KEY_IR_PIN, IR_PIN);
  s_sd_cs = cfgPrefs.getUChar(KEY_SD_CS, 4);
  cfgPrefs.end();
  // apply IR pin to module
  setIrPin(s_ir_pin);
}

uint8_t configGetIrPin(){ return s_ir_pin; }
void configSetIrPin(uint8_t pin){
  s_ir_pin = pin;
  cfgPrefs.begin(CFG_NS, false);
  cfgPrefs.putUChar(KEY_IR_PIN, s_ir_pin);
  cfgPrefs.end();
  setIrPin(s_ir_pin);
}

uint8_t configGetSdCsPin(){ return s_sd_cs; }
void configSetSdCsPin(uint8_t pin){
  s_sd_cs = pin;
  cfgPrefs.begin(CFG_NS, false);
  cfgPrefs.putUChar(KEY_SD_CS, s_sd_cs);
  cfgPrefs.end();
}
