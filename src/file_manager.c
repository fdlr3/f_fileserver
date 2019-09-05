#define _DEFAULT_SOURCE
#include "file_manager.h"
#include "logger.h"
#include <errno.h>

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
        strcpy(ins.arg0, (char*)(barr+6));

    }

    //5. 100BYTE arg1 (has to have null terminate)
    if(ins.flag_c == 2){
        char* nult = memchr(barr+106, '\0', 99);
        if(nult == NULL){
            Log("No nul terminate in second argument or size too big.");
        }
        strcpy(ins.arg1, (char*)(barr+106));

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
        case if_PUSH:           return "PUSH";
        case if_GET:            return "GET";
        case if_UP:             return "UPDATE";
        case if_REM:            return "REMOVE";
        case if_DIR:            return "DIRECTORY";
        case if_AUTH:           return "AUTHENTICATE";
        case if_GO:             return "GO";
        case if_REV:            return "REV";
        case if_PATH:           return "PATH";
        default:                return "ERROR";
    }
}

BYTE*
get_dir(Instruction *ins) {
    struct dirent*  d;
    size_t          str_size;
    size_t          end         = 0;
    size_t          file_len    = 0;
    BYTE*           data        = malloc(1024);
    const char      del         = '~';

    if(!data){
        return NULL;
    }
    str_size = 1024;

    while ((d = readdir(ins->dirptr)) != NULL) {
        file_len = strlen(d->d_name);
        if(str_size <= end + file_len){
            BYTE* temp = realloc(data, str_size*2);
            if(temp == NULL){
                free(data);
                return NULL;
            }
            data = temp;
            str_size *= 2;
        }
        if (strcmp(d->d_name, ".") == 0 || 
            strcmp(d->d_name, "..") == 0){
            continue;
        }
        if (d->d_type == DT_REG || d->d_type == DT_DIR) {
            memcpy(data + end, d->d_name, file_len + 1);
            end += file_len;
            data[end++] = del;
        }
    }
    data[end] = '\0';
    printf("%s", data);
    return data;
}

int 
dir_valid(const char* path){
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        //exists
        return 1;
    } else if (ENOENT == errno) {
        //doesnt exist
        return 0;
    } else {
        //something else
        return -1;
    }
}


int 
un_remove(const char* fpath, const struct stat* sb, 
int typeflag, struct FTW* ftwbuf) {
    int rv = remove(fpath);
    if (rv)
        perror(fpath);

    return rv;
}

int 
all_rem(char *path)
{
    return nftw(path, un_remove, 64, FTW_DEPTH | FTW_PHYS);
}

