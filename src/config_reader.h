#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stdio.h>
#define CONFIGPATH "/home/duler/Desktop/Root/Config/config.txt"


typedef enum{
    ID_TAG = 1 << 0,
    PW_TAG = 1 << 1,
    SZ_TAG = 1 << 2
} config_TAG;

typedef struct{
    config_TAG tag;
    char* value;
} config_value;

config_value conf_values[] = {
    { ID_TAG, "#ID" },
    { PW_TAG, "#PW" },
    { SZ_TAG, "#SZ" }
};

static int config_size = sizeof(conf_values) / sizeof(conf_values[0]);


FILE* open_config();
char* get_tag(FILE* fp, char* dest, config_TAG tag);

#endif