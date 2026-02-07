#include "modules/clock.h"
#include "core/ui.h"
#include <time.h>

void setupTime(){
  setenv("TZ", "ICT-7", 1);
  tzset();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

bool timeValid(){
  time_t now = time(nullptr);
  return now > 1609459200; // 2021-01-01
}

void drawClock(){
  M5.Display.fillScreen(COLOR_BG);
  drawStatus();
  time_t now = time(nullptr);
  if (!timeValid()){
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(6, STATUS_H + 20);
    M5.Display.print("Syncing time...");
    return;
  }
  struct tm *t = localtime(&now);
  char timeBuf[16];
  char dateBuf[20];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", t);
  strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", t);

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(30, 50);
  M5.Display.print(timeBuf);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(70, 80);
  M5.Display.print(dateBuf);
}
