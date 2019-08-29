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
    if_PUSH     = 1 << 0,
    if_GET      = 1 << 1,
    if_REM      = 1 << 2,
    if_UP       = 1 << 3,
    if_DIR      = 1 << 4,
    if_AUTH     = 1 << 5,
    if_RESP     = 1 << 6,
    if_ERROR    = 1 << 7
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

#endif