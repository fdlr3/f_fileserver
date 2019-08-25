#define _GNU_SOURCE
#include "config_reader.h"
#include <stdlib.h>
#include <string.h>

char* 
get_tag(char* dest, config_TAG tag){
    FILE*       fp          = fopen(CONFIGPATH, "r");
    const char* search_tag  = NULL;
    char        buffer[256] = {0};
    char *      line        = NULL;
    size_t      len         = 0;
    ssize_t     read;

    if(fp == NULL) { return NULL; }

    for(int i = 0; i < config_size; i++){
        if(tag == conf_values[i].tag){
            search_tag = conf_values[i].value;
            break;
        }
    }
    if(search_tag == NULL) { return NULL; }

    while ((read = getline(&line, &len, fp)) != -1) {
        memcpy(buffer, line, strlen(line)+1);
        buffer[3] = '\0';
        
        if(strcmp(search_tag, buffer) == 0){
            strcpy(dest, buffer+4);
            break;
        }
    }
    free(line);
    fclose(fp);
    return dest;
}
