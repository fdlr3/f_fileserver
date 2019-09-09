#define _DEFAULT_SOURCE
#include "file_manager.h"
#include "logger.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

//unlinks folder files
int un_remove   (const char* _fpath, const struct stat* _sb, 
                int _typeflag, struct FTW* _ftwbuf);


/*########## DIRECTORY ##########*/

//returns all files and folders in the directory
BYTE*
dir_contents(Instruction *_ins) 
{
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

    while ((d = readdir(_ins->dirptr)) != NULL) {
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
    return data;
}

//checks if the directory is valid
int 
dir_valid(const char* _path)
{
    DIR* dir = opendir(_path);
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
un_remove(const char* _fpath, const struct stat* _sb, 
          int _typeflag, struct FTW* ftwbuf) 
{
    int rv = remove(_fpath);
    if (rv)
        perror(_fpath);

    return rv;
}

//removes a directory
int 
remove_directory(char *_path)
{
    return nftw(_path, un_remove, 64, FTW_DEPTH | FTW_PHYS);
}

/*########## INSTRUCTION ##########*/

//creates an instruction
int 
init_instruction(Instruction* _ins, BYTE* _barr)
{
    memset(_ins, 0, sizeof(Instruction));
    _ins->fptr = NULL;

    //1. 1BYTE instruction
    _ins->flag = (instruction_flag)_barr[0];
    if(strcmp(get_ins_name(_ins->flag), "ERROR") == 0) return -1;

    //2. 1BYTE number of args (uint8_t)
    _ins->flag_c = (uint8_t)_barr[1];
    if(_ins->flag_c < 0 || _ins->flag_c > 2) return -1;
    
    //3. 4BYTE file size (uint32_t)
    memcpy(&(_ins->file_size), _barr+2, UI32_B);
    if(_ins->flag == if_PUSH || _ins->flag == if_UP){
        if(_ins->file_size == 0) return -1;
        if(_ins->file_size > MAX_FILE_SIZE) return -1;
    }

    //4. 100BYTE arg0 (has to have null terminate)
    if(_ins->flag_c > 0){
        char* nult = memchr(_barr+6, '\0', 100);
        if(nult == NULL) return -1;
        strcpy(_ins->arg0, (char*)(_barr+6));
    }

    //5. 100BYTE arg1 (has to have null terminate)
    if(_ins->flag_c == 2){
        char* nult = memchr(_barr+106, '\0', 100);
        if(nult == NULL) return -1;
        strcpy(_ins->arg1, (char*)(_barr+106));
    }

    return 1;
}

//returns the full-name of the instruction
char* 
get_ins_name(instruction_flag _flag)
{
    switch(_flag){
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

//generate random 4BYTE header for login
uint32_t 
rnd_key(){
    return (uint32_t)(rand() % (INT32_MAX) + 1);
}

/*########## FILES ##########*/

//returns file size
uint32_t 
file_size(FILE *_fp)
{
    unsigned long filelen = 0;
    fseek(_fp, 0, SEEK_END); 
    filelen = ftell(_fp); 
    fseek(_fp, 0, SEEK_SET); 
    return (uint32_t)filelen;
}
