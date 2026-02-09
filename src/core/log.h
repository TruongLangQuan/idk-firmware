#pragma once
#include <Arduino.h>

// Use distinct symbol names to avoid conflicts with esp-idf macros
void dbg_log_init();
void dbg_log_i(const char *fmt, ...);
void dbg_log_w(const char *fmt, ...);
void dbg_log_e(const char *fmt, ...);

#define LOGI(...) dbg_log_i(__VA_ARGS__)
#define LOGW(...) dbg_log_w(__VA_ARGS__)
#define LOGE(...) dbg_log_e(__VA_ARGS__)
