#ifndef _FDEFINES_INCLUDED_
#define _FDEFINES_INCLUDED_

#include <inttypes.h>

//define of BYTE
typedef uint8_t BYTE;

//path to root
#ifdef __arm__
    #define ROOT "/home/pi/Root/"
#elif unix
    #ifdef JERNEJ_LINUX
        #define ROOT "/home/jernej/Root/"
    #elif DROPLET_LINUX
        #define ROOT "/root/Root/"
    #elif FILIP_LINUX
        #define ROOT "/home/duler/Desktop/Root/"
    #endif
#endif

//path to config file
#ifdef __arm__
    #define CONFIGPATH "/home/pi/Documents/Root/Config/config.txt"
#elif unix
    #ifdef JERNEJ_LINUX
        #define CONFIGPATH "/home/jernej/Root/config.txt"
    #elif DROPLET_LINUX
        #define CONFIGPATH "/root/Root/config.txt"
    #elif FILIP_LINUX
        #define CONFIGPATH "/home/duler/Desktop/Root/Config/config.txt"
    #endif
#endif

#ifdef __arm__
    #define LOG_ROOT "/home/pi/Root/Logs/"
#elif unix
    #ifdef JERNEJ_LINUX
        #define LOG_ROOT "/home/jernej/Root/Logs/"
    #elif DROPLET_LINUX
        #define LOG_ROOT "/home/Root/Logs/"
    #elif FILIP_LINUX
        #define LOG_ROOT "/home/duler/Desktop/Root/Logs/"
    #endif     
#endif

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


#endif