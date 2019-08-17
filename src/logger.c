#include "logger.h"
#include <stdlib.h>
#include <string.h>


struct Logger{
    FILE* logfp;
    time_t start_time;
    char directory[256];
};

static struct Logger __log;

void start_logger(){    
    struct tm* info;

    time(&(__log.start_time));
    info = localtime(&(__log.start_time));

    strcpy(__log.directory, LOG_ROOT);
    strftime(__log.directory + strlen(LOG_ROOT), 100, "LOG%d-%b-%Y--%I:%M", info);    
    strcat(__log.directory, ".txt");
}

void Log(const char* format, ...){
    //open file
    __log.logfp = fopen(__log.directory, "a");
    

    struct tm*  info;
    char        msg[256];
    va_list     args;

    //get time
    time(&(__log.start_time));
    info = localtime(&(__log.start_time));
    strftime(msg, 256, "%x-%I:%M%p -> ", info);
    size_t msg_start = strlen(msg);

    //write user message
    va_start(args, format);
    vsnprintf(msg + msg_start, sizeof(msg), format, args);
    va_end(args);

    if(!__log.logfp){
        printf("Couldnt log message %s", msg);
    }
    fprintf(__log.logfp, "%s\n", msg);
    fclose(__log.logfp);
}