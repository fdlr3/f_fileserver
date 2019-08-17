#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_ROOT "/home/duler/Desktop/Root/Logs/"

void start_logger();
void Log(const char* msg, ...);

#endif