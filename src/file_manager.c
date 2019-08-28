#define D_BSD_SOURCE
#include "file_manager.h"
#include <stdlib.h>
#include "logger.h"
#include <sys/stat.h>

uint32_t 
parse_4_bytearr(BYTE* arr){
    uint32_t size = 0;
    memcpy(&size, arr, sizeof(uint32_t));
    return size;
}

void 
parse_num(BYTE* arr, uint32_t n){
    memcpy(arr, &n, sizeof(uint32_t));
}

uint32_t 
file_size(FILE *fp){
    unsigned long filelen = 0;
    fseek(fp, 0, SEEK_END); 
    filelen = ftell(fp); 
    fseek(fp, 0, SEEK_SET); 
    return (uint32_t)filelen;
}

Instruction 
init_instruction(BYTE* barr){
    Instruction ins;
    size_t      root_len = strlen(ROOT);

    memset(&ins, 0, sizeof(Instruction));
    ins.valid = true;
    ins.fptr = NULL;

    //1. 1BYTE instruction
    ins.flag = (instruction_flag)barr[0];
    if(strcmp(get_ins_name(ins.flag), "ERROR") == 0){
        Log("Wrong instruction flag.");
        ins.valid = false;
        return ins;
    }

    //2. 1BYTE number of args (uint8_t)
    ins.flag_c = (uint8_t)barr[1];
    if(ins.flag_c < 0 || ins.flag_c > 2) {
        Log("Wrong number of arguments sent.");
        ins.valid = false;
        return ins;
    }
    
    //3. 4BYTE file size (uint32_t)
    memcpy(&(ins.file_size), barr+2, sizeof(uint32_t));

    //4. 100BYTE arg0 (has to have null terminate)
    if(ins.flag_c > 0){
        char* nult = memchr(barr+6, '\0', 99);
        if(nult == NULL){
            Log("No nul terminate in first argument or size too big.");
        }
        if(ins.flag == if_AUTH){
            strcpy(ins.arg0, (char*)(barr+6));
        } else {
            memcpy(ins.arg0, ROOT, root_len);
            strcat(ins.arg0 + root_len, (char*)(barr+6));
        }
    }

    //5. 100BYTE arg1 (has to have null terminate)
    if(ins.flag_c == 2){
        char* nult = memchr(barr+106, '\0', 99);
        if(nult == NULL){
            Log("No nul terminate in second argument or size too big.");
        }
        if(ins.flag == if_AUTH){
            strcpy(ins.arg1, (char*)(barr+106));
        } else {
            memcpy(ins.arg1, ROOT, root_len);
            strcat(ins.arg1 + root_len, (char*)(barr+106));
        }
    }

    Log("\nInstruction recieved with data:\nInstruction flag: %s,\nflag count: %u\n"
    "file size: %u,\nargument 1: %s,\nanargument 2: %s",
    get_ins_name(ins.flag), 
    ins.flag_c, 
    ins.file_size, 
    ins.arg0, 
    ins.arg1);

    return ins;
}

char* 
get_ins_name(instruction_flag flag){
    switch(flag){
        case if_PUSH: return "PUSH";
        case if_GET:  return "GET";
        case if_UP:   return "UPDATE";
        case if_REM:  return "REMOVE";
        case if_DIR:  return "DIRECTORY";
        case if_AUTH: return "AUTHENTICATE";
        default:      return "ERROR";
    }
}

BYTE*
get_dir(Instruction *ins) {
    struct dirent*  d;
    size_t          length      = 0, 
                    data_size   = FILE_CHUNK;
    BYTE*           data        = malloc(FILE_CHUNK);
    time_t          change_time = 0;
    long            file_size   = 0;
    char            file_path[256];
    BYTE            buffer[256];
    uint16_t        root_len = strlen(ROOT);

    if(!data || root_len < 1) return NULL;
    strcpy(file_path, ROOT);
    *data = '\0';

    while ((d = readdir(ins->dirptr)) != NULL) {
        if (d->d_type == DT_REG) {
            size_t name_size = strlen(d->d_name);
            //-2 for delimiter and \0
            if(data_size - 255 <= length + name_size){
                BYTE* temp = realloc(data, data_size * 2);
                if(temp){
                    data = temp;
                    data_size *= 2;
                }
            }
            //set file path
            strcat(file_path, d->d_name);
            if(!get_file_data(file_path, &change_time, &file_size)){
                change_time = 0; file_size = 0;
            }
            file_path[root_len] = '\0';

            //set name
            strcpy((char*)buffer, d->d_name);
            buffer[name_size] = '*'; 
            //set size
            snprintf((char*)(buffer+name_size + 1), 255-name_size, "%ld*", file_size);
            size_t cur_len = strlen(buffer);
            //set time
            snprintf((char*)(buffer+cur_len), 255-cur_len, "%ld~\0", (long)change_time);

            length += strlen(buffer);
            strcat(data, buffer);
        }
    }
    printf("%s\n", (char*)data);
    return data;
}


bool 
get_file_data(const char *file_path, __time_t* time, __off_t* size){
    struct stat statbuf;
    if(stat(file_path, &statbuf) == -1){
        return false;
    }
    *time = statbuf.st_mtime;
    *size = statbuf.st_size;
    return true;
}

