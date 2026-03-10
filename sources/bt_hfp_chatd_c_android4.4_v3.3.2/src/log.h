#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
static inline void bt_logp(const char* lvl, const char* fmt, ...) {
    time_t t=time(NULL); struct tm tmv; localtime_r(&t,&tmv);
    fprintf(stderr,"[%02d:%02d:%02d] %s: ", tmv.tm_hour, tmv.tm_min, tmv.tm_sec, lvl);
    va_list ap; va_start(ap,fmt); vfprintf(stderr,fmt,ap); va_end(ap);
    fprintf(stderr,"\n");
}
#define LOGI(...) bt_logp("I", __VA_ARGS__)
#define LOGW(...) bt_logp("W", __VA_ARGS__)
#define LOGE(...) bt_logp("E", __VA_ARGS__)
