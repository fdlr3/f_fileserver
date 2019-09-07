#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

#define _XOPEN_SOURCE 500

#include "fdefines.h"
#include <ftw.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>

typedef enum{
    if_PUSH             = 0x00,
    if_GET              = 0x01,
    if_REM              = 0x02,
    if_UP               = 0x03,
    if_DIR              = 0x04,
    if_AUTH             = 0x05,
    if_GO               = 0x06, //go to directory
    if_REV              = 0x07, //Go back in directory
    if_PATH             = 0x08,
    if_MKFD             = 0x09,
    if_RMFD             = 0x0A
} instruction_flag;

typedef enum{
    e_PATH = 0x01,
    e_FILE = 0x02,
    e_READ_FILE = 0x03,
    e_FILE_SIZE = 0x04,
    e_REMOVE_FILE = 0x05,
    e_PATH_OVERFLOW = 0x06,
    e_PATH_ZERO_LEN = 0x07,
    e_FOLDER_NOT_FOUND = 0x08,
    e_FOLDER = 0x09
} f_errors;

typedef struct{
    bool valid;
    instruction_flag flag;
    uint8_t flag_c;
    char arg0[ARG_SIZE];
    char arg1[ARG_SIZE];
    uint32_t file_size;
    FILE* fptr;
    DIR *dirptr;
} Instruction;


/*########## DIRECTORY ##########*/

//returns all files and folders in the directory
BYTE *dir_contents(Instruction *_ins);
//checks if the directory is valid
int dir_valid(const char* _path);
//removes a directory
int remove_directory(char *_path);



/*########## INSTRUCTION ##########*/

//creates an instruction
Instruction init_instruction(BYTE* _barr);
//returns the full-name of the instruction
char* get_ins_name(instruction_flag _flag);



/*########## FILES ##########*/

//returns file size
uint32_t file_size(FILE *_fp);



#endif