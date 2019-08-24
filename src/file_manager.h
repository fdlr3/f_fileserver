#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <dirent.h>

#define ROOT "/home/duler/Desktop/Root/"
//#define ROOT "/home/pi/Root/"
#define D_BSD_SOURCE
#define FILE_CHUNK 1024
#define MAX_FILE_SIZE INT32_MAX
#define ARG_SIZE 100
typedef uint8_t BYTE;

/*  ####### SIZE - 206 BYTES #######
*   1 BYTE = instruction
*   1 BYTE = argument count (uint8_t)
*   4 BYTE = file_size (uint32_t)
*   100 BYTE = arg0
*   100 BYTE = arg1
*
*   get [file name] (gets file)
*   push [file name] (pushes file)
*   dir (get all files)
*   del [file_name] (delete file)
*   up [replaced_file_name] [file_name] (replace file with a new one)
*
*
*/

typedef enum{
    if_PUSH     = 1 << 0,
    if_GET      = 1 << 1,
    if_REM      = 1 << 2,
    if_UP       = 1 << 3,
    if_DIR      = 1 << 4,
    if_ERROR    = 1 << 5
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
char* get_instruction_name(instruction_flag flag);

bool get_file_data(const char *file_path, __time_t* time, __off_t* size);

#endif