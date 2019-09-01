#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <dirent.h>

#ifdef __arm__
    #define ROOT "/home/pi/Root/"
#else
    #define ROOT "/home/duler/Desktop/Root/"
#endif

#define FILE_CHUNK 1024
#define MAX_FILE_SIZE INT32_MAX
#define ARG_SIZE 100
typedef uint8_t BYTE;

typedef enum{
    if_PUSH             = 0x00,
    if_GET              = 0x01,
    if_REM              = 0x02,
    if_UP               = 0x03,
    if_DIR              = 0x04,
    if_AUTH             = 0x05,
    if_LOGIN_STATUS     = 0x06,
    if_GO               = 0x07, //go to directory
    if_REV              = 0x08, //Go back in directory
    if_PATH             = 0x09,
    if_MKFD             = 0x0A,
    if_RMFD             = 0x0B
} instruction_flag;

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

Instruction init_instruction(BYTE* barr);
uint32_t parse_4_bytearr(BYTE* arr);
void parse_num(BYTE* arr, uint32_t n);
uint32_t file_size(FILE *fp);
BYTE *get_dir(Instruction *ins);
char* get_ins_name(instruction_flag flag);
bool get_file_data(const char *file_path, __time_t* time, __off_t* size);
int dir_valid(const char* path);

#endif