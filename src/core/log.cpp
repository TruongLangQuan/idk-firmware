#include "core/log.h"
#include <stdarg.h>

void dbg_log_init(){
  Serial.begin(115200);
  delay(10);
  char buf[64];
  snprintf(buf, sizeof(buf), "[LOG] boot millis=%lu", millis());
  Serial.println(buf);
}

static void dbg_vlog(const char *level, const char *fmt, va_list ap){
  char buf[256];
  int n = snprintf(buf, sizeof(buf), "[%s] ", level);
  vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
  Serial.println(buf);
}

void dbg_log_i(const char *fmt, ...){
  va_list ap; va_start(ap, fmt); dbg_vlog("I", fmt, ap); va_end(ap);
}
void dbg_log_w(const char *fmt, ...){
  va_list ap; va_start(ap, fmt); dbg_vlog("W", fmt, ap); va_end(ap);
}
void dbg_log_e(const char *fmt, ...){
  va_list ap; va_start(ap, fmt); dbg_vlog("E", fmt, ap); va_end(ap);
}
