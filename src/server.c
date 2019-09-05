#include "server.h"
#include "logger.h"
#include "config_reader.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void 
start_server(int16_t port, f_server *server){
    memset(server, 0, sizeof(f_server));
    server->fc.authenticated = false;
    server->port_num = port;
    server->close_server = close_server;
    server->listen_server = listen_server;
    clean_client(&(server->fc));

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
        
        Log("Error opening socket on address: %s with port: %u",
        inet_ntoa(sd->sin_addr),
        (unsigned int)sd->sin_port);
        exit(EXIT_FAILURE);
    }
    Log("Opened socket on address: %s with port: %u.", 
    inet_ntoa(sd->sin_addr),
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

    if (listen(serverp->server_fd, 1) == -1) {
        Log("Error listening for client.");
    }
    Log("Listening for new client.");

    serverp->fc.fd = accept(serverp->server_fd,
                    (struct sockaddr *)&(serverp->fc.addr),
                    &serverp->fc.clilen);
    if (serverp->server_fd < 0) {
        Log("Error accepting cliet.");
    }
    Log("Client with address: %s and file descriptor: %u successfully accepted.",
    inet_ntoa(serverp->server_addr.sin_addr),
    (unsigned int)serverp->fc.fd);
}

void 
server_IO(f_server* fs){
    f_client* fc = &(fs->fc);
    Log("Started listening for incoming instructions.");
    Instruction ins = {0};
    BYTE ins_buffer[206] = {0};
    while(1){
        //read instruction
        int n = read_data(fc->fd, ins_buffer, 206);

        //check in recieved instruction
        if(n == 0){
            close(fc->fd);
            clean_client(&(fs->fc));
            Log("Client disconnected from the server.");
            return;
        }
        else if(n != 206){
            Log("Instruction size was the incorrect size of %i", n);
        }

        //parse instruction
        ins = init_instruction(ins_buffer);
        if(!ins.valid) {
            Log("Error in instruction.");
            continue;
        }

        //check if user is not authenticated
        if(!check_auth(fc) && ins.flag != if_AUTH) { 
            Log("Instruction denied because user is not authenticated.");
            continue; 
        }

        switch(ins.flag){
            case if_PUSH:{
                char buffer[255];
                char* temp = prepare_path(buffer, fc, ins.arg0);
                if(temp == NULL){
                    Log("ERROR if_PUSH."); //fix
                    break;
                }
                ins.fptr = fopen(buffer, "w");
                if(ins.fptr == NULL) { 
                    Log("Running instruction %s failed to open file %s", 
                        get_ins_name(ins.flag), ins.arg0);
                    continue;
                }
                
                int n = read_file(ins.fptr, fc, ins.file_size);
                if(n == 0){
                    Log("Failed reading file because client closed mid send.");
                    fclose(ins.fptr);
                    remove(ins.arg0);
                } else { 
                    fclose(ins.fptr);
                }
                break;
            }
            case if_GET:{
                BYTE s_buff[4] = {0};
                memset(s_buff, 0, 4);

                char buffer[255];
                char* temp = prepare_path(buffer, fc, ins.arg0);
                if(temp == NULL){
                    Log("ERROR if_PUSH."); //fix
                    break;
                }

                ins.fptr = fopen(buffer, "rb");
                if(ins.fptr == NULL) { 
                    Log("Running instruction %s failed to open file %s", 
                        get_ins_name(ins.flag), ins.arg0);
                    continue;
                }
                //send 4BYTES of file size
                size_t size = file_size(ins.fptr);
                parse_num(s_buff, size);
                send_data(fc->fd, s_buff, 4);
                //send actual file
                send_file(ins.fptr, fc);
                fclose(ins.fptr);
                break;
            }
            case if_REM:{
                char buffer[255];
                char* temp = prepare_path(buffer, fc, ins.arg0);
                if(temp == NULL){
                    Log("ERROR if_REM."); //fix
                    break;
                }

                int n = remove(buffer);
                if(n == -1){
                    Log("Failed to remove file %s.", ins.arg0);
                } else {
                }
                break;
            }
            case if_UP:{
                char buffer_e[255];
                char buffer_u[255];
                char* temp_e = prepare_path(buffer_e, fc, ins.arg0);
                char* temp_u = prepare_path(buffer_u, fc, ins.arg1);
                if(temp_e == NULL || temp_u == NULL){
                    Log("ERROR if_REM."); //fix
                    break;
                }

                int n = remove(buffer_e);
                if(n == -1){
                    Log("Failed to remove file %s.", ins.arg0);
                }

                ins.fptr = fopen(buffer_u,"w");
                if(ins.fptr == NULL) { 
                    Log("Running instruction %s failed to open file %s", 
                        get_ins_name(ins.flag), ins.arg0);
                    continue;
                }

                int nn = read_file(ins.fptr, fc, ins.file_size);
                if(nn == 0){
                    Log("Failed reading file because client closed mid send.");
                    fclose(ins.fptr);
                    remove(buffer_u);
                } else { 
                    fclose(ins.fptr);
                }
                break;
            }
            case if_DIR:{ 
                BYTE*   buffer          = NULL;
                size_t  i               = 0, 
                        len             = 0, 
                        send_length     = 0,
                        cur_len         = 0;
                BYTE    buffer_len[4];
                memset(buffer_len, 0 , 4);

                
                ins.dirptr = opendir(fc->f_directory);
                if(ins.dirptr == NULL) { 
                    Log("Running instruction %s failed to open directory", 
                        get_ins_name(ins.flag));
                    continue;
                }

                buffer = get_dir(&ins);
                len = strlen((char*)buffer) + 1;
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
            case if_AUTH:{
                //random numbers 
                BYTE buffer[5] = { 0x7F, 0x3E, 0x24, 0x55 };
                BYTE logged_in_flag     = 0xFF;
                BYTE logged_out_flag    = 0x00;

                if(!check_auth(fc)){
                    if(authenticate(ins.arg0, ins.arg1)){
                        fc->authenticated = true;
                        buffer[4] = logged_in_flag;
                        Log("User %s is authenticated.", ins.arg0);
                    } else {
                        buffer[4] = logged_out_flag;
                        Log("Cannot authenticate user %s.", ins.arg0);
                    }
                } else {
                    buffer[4] = logged_in_flag;
                }
                send_data(fc->fd, buffer, 5);
                break;
            }
            case if_GO:{
                if(go_dir(fc, ins.arg0)){
                    Log("Successful switching to %s", fc->f_directory);
                } else {
                    Log("Unsucessful GO instruction.");
                }
                break;
            }
            case if_REV:{
                if(rev_dir(fc)){
                    Log("Successful reversing to %s", fc->f_directory);
                } else {
                    Log("Unsucessful REV instruction.");
                }
                break;
            }
            case if_PATH:{
                BYTE size_buff[4];
                parse_num(size_buff, fc->fdir_len + 1);
                send_data(fc->fd, size_buff, 4);
                send_data(fc->fd, (BYTE*)fc->f_directory, fc->fdir_len + 1);
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

void 
clean_client(f_client *fc){
    memset(fc, 0, sizeof(f_client));
    size_t root_len = strlen(ROOT);
    strcpy(fc->f_directory, ROOT);
    fc->fdir_len = root_len;
    fc->root_end = root_len;
}


int 
send_file(FILE *fp, f_client* fc){
    size_t  read_size   = FILE_CHUNK,
            n = 0;
    BYTE    file_buffer[FILE_CHUNK];
    
    while((n = fread(file_buffer, 1, read_size, fp)) > 0){
        send_data(fc->fd, file_buffer, n);
    }
    return 1;
}

int 
read_file(FILE *fp, f_client* fc, size_t size){
    BYTE    read_buffer[FILE_CHUNK];
    size_t  pack_size   = size >= FILE_CHUNK ? FILE_CHUNK : size,
            data_read   = 0,
            data_wrote  = 0;
    Log("Reading file with size: %u bytes or %u MB.", size, size / 1000);

    while(size > 0){
        pack_size = size < FILE_CHUNK ? size : FILE_CHUNK;
        //read data
        int nn = read_data(fc->fd,read_buffer, pack_size);
        if(nn == 0) { return 0; }
        else { data_read += nn; }
        //write to file
        data_wrote += fwrite(read_buffer, sizeof(BYTE), pack_size, fp);
        size = size >= pack_size ? size - pack_size : 0;
    }
    Log("Read %u bytes and wrote %u bytes to file", data_read, data_wrote);
    return 1;
}

int 
send_data(int fd, BYTE* buffer, size_t n){
    int     offset  = 0;
    size_t  sent    = 0;

    while((sent = send(fd, buffer + offset, n, 0)) > 0 || sent == -1){
        if(sent > 0) {
            offset += sent;
            n -= sent;
        }
    }  
    return (int)sent;
}

int
read_data(int fd, BYTE* buffer, size_t n){
    int read_b = 0;
    int result = 0;

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


bool 
check_auth(f_client *fc){
    return fc->authenticated ? true : false;
}

bool 
authenticate(const char* id, const char* hash){
    bool ID_AUTH = false,
         PW_AUTH = false;

    char    buffer[256] = {0};
    get_tag(buffer, ID_TAG);
    if(strcmp(buffer, id) == 0) { ID_AUTH = true; }
    get_tag(buffer, PW_TAG);
    if(strcmp(buffer, hash) == 0) { PW_AUTH = true; }
    
    return ID_AUTH && PW_AUTH;
}


bool 
go_dir(f_client *fc, const char* dir){
    char            buffer[255];
    size_t          dir_len = strlen(dir);
    const char*     slash   = "/\0";
    size_t          len     = 0;
    int             valid;

    //check if size wont reach higher of 255 or dir length is 0
    if(dir_len == 0){
        Log("Directory length is 0.");
        return false;
    }
    if(dir_len + fc->fdir_len + 2 > 255){
        Log("Directory size is larger than 255 bytes.");
        return false;
    }

    //prepare new path to buffer
    memcpy(buffer, fc->f_directory, fc->fdir_len);
    len = fc->fdir_len;
    memcpy(buffer + len, dir, dir_len);
    len += dir_len;
    memcpy(buffer + len, slash, 2);
    len += 1;

    //check if dir is valid
    valid = dir_valid(buffer);
    if(valid == 1) {
        memcpy(fc->f_directory, buffer, len+1);
        fc->fdir_len = len;
        return true;
    } else {
        return false;
    }
}

bool 
rev_dir(f_client *fc){
    char buffer[255];
    size_t dir_len      = fc->fdir_len;
    int counter         = 0;
    int valid;
    int buffer_len;

    //if current directory is root dont go back
    if(fc->fdir_len == fc->root_end){
        return false;
    }

    //go one directory back
    memcpy(buffer, fc->f_directory, dir_len + 1);
    for(int i = dir_len; i >= fc->root_end - 1; i--){
        if(buffer[i] == '/') {
            if(counter == 1){
                buffer[i + 1] = '\0';
                break;
            }
            counter++;
        }
    }

    //check if new buffer is valid
    valid = dir_valid(buffer);
    if(valid == 1) {
        buffer_len = strlen(buffer);
        memcpy(fc->f_directory, buffer, buffer_len + 1);
        fc->fdir_len = buffer_len;
        return true;
    } else {
        return false;
    }
}


bool 
make_folder(f_client *fc, const char* folder){
    char buffer[255];
    char* temp;
    int result;

    temp = prepare_path(buffer, fc, folder);
    if(temp == NULL){
        return false;
    }
    result = mkdir(buffer, 0700);

    return result == 0 ? true : false;
}

bool 
remove_folder(f_client *fc, const char* folder){
    char buffer[255];
    char* temp;
    int result;

    temp = prepare_path(buffer, fc, folder);
    if(temp == NULL){
        return false;
    }
    result = 0; /*all_rem(buffer);*/ printf("TODO -> REMOVE_FOLDER server.c");

    return result == 0 ? true : false;
}


char* 
prepare_path(char* src, f_client *fc, const char* file){
    size_t file_len   = strlen(file);
    size_t dir_len      = fc->fdir_len;

    //new path length is larger than 255 or file len is 0
    if((file_len + dir_len + 1 > 255) || file_len == 0){
        return NULL;
    }

    memcpy(src, fc->f_directory, dir_len);
    memcpy(src + dir_len, file, file_len + 1);

    return src;
}

