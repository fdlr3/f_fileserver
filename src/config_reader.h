#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include "fdefines.h"
#include <stdio.h>

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