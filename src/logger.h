#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "fdefines.h"
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

void start_logger(const char* _log_path);
void Log(const char* msg, ...);

#endif