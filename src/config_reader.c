#define _GNU_SOURCE
#include "config_reader.h"
#include <stdlib.h>
#include <string.h>

char* 
get_tag(const char* _conf_path, char* _dest, const char* _tag){
    FILE*       fp          = fopen(_conf_path, "r");
    char *      line        = NULL;
    size_t      len         = 0;
    ssize_t     read;

    if(fp == NULL) { 
        printf("Cannot open config file. Exiting..\n");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        //check if line length is bigger than BUFF_SIZE - 1
        if(read > BUFF_SIZE-1){
            printf("Error in config file. Exiting..\n");
            exit(EXIT_FAILURE);
        }
        //check if line length is smaller than 2
        if(read < 2){
            continue;
        }

        char* pt = strchr(line, ' ');
        if(!pt){
            printf("Error in config file. Exiting..\n");
            exit(EXIT_FAILURE);
        }
        *pt = '\0';
        if(strcmp(line, _tag) == 0){
            memcpy(_dest, pt + 1, read);
            *(_dest + (read+1)) = '\0';
            char* ptnl = strchr(_dest, '\n');
            if(ptnl) *ptnl = '\0';
            break;
        } else continue;
    }
    free(line);
    fclose(fp);
    return _dest;
}

int 
auth_user(const char* _conf_path, const char* _id, const char* _hash)
{
    int     ID_AUTH = 1,
            PW_AUTH = 1;
    char    buffer[256] = {0};
    
    get_tag(_conf_path, buffer, ID_TAG);
    ID_AUTH = strcmp(buffer, _id);
    get_tag(_conf_path, buffer, PW_TAG);
    PW_AUTH = strcmp(buffer, _hash);
    
    return ID_AUTH && PW_AUTH;
}


