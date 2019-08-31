#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stdio.h>


#ifdef __arm__
    #define CONFIGPATH "/home/pi/Documents/Root/Config/config.txt"
#else
    #define CONFIGPATH "/home/duler/Desktop/Root/Config/config.txt"
#endif

typedef enum{
    ID_TAG = 1 << 0,
    PW_TAG = 1 << 1,
    SZ_TAG = 1 << 2
} config_TAG;

typedef struct{
    config_TAG tag;
    char* value;
} config_value;

extern const config_value conf_values[];

char* get_tag(char* dest, config_TAG tag);

#endif