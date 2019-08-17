#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "file_manager.h"

#define PORT_N 1553
void 
send_file(FILE *fp, int fd);

void 
read_file(FILE *fp, int fd);

void 
send_data(int fd, BYTE* buffer, size_t n);

void
read_data(int fd, BYTE* buffer, size_t n);

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno = PORT_N, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    FILE* fp = fopen("/home/duler/Desktop/Root/test.txt", "rb");
    printf("Sockfd %i\n", sockfd);
    send_file(fp, sockfd);
    

    
    close(sockfd);
    return 0;
}

void 
send_file(FILE *fp, int fd){
    long file_len = file_size(fp);
    size_t n = 0, sent = 0;
    BYTE file_buffer[FILE_CHUNK];
    BYTE inter_buffer[5];

    while((n = fread(file_buffer+4, sizeof(BYTE), FILE_CHUNK-4, fp)) > 0){
        parse_num(file_buffer, n);
        send_data(fd, file_buffer, n+4);
        read_data(fd, inter_buffer, 5);
        //read intermediate if data was recieved
        if(read_intermediate(inter_buffer) != flag_GO){
            error("Error from client side");
        }

        memset(file_buffer, 0, FILE_CHUNK);
        memset(inter_buffer, 0, 5);
    }
    prepare_intermediate(inter_buffer, flag_DONE);
    send_data(fd, inter_buffer, 5);
}

void 
read_file(FILE *fp, int fd){
    BYTE buffer5B[5];
    BYTE buffer1020B[FILE_CHUNK-4];
    uint32_t pack_size = 0;

    //read first 4 bytes and get packet size 
    while(1){
        read_data(fd,buffer5B, 5);
        pack_size = parse_4_bytearr(buffer5B);
        //check if file is done sending
        if(pack_size == INTERMEDIATE_NUM){ 
            read_data(fd,buffer5B[3], 1);
            if(read_intermediate(buffer5B) == flag_DONE){
                break;
            }
            else{
                error("Error in reading file");
            }
        }
        //read data
        read_data(fd,buffer1020B, pack_size);
        fwrite(buffer1020B, sizeof(BYTE), pack_size, fp);
        //write intermadiate
        prepare_intermediate(buffer5B, flag_GO);

        memset(buffer1020B, 0, FILE_CHUNK - 4);
        memset(buffer5B, 0, 5);
    }
}

void 
send_data(int fd, BYTE* buffer, size_t n){
    int offset = 0;
    size_t sent = 0;
    while((sent = send(fd, buffer + offset, n, 0)) > 0 || sent == -1){
        if(sent > 0){
            offset += sent;
            n -= sent;
        }
    }  
}

void
read_data(int fd, BYTE* buffer, size_t n){
    int read_b = 0;
    int result;
    while (read_b < n)
    {
        result = read(fd, buffer + read_b, n - read_b);
        read_b += result;
    }
}