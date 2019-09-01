#ifndef _SERVER_H_
#define _SERVER_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "file_manager.h"

typedef struct{
    int fd;
    struct sockaddr_in addr;
    socklen_t clilen;
    bool authenticated;

    //current directory path
    char f_directory[255];
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
    //f_pointer to run and close server
    void(* listen_server)   (void*);
    void(* close_server)    (void*);
} f_server;

void start_server(int16_t port, f_server *server);
void listen_server(void *server);
void server_IO(f_server *fs);
void close_server(void *server);
void clean_client(f_client *fc);

int send_data(int fd, BYTE* buffer, size_t n);
int read_data(int fd, BYTE* buffer, size_t n);
int send_file(FILE *fp, f_client* fc);
int read_file(FILE *fp, f_client* fc, size_t size);

bool check_auth(f_client *fc);
bool authenticate(const char* id, const char* hash);

bool go_dir(f_client *fc, const char* dir);
bool rev_dir(f_client *fc);

bool folder_action(f_client *fc, const char* folder);
bool remove_folder(f_client *fc, const char* folder);

char* prepare_path(char* src, f_client *fc, const char* file);

#endif