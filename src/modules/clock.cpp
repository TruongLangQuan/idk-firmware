#include "modules/clock.h"
#include "core/ui.h"
#include <time.h>

void setupTime(){
  // Use Vietnam timezone (Ho Chi Minh)
  setenv("TZ", "Asia/Ho_Chi_Minh", 1);
  tzset();
  // Configure NTP servers; TZ env + tzset() make localtime() return Vietnam time
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

bool timeValid(){
  time_t now = time(nullptr);
  return now > 1609459200; // 2021-01-01
}

void drawClock(bool showStatus){
  M5.Display.fillScreen(COLOR_BG);
  if (showStatus) drawStatus();
  
  time_t now = time(nullptr);
  if (!timeValid()){
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    int y = showStatus ? STATUS_H + 20 : 20;
    M5.Display.setCursor(6, y);
    M5.Display.print("Syncing time...");
    return;
  }
  // Some builds don't honor TZ names; compute Vietnam local time (UTC+7) explicitly.
  time_t localNow = now + 7 * 3600; // +7 hours
  struct tm *t = gmtime(&localNow);
  char timeBuf[16];
  char dateBuf[20];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", t);
  strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", t);

  // Large centered clock - larger text size
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(5); // Increased from 4 to 5
  int16_t tx = (SCREEN_W - (int)strlen(timeBuf) * 30) / 2; // approximate centering for size 5
  if (tx < 0) tx = 6;
  int yOffset = showStatus ? STATUS_H : 0;
  M5.Display.setCursor(tx, (SCREEN_H/2) - 35 + yOffset/2);
  M5.Display.print(timeBuf);
  M5.Display.setTextSize(2); // Increased from 1 to 2
  int16_t dx = (SCREEN_W - (int)strlen(dateBuf) * 12) / 2;
  if (dx < 0) dx = 6;
  M5.Display.setCursor(dx, (SCREEN_H/2) + 20 + yOffset/2);
  M5.Display.print(dateBuf);
}
