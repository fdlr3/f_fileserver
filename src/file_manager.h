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
    if_DIR              = 0x03,
    if_AUTH             = 0x04,
    if_GO               = 0x05, //go to directory
    if_REV              = 0x06, //Go back in directory
    if_PATH             = 0x07,
    if_MKFD             = 0x08,
    if_RMFD             = 0x09
} instruction_flag;


typedef struct{
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
int init_instruction(Instruction* _ins, BYTE* _barr);
//returns the full-name of the instruction
char* get_ins_name(instruction_flag _flag);
//generate random 4BYTE header for login
uint32_t rnd_key();



/*########## FILES ##########*/

//returns file size
uint32_t file_size(FILE *_fp);
//returns 1 if the file is valid or -1 if invalid
int file_valid(const char* _path);


#endif