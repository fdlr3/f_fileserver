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
    //2. 1BYTE number of args (uint8_t)
    ins.flag_c = (uint8_t)barr[1];
    if(ins.flag_c < 0 || ins.flag_c > 2) {
        Log("Wrong number of arguments sent.");
        ins.valid = false;
    }
    //3. 4BYTE file size (uint32_t)
    memcpy(&(ins.file_size), barr+2, sizeof(uint32_t));
    //4. 100BYTE arg0 (has to have null terminate)
    if(ins.flag_c > 0){
        memcpy(ins.arg0, ROOT, root_len);
        strcat(ins.arg0 + root_len, barr+6);
    }
    //5. 100BYTE arg1 (has to have null terminate)
    if(ins.flag_c == 2){
        memcpy(ins.arg0, ROOT, root_len);
        strcat(ins.arg0 + root_len, barr+106);
    }

    Log("\nInstruction recieved with data:\nInstruction flag: %u,\nflag count: %u\n"
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
    struct dirent*  d;
    size_t          length = 0, 
                    data_size = FILE_CHUNK;
    BYTE*           data = malloc(FILE_CHUNK);

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
}




