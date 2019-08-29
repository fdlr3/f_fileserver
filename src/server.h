#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "file_manager.h"

typedef struct{
    int fd;
    struct sockaddr_in addr;
    socklen_t clilen;
    bool authenticated;
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
    //f_pointer to run and close server
    void(* listen_server)    (void*);
    void(* close_server)  (void*);
} f_server;

void start_server(int16_t port, f_server *server);
void listen_server(void *server);
void server_IO(f_server *fs);
void close_server(void *server);

int send_data(int fd, BYTE* buffer, size_t n);
int read_data(int fd, BYTE* buffer, size_t n);
int send_file(FILE *fp, f_client* fc);
int read_file(FILE *fp, f_client* fc, size_t size);

bool check_auth(f_client *fc);
bool authenticate(const char* id, const char* hash);
#endif