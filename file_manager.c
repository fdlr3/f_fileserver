#include "file_manager.h"
#include <stdlib.h>
#include "logger.h"



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
    long filelen = 0;
    fseek(fp, 0, SEEK_END); // seek to end of file
    filelen = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET); // seek back to beginning of file
    return filelen;
}

Instruction 
init_instruction(BYTE* barr){
    Instruction ins;
    memset(&ins, 0, sizeof(Instruction));
    ins.fptr = NULL;
    memset(ins.arg0, '\0', ARG_SIZE);
    memset(ins.arg1, '\0', ARG_SIZE);

    //1. 1BYTE instruction
    ins.flag = (instruction_flag)barr[0];
    //2. 1BYTE number of args (uint8_t)
    ins.flag_c = (uint8_t)barr[1];
    if(ins.flag_c < 0 && ins.flag_c > 2) {
        Log("Wrong number of arguments sent.");
        exit(EXIT_FAILURE);
    }
    //3. 4BYTE file size (uint32_t)
    memcpy(&(ins.file_size), barr+2, sizeof(uint32_t));
    //4. 100BYTE arg0 (has to have null terminate)
    if(ins.flag_c > 0){
        strcpy(ins.arg0, ROOT);
        strcat(ins.arg0, barr+6);
    }
    //5. 100BYTE arg1 (has to have null terminate)
    if(ins.flag_c == 2){
        strcpy(ins.arg1, ROOT);
        strcat(ins.arg1, barr+106);
    }

    Log("Instruction recieved with data:\nInstruction flag: %s,\nflag count: %u"
    "file size: %u,\nargument 1: %s,\nanargument 2: %s",
    ins.flag, 
    ins.flag_c, 
    ins.file_size, 
    ins.arg0, 
    ins.arg1);

    return ins;
}

char* 
get_instruction_name(instruction_flag flag){
    switch(flag){
        case if_PUSH: return "PUSH";
        case if_GET:  return "GET";
        case if_UP:   return "UPDATE";
        case if_REM:  return "REMOVE";
        case if_DIR:  return "DIRECTORY";
        default:      return "ERROR";
    }
}

BYTE*
get_dir(Instruction *ins) {
    struct dirent* d;
    size_t length = 0, data_size = FILE_CHUNK;
    BYTE* data = malloc(FILE_CHUNK);
    if(!data) exit(1);
    *data = '\0';

    //d = opendir(".");
    while ((d = readdir(ins->dirptr)) != NULL) {
        if (d->d_type == DT_REG) {
            size_t name_size = strlen(d->d_name);
            //-2 for delimiter and \0
            if(data_size - 2 <= length + name_size){
                BYTE* temp = realloc(data, data_size * 2);
                if(temp){
                    data = temp;
                    data_size *= 2;
                }
            }
            strcat(data, d->d_name);
            data[length+name_size] = '~';
            data[length+name_size+1] = '\0';
            length += name_size + 1;
        }
    }
    return data;
    //closedir(d);
}




