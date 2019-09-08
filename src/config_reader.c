#define _GNU_SOURCE
#include "config_reader.h"
#include <stdlib.h>
#include <string.h>

const config_value conf_values[] = {
    { ID_TAG, "#ID" },
    { PW_TAG, "#PW" },
    { SZ_TAG, "#SZ" }
};

char* 
get_tag(char* dest, config_TAG tag){
    FILE*       fp          = fopen(CONFIGPATH, "r");
    const char* search_tag  = NULL;
    char        buffer[256] = {0};
    char *      line        = NULL;
    size_t      len         = 0;
    ssize_t     read;

    if(fp == NULL) { return NULL; }

    int config_size = sizeof(conf_values) / sizeof(conf_values[0]);

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
            size_t len = strlen(buffer+4);
            strcpy(dest, buffer+4);
            for(int i = 0; i < len; i++){
                if(dest[i] == '\n'){
                    dest[i] = '\0';
                    break;
                }
            }
            break;
        }
    }
    free(line);
    fclose(fp);
    return dest;
}

int 
auth_user(const char* _id, const char* _hash)
{
    int     ID_AUTH = 1,
            PW_AUTH = 1;
    char    buffer[256] = {0};
    
    get_tag(buffer, ID_TAG);
    ID_AUTH = strcmp(buffer, _id);
    get_tag(buffer, PW_TAG);
    PW_AUTH = strcmp(buffer, _hash);
    
    return ID_AUTH && PW_AUTH;
}


