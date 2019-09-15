#ifndef _FDEFINES_INCLUDED_
#define _FDEFINES_INCLUDED_

#include <inttypes.h>
#include <stdio.h>

//define of BYTE
typedef uint8_t BYTE;

//send or read max packet size
#define FILE_CHUNK      1024
//max file size 
#define MAX_FILE_SIZE   INT32_MAX - 1
//size of an instruction argument size
#define ARG_SIZE        100
//define buffer size
#define BUFF_SIZE       255
//sizeof uint32_t
#define UI32_B          sizeof(uint32_t)
//instruction size
#define INS_SIZE        206

//SUCCESS
#define SUCCESS 0xFF
//FAILED
#define FAIL 0x00

//config file defines
#define ID_TAG          "ID"
#define PW_TAG          "PW"
#define ROOTPATH_TAG    "RootPath"
#define LOGPATH_TAG     "LogPath"

#endif