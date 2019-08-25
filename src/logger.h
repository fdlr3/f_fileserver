#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __arm__
    #define LOG_ROOT "/home/pi/Root/Logs/"
#else
    #define LOG_ROOT "/home/duler/Desktop/Root/Logs/"
#endif

void start_logger();
void Log(const char* msg, ...);

#endif