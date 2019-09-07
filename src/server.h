#ifndef _SERVER_H_
#define _SERVER_H_

#include "fdefines.h"
#include "file_manager.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>


typedef struct{
    int fd;
    struct sockaddr_in addr;
    socklen_t clilen;
    bool auth;

    //current directory path
    char f_directory[BUFF_SIZE];
    //length of directory path
    uint8_t fdir_len;
    //length where root ends
    uint8_t root_end;
} f_client;

//SERVER STRUCT
typedef struct {
    //client
    f_client fc;
    //file descriptor for the socket
    int server_fd;
    //socket address
    struct sockaddr_in server_addr;
    //port number
    int16_t port_num;
    
} f_server;

void    start_server    (int16_t _port, f_server* _server);
void    listen_server   (f_server* _server);
void    server_IO       (f_server *_fs);
void    close_server    (f_server *_fs);

int 
authenticate_u(const char* _id, const char* _hash);

#endif