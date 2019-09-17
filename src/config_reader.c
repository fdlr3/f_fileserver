#define _GNU_SOURCE
#include "config_reader.h"
#include <stdlib.h>
#include <string.h>

//Removes all \n and spaces. _dest has to be 255 chars long.
//null is returned on error
static char* get_tag_value(char* _dest_tag, char* _dest_value, const char* _src);

char* 
get_tag(const char* _conf_path, char* _dest, const char* _tag){
    FILE*       fp          = fopen(_conf_path, "r");
    char *      line        = NULL;
    size_t      len         = 0;
    ssize_t     read;
    char        tag_buff[BUFF_SIZE];
    char        val_buff[BUFF_SIZE];

    if(!fp) { 
        perr("Error cannot open config file. Exiting..");
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        //check if line length is bigger than BUFF_SIZE - 1
        if(read > BUFF_SIZE-1){
            if(line) free(line);
            perr("Error in config file. Exiting..");
        }
        //check if line length is smaller than 2
        if(read < 2){
            continue;
        }

        //parse line
        void *tpt = get_tag_value(tag_buff, val_buff, line);
        if(!tpt) {
            if(line) free(line);
            perr("Error in config file. Exiting..");
        }

        if(strcmp(tag_buff, _tag) == 0){
            strcpy(_dest, val_buff);
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
    
    return !(ID_AUTH && PW_AUTH);
}

static char* 
get_tag_value(char* _dest_tag, char* _dest_value, const char* _src)
{
    char temp_buffer[BUFF_SIZE];
    memcpy(temp_buffer, _src, strlen(_src));

    char* pt = strchr(temp_buffer, ' ');
    if(!pt){
        return NULL;
    }

    //copy tag
    *pt = '\0';
    strcpy(_dest_tag, temp_buffer);

    //copy value
    strcpy(_dest_value, pt + 1);
    char *ptnl = strchr(_dest_value, '\n');
    if (ptnl) *ptnl = '\0';

    return _dest_value;
}


