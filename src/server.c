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

static char*    prepare_path(char* _src, f_client *_fc, const char* _file);
static void     clean_client(f_client *_fc);

static int      f_push      (f_client* _fc, Instruction* _ins);
static int      f_get       (f_client* _fc, Instruction* _ins);
static int      f_rm        (f_client* _fc, Instruction* _ins);
static int      f_up        (f_client* _fc, Instruction* _ins);
static int      f_dir       (f_client* _fc, Instruction* _ins);
static int      f_auth      (f_client* _fc, Instruction* _ins);
static int      f_go        (f_client* _fc, Instruction* _ins);
static int      f_rev       (f_client* _fc, Instruction* _ins);
static int      f_path      (f_client* _fc, Instruction* _ins);
static int      f_mkfd      (f_client* _fc, Instruction* _ins);
static int      f_rmfd      (f_client* _fc, Instruction* _ins);

static int      send_data   (int _fd, BYTE* _buffer, size_t _n);
static int      read_data   (int _fd, BYTE* _buffer, size_t _n);
static int      send_file   (FILE *_fp, f_client* _fc);
static int      read_file   (FILE *_fp, f_client* _fc, size_t _size);




void 
start_server(int16_t _port, f_server* _server)
{
    memset(_server, 0, sizeof(f_server));
    _server->fc.auth = false;
    _server->port_num = _port;
    clean_client(&(_server->fc));

    //setup server struct
    struct sockaddr_in* sd = &(_server->server_addr);
    sd->sin_family = AF_INET;
    sd->sin_addr.s_addr = INADDR_ANY;
    sd->sin_port = htons(_port);

    //opens a new socket 
    //if return is -1 it failed
    //else it returns the file descriptor
    int* s_fd = &(_server->server_fd);
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
            (struct sockaddr*) &(_server->server_addr), 
            sizeof(struct sockaddr_in)) < 0){
        Log("Failed binding local address to the socket.");
        exit(EXIT_FAILURE);
    }
    Log("Successful bind to socket.");
}

void 
listen_server(f_server* _server)
{
    if (listen(_server->server_fd, 1) == -1) {
        Log("Error listening for client.");
    }
    Log("Listening for new client.");

    _server->fc.fd = accept(_server->server_fd,
                    (struct sockaddr *)&(_server->fc.addr),
                    &_server->fc.clilen);
    if (_server->server_fd < 0) {
        Log("Error accepting cliet.");
    }
    Log("Client with address: %s and file descriptor: %u successfully accepted.",
    inet_ntoa(_server->server_addr.sin_addr),
    (unsigned int)_server->fc.fd);
}

void 
server_IO(f_server* _fs)
{
    Log("Started listening for incoming instructions.");

    f_client*   fc = &(_fs->fc);
    Instruction ins = {0};
    BYTE        ins_buffer[INS_SIZE];
    int         n;
    while(1){
        //read instruction
        n = read_data(fc->fd, ins_buffer, INS_SIZE);

        //check in recieved instruction
        if(n == 0){
            close(fc->fd);
            clean_client(fc);
            Log("Client disconnected from the server.");
            return;
        }
        else if(n != INS_SIZE){
            Log("Instruction size was the incorrect size of %i", n);
        }

        //parse instruction
        ins = init_instruction(ins_buffer);
        if(!ins.valid) {
            Log("Error in instruction.");
            continue;
        }

        //check if user is not authenticated
        if(!(fc->auth) && ins.flag != if_AUTH) { 
            Log("Instruction denied because user is not authenticated.");
            continue; 
        }

        switch(ins.flag){
            case if_PUSH:{
                f_push(fc, &ins);
                break;
            }
            case if_GET:{
                f_get(fc, &ins);
                break;
            }
            case if_REM:{
                f_rm(fc, &ins);
                break;
            }
            case if_UP:{
                f_up(fc, &ins);
                break;
            }
            case if_DIR:{ 
                f_dir(fc, &ins);
                break;
            }
            case if_AUTH:{
                f_auth(fc, &ins);
                break;
            }
            case if_GO:{
                f_go(fc, &ins);
                break;
            }
            case if_REV:{
                f_rev(fc, &ins);
                break;
            }
            case if_PATH:{
                f_path(fc, &ins);
                break;
            }
            case if_MKFD: {
                f_mkfd(fc, &ins);
                break;
            }
            case if_RMFD: {
                f_rmfd(fc, &ins);
                break;
            }
            default:{
                break;
            }
        }
    }
}

void 
close_server(f_server *_fs)
{
    close(_fs->server_fd);
    close(_fs->fc.fd);
}




static void 
clean_client(f_client *_fc)
{
    memset(_fc, 0, sizeof(f_client));
    size_t root_len = strlen(ROOT);
    strcpy(_fc->f_directory, ROOT);
    _fc->fdir_len = root_len;
    _fc->root_end = root_len;
}


static int 
send_file(FILE *_fp, f_client* _fc)
{
    size_t  read_size   = FILE_CHUNK,
            n = 0;
    BYTE    file_buffer[FILE_CHUNK];
    
    while((n = fread(file_buffer, 1, read_size, _fp)) > 0){
        send_data(_fc->fd, file_buffer, n);
    }
    return 1;
}

static int 
read_file(FILE *_fp, f_client* _fc, size_t _size)
{
    BYTE    read_buffer[FILE_CHUNK];
    size_t  pack_size   = _size >= FILE_CHUNK ? FILE_CHUNK : _size,
            data_read   = 0,
            data_wrote  = 0;
    Log("Reading file with size: %u bytes or %u MB.", _size, _size / 1000);

    while(_size > 0){
        pack_size = _size < FILE_CHUNK ? _size : FILE_CHUNK;
        //read data
        int nn = read_data(_fc->fd,read_buffer, pack_size);
        if(nn == 0) { return 0; }
        else { data_read += nn; }
        //write to file
        data_wrote += fwrite(read_buffer, sizeof(BYTE), pack_size, _fp);
        _size = _size >= pack_size ? _size - pack_size : 0;
    }
    Log("Read %u bytes and wrote %u bytes to file", data_read, data_wrote);
    return 1;
}

static int 
send_data(int _fd, BYTE* _buffer, size_t _n)
{
    int     offset  = 0;
    size_t  sent    = 0;

    while((sent = send(_fd, _buffer + offset, _n, 0)) > 0 || sent == -1){
        if(sent > 0) {
            offset += sent;
            _n -= sent;
        }
    }  
    return (int)sent;
}

static int
read_data(int _fd, BYTE* _buffer, size_t _n)
{
    int read_b = 0;
    int result = 0;

    while (read_b < _n)
    {
        result = read(_fd, _buffer + read_b, _n - read_b);
        if(result == 0){
            return 0;
        }
        read_b += result;
    }
    return read_b;
}


static char* 
prepare_path(char* _src, f_client *_fc, const char* _file)
{
    size_t file_len   = strlen(_file);
    size_t dir_len    = _fc->fdir_len;

    //new path length is larger than 255 or file len is 0
    if((file_len + dir_len + 1 > BUFF_SIZE) || file_len == 0){
        return NULL;
    }

    memcpy(_src, _fc->f_directory, dir_len);
    memcpy(_src + dir_len, _file, file_len + 1);

    return _src;
}


//#################################################################

static int     
f_push(f_client* _fc, Instruction* _ins)
{
    char    path_buff[BUFF_SIZE];
    int     n;
    char    *temp_ptr = NULL;

    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) {
        return e_PATH;
    }

    _ins->fptr = fopen(path_buff, "w");
    if (_ins->fptr == NULL) {
        return e_FILE;
    }

    n = read_file(_ins->fptr, _fc, _ins->file_size);
    if (n == 0) {
        fclose(_ins->fptr);
        remove(_ins->arg0);
        return e_READ_FILE;
    }
    else {
        fclose(_ins->fptr);
        return 0;
    }
}

static int    
f_get(f_client* _fc, Instruction* _ins)
{
    BYTE        size_buff[UI32_B];
    char        path_buff[BUFF_SIZE];
    char*       temp_ptr = NULL;
    uint32_t    f_size;
    
    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) {
        return e_PATH;
    }

    _ins->fptr = fopen(path_buff, "rb");
    if (_ins->fptr == NULL) {
        return e_FILE;
    }

    //1. Send file size
    f_size = file_size(_ins->fptr);
    if(f_size == 0){
        fclose(_ins->fptr);
        return e_FILE_SIZE;
    }
    memset(size_buff, 0, UI32_B);
    memcpy(size_buff, &f_size, UI32_B);
    send_data(_fc->fd, size_buff, UI32_B);

    //2. Send actual file
    send_file(_ins->fptr, _fc);
    fclose(_ins->fptr);

    return 0;
}

static int     
f_rm(f_client* _fc, Instruction* _ins)
{
    char    path_buff[BUFF_SIZE];
    char*   temp_ptr = NULL;

    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) {
        return e_PATH;
    }

    if (remove(path_buff) == -1) {
        return e_REMOVE_FILE;
    }
    return 0;
}

static int     
f_up(f_client* _fc, Instruction* _ins)
{
    char path_buffe[BUFF_SIZE];
    char path_buffu[BUFF_SIZE];
    char* temp_eptr = NULL;
    char* temp_uptr = NULL;

    temp_eptr = prepare_path(path_buffe, _fc, _ins->arg0);
    temp_uptr = prepare_path(path_buffu, _fc, _ins->arg1);

    if (temp_eptr == NULL || temp_uptr == NULL) {
        return e_PATH;
    }

    if (remove(path_buffe) == -1) {
        return e_REMOVE_FILE;
    }

    _ins->fptr = fopen(path_buffu, "w");
    if (_ins->fptr == NULL) {
        return e_FILE;
    }

    if (read_file(_ins->fptr, _fc, _ins->file_size) == 0) {
        fclose(_ins->fptr);
        remove(path_buffu);
        return e_READ_FILE;
    }
    fclose(_ins->fptr);

    return 0;
}

static int
f_dir(f_client* _fc, Instruction* _ins)
{
    //FIXIT
    BYTE *buffer = NULL;
    size_t i = 0,
           len = 0,
           send_length = 0,
           cur_len = 0;
    BYTE buffer_len[UI32_B];
    memset(buffer_len, 0, 4);

    _ins->dirptr = opendir(_fc->f_directory);
    if (_ins->dirptr == NULL)
    {
        Log("Running instruction %s failed to open directory",
            get_ins_name(_ins->flag));
        return 1;
    }

    buffer = dir_contents(_ins);
    len = strlen((char *)buffer) + 1;
    cur_len = len;
    //send data length
    memcpy(buffer_len, &len, UI32_B);
    send_data(_fc->fd, buffer_len, 4);
    //send data
    while (i != len)
    {
        send_length = cur_len >= FILE_CHUNK ? FILE_CHUNK : cur_len;
        if (send_length == FILE_CHUNK)
        {
            cur_len -= FILE_CHUNK;
        }
        else
        {
            cur_len = 0;
        }
        send_data(_fc->fd, buffer + i, send_length);
        i += send_length;
    }

    if (buffer)
    {
        free(buffer);
    }
    closedir(_ins->dirptr);
    return 0;
}

int 
authenticate_u(const char* _id, const char* _hash)
{
    int     ID_AUTH = 1,
            PW_AUTH = 1;
    char    buffer[256] = {0};
    
    get_tag(buffer, ID_TAG);
    ID_AUTH = strcmp(buffer, _id);
    get_tag(buffer, PW_TAG);
    PW_AUTH = strcmp(buffer, _hash);
    
    return ID_AUTH && PW_AUTH;
}

static int     
f_auth(f_client* _fc, Instruction* _ins)
{
    //FIXIT
    BYTE buffer[5] = {0x7F, 0x3E, 0x24, 0x55};
    BYTE logged_in_flag = 0xFF;
    BYTE logged_out_flag = 0x00;

    if (!(_fc->auth))
    {
        if (authenticate_u(_ins->arg0, _ins->arg1))
        {
            _fc->auth = true;
            buffer[4] = logged_in_flag;
            Log("User %s is authenticated.", _ins->arg0);
        }
        else
        {
            buffer[4] = logged_out_flag;
            Log("Cannot authenticate user %s.", _ins->arg0);
        }
    }
    else
    {
        buffer[4] = logged_in_flag;
    }
    send_data(_fc->fd, buffer, 5);
    return 0;
}

static int     
f_go(f_client* _fc, Instruction* _ins)
{
    char            buffer[BUFF_SIZE];
    size_t          dir_len = strlen(_ins->arg0);
    const char*     slash   = "/\0";
    size_t          len     = 0;
    int             valid;

    //check if size wont reach higher of 255 or dir length is 0
    if(dir_len == 0) {
        return e_PATH_ZERO_LEN;
    }
    if(dir_len + _fc->fdir_len + 2 > BUFF_SIZE) {
        return e_PATH_OVERFLOW;
    }

    //prepare new path to buffer
    memcpy(buffer, _fc->f_directory, _fc->fdir_len);
    len = _fc->fdir_len;
    memcpy(buffer + len, _ins->arg0, dir_len);
    len += dir_len;
    memcpy(buffer + len, slash, 2);
    len += 1;

    //check if dir is valid
    valid = dir_valid(buffer);
    if(valid == 1) {
        memcpy(_fc->f_directory, buffer, len+1);
        _fc->fdir_len = len;

        return 0;
    } else {
        return e_FOLDER_NOT_FOUND;
    }
}

static int     
f_rev(f_client* _fc, Instruction* _ins)
{
    char buffer[BUFF_SIZE];
    size_t dir_len      = _fc->fdir_len;
    int counter         = 0;
    int valid;
    int buffer_len;

    //if current directory is root dont go back
    if(_fc->fdir_len == _fc->root_end){
        return e_FOLDER;
    }

    //go one directory back
    memcpy(buffer, _fc->f_directory, dir_len + 1);
    for(int i = dir_len; i >= _fc->root_end - 1; i--){
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
        memcpy(_fc->f_directory, buffer, buffer_len + 1);
        _fc->fdir_len = buffer_len;

        return 0;
    } else {
        return e_FOLDER_NOT_FOUND;
    }
}

static int     
f_path(f_client* _fc, Instruction* _ins)
{
    BYTE size_buff[UI32_B];
    uint32_t size = (_fc->fdir_len) + 1;

    memcpy(size_buff, &(size), UI32_B);
    send_data(_fc->fd, size_buff, UI32_B);
    send_data(_fc->fd, (BYTE *)_fc->f_directory, _fc->fdir_len);

    return 0;
}

static int     
f_mkfd(f_client* _fc, Instruction* _ins)
{
    char buffer[BUFF_SIZE];

    if(prepare_path(buffer, _fc, _ins->arg0) == NULL){
        return e_PATH;
    }
    int result = mkdir(buffer, 0700);

    //FIXIT
    return result;
}

static int     
f_rmfd(f_client* _fc, Instruction* _ins)
{
    char buffer[BUFF_SIZE];

    if(prepare_path(buffer, _fc, _ins->arg0) == NULL){
        return e_PATH;
    }
    int result = remove_directory(buffer);

    //FIXIT
    return result;
}

