#include "system/power.h"

void setBacklight(uint8_t level){
  const int ch = 0;
  ledcSetup(ch, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, ch);
  ledcWrite(ch, level);
}
