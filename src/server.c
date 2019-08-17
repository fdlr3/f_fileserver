#include "server.h"
#include "logger.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>

static void 
error(const char *msg)
{
    perror(msg);
}

void 
start_server(int16_t port, f_server *server){
    memset(server, 0, sizeof(f_server));
    server->port_num = port;
    server->close_server = close_server;
    server->listen_server = listen_server;

    //setup server struct
    struct sockaddr_in* sd = &server->server_addr;
    sd->sin_family = AF_INET;
    sd->sin_addr.s_addr = INADDR_ANY;
    sd->sin_port = htons(port);

    //opens a new socket 
    //if return is -1 it failed
    //else it returns the file descriptor
    int* s_fd = &(server->server_fd);
    *s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*s_fd < 0){
        Log("Error opening socket on address: %u with port: %u",
        (unsigned int)sd->sin_addr.s_addr,
        (unsigned int)sd->sin_port);
        exit(EXIT_FAILURE);
    }
    Log("Opened socket on address: %u with port: %u.", 
    (unsigned int)sd->sin_addr.s_addr,
    (unsigned int)sd->sin_port);

    //binds the local address to the socket
    if(bind(
            *s_fd,
            (struct sockaddr*) &server->server_addr, 
            sizeof(struct sockaddr_in)) < 0){
        Log("Failed binding local address to the socket.");
        exit(EXIT_FAILURE);
    }
    Log("Successful bind to socket.");
}

void 
listen_server(void *server){
    f_server *serverp = server;

    Log("Listening for new client.");
    //if return -1 false, else true
    if (listen(serverp->server_fd, 1) == -1) {
        Log("Error listening for client.");
    }
    Log("Client listen successful.");
    //accepts connections and sets a new file descriptor "newsockfd"
    //if return is -1 its an error
    serverp->fc.fd = accept(serverp->server_fd,
                    (struct sockaddr *)&(serverp->fc.addr),
                    &serverp->fc.clilen);
    if (serverp->server_fd < 0) {
        Log("Error accepting cliet.");
    }
    Log("Client with address: %u and file descriptor: %u successfully accepted.",
    (unsigned int)serverp->fc.addr.sin_addr.s_addr,
    (unsigned int)serverp->fc.fd);
}

void 
server_IO(f_client* fc){
    Log("Started listening for incoming instructions.");
    Instruction ins = {0};
    BYTE ins_buffer[206] = {0};
    while(1){
        int n = read_data(fc->fd, ins_buffer, 206);
        //if client socked closed
        if(n == 0){
            close(fc->fd);
            memset(fc, 0, sizeof(f_client));
            Log("Client disconnected from the server.");
            return;
        }
        ins = init_instruction(ins_buffer);

        switch(ins.flag){
            case if_PUSH:{
                ins.fptr = fopen(ins.arg0,"w");
                if(ins.fptr == NULL) { exit(EXIT_FAILURE); /*TODO ERROR */}
                read_file(ins.fptr, fc, ins.file_size);
                fclose(ins.fptr);
                break;
            }
            case if_GET:{
                BYTE buffer[4] = {0};
                ins.fptr = fopen(ins.arg0, "rb");
                if(ins.fptr == NULL) { exit(EXIT_FAILURE); /*TODO ERROR */}
                //send 4BYTES of file size
                size_t size = file_size(ins.fptr);
                parse_num(buffer, size);
                send_data(fc->fd, buffer, 4);
                //send actual file
                send_file(ins.fptr, fc);
                fclose(ins.fptr);
                break;
            }
            case if_REM:{
                remove(ins.arg0);
                break;
            }
            case if_UP:{
                remove(ins.arg0);
                ins.fptr = fopen(ins.arg1,"w");
                if(ins.fptr == NULL) { exit(EXIT_FAILURE); /*TODO ERROR */}
                read_file(ins.fptr, fc, ins.file_size);
                fclose(ins.fptr);
                break;
            }
            case if_DIR:{
                BYTE* buffer = NULL;
                BYTE buffer_len[4] = {0};
                size_t i = 0, len = 0, send_length = 0,
                cur_len = 0;
                ins.dirptr = opendir(ROOT);
                if(ins.dirptr == NULL) { exit(EXIT_FAILURE); /*TODO ERROR */}
                buffer = get_dir(&ins);
                len = strlen(buffer);
                cur_len = len;
                //send data length
                parse_num(buffer_len, len);
                send_data(fc->fd, buffer_len, 4);
                //send data
                while(i != len){
                    send_length = cur_len >= FILE_CHUNK ? FILE_CHUNK : cur_len;
                    if(send_length == FILE_CHUNK) {
                        cur_len -= FILE_CHUNK;
                    } else {
                        cur_len = 0;
                    }
                    send_data(fc->fd, buffer + i,send_length);
                    i += send_length;
                }

                if(buffer) {
                    free(buffer);
                }
                closedir(ins.dirptr);
                break;
            }
            default:{
                break;
            }
        }
    }
}

void 
close_server(void *server){
    f_server *serverp = server;
    close(serverp->server_fd);
    close(serverp->fc.fd);
}

int 
send_file(FILE *fp, f_client* fc){
    size_t read_size = FILE_CHUNK;
    size_t n = 0;
    BYTE file_buffer[FILE_CHUNK];
    
    while((n = fread(file_buffer, 1, read_size, fp)) > 0){
        send_data(fc->fd, file_buffer, n);
    }
    return 1;
}

int 
read_file(FILE *fp, f_client* fc, size_t size){
    BYTE read_buffer[FILE_CHUNK];
    size_t pack_size = size >= FILE_CHUNK ? FILE_CHUNK : size;

    while(size > 0){
        pack_size = size < FILE_CHUNK ? size : FILE_CHUNK;
        //read data
        int nn = read_data(fc->fd,read_buffer, pack_size);
        if(nn == 0) { return 0; }
        //write to file
        size_t n = fwrite(read_buffer, sizeof(BYTE), pack_size, fp);
        size = size >= pack_size ? size - pack_size : 0;
    }
    return 1;
}

int 
send_data(int fd, BYTE* buffer, size_t n){
    int offset = 0;
    size_t sent = 0;
    while((sent = send(fd, buffer + offset, n, 0)) > 0 || sent == -1){
        if(sent > 0){
            offset += sent;
            n -= sent;
        }
    }  
    return (int)sent;
}

int
read_data(int fd, BYTE* buffer, size_t n){
    int read_b = 0;
    int result;
    while (read_b < n)
    {
        result = read(fd, buffer + read_b, n - read_b);
        if(result == 0){
            return 0;
        }
        read_b += result;
    }
    return read_b;
}

