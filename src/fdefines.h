#ifndef _FDEFINES_INCLUDED_
#define _FDEFINES_INCLUDED_

#include <inttypes.h>
#include <stdio.h>
#include "logger.h"

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
//max login attempts before block
#define MAX_AUTH_AT     3

//SUCCESS
#define SUCCESS 0xFF
//FAILED
#define FAIL 0x00

//tag for ID
#define ID_TAG          "ID"
//tag for password
#define PW_TAG          "PW"
//tag for root folder path
#define ROOTPATH_TAG    "RootPath"
//tag for log folder path
#define LOGPATH_TAG     "LogPath"
//hostname tag
#define HOST_TAG        "Hostname"
//port tag
#define PORT_TAG        "Port"
//number of acceptabled IPS listed in config file
#define AIPNUM_TAG      "AIPNum"
//acceptable IP -> "AIP" + number (e.g. "AIP4" )
#define AIP_TAG         "AIP"

#define perr(x)\
    puts(x);\
    Log(x);\
    exit(0);

#endif