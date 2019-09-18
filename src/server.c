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
#include <netdb.h>

static char*    prepare_path(char* _src, f_client *_fc, const char* _file);
static void     clean_client(f_server *_fs);
static int      send_resp   (f_client* _fc, uint8_t _flag);

//f methods -> 1 = SUCCESS, 0 = CLIENT DISCONNECT, -1 = FAIL
static int      f_push      (f_server* _fs, Instruction* _ins);
static int      f_get       (f_server* _fs, Instruction* _ins);
static int      f_rm        (f_server* _fs, Instruction* _ins);
static int      f_dir       (f_server* _fs, Instruction* _ins);
static int      f_auth      (f_server* _fs, Instruction* _ins);
static int      f_go        (f_server* _fs, Instruction* _ins);
static int      f_rev       (f_server* _fs, Instruction* _ins);
static int      f_path      (f_server* _fs, Instruction* _ins);
static int      f_mkfd      (f_server* _fs, Instruction* _ins);
static int      f_rmfd      (f_server* _fs, Instruction* _ins);

static int      send_data   (int _fd, BYTE* _buffer, size_t _n);
static int      read_data   (int _fd, BYTE* _buffer, size_t _n);
static int      send_file   (FILE *_fp, f_client* _fc);
static int      read_file   (FILE *_fp, f_client* _fc, size_t _size);




void 
start_server (f_server* _server, const char* _port, 
             const char* _hostname, const char* _conf_path)
{
    struct hostent* he;
    int n;

    memset(_server, 0, sizeof(f_server));

    if(!_port || !_hostname || !_conf_path){
        perr("Error reading char** argv. Exiting..");
    }

    //set config path
    n = file_valid(_conf_path);
    if(n == -1){
        perr("Error config path is invalid. Exiting..");
    }
    strncpy(_server->config_path, _conf_path, BUFF_SIZE);

    //set port
    _server->port_num = atoi(_port);
    if(_server->port_num == 0){
        perr("Error in parsing port number. Exiting..");
    }
    //set hostname
    if ((he = gethostbyname(_hostname) ) == NULL ) {
        perr("Error in hostname. Exiting..");
    }
    //set root folder
    get_tag(_server->config_path,_server->fc.f_directory, ROOTPATH_TAG);
    n = dir_valid(_server->fc.f_directory);
    if(n == -1){
        perr("Error root path doesnt exist. Exiting..");
    }
    _server->fc.fdir_len = strlen(_server->fc.f_directory);
    strcpy(_server->root_path, _server->fc.f_directory);

    //set logs folder
    get_tag(_server->config_path, _server->logs_path, LOGPATH_TAG);
    n = dir_valid(_server->logs_path);
    if(n == -1){
        perr("Error log path doesnt exist. Exiting..");
    }
    
    //start logger
    start_logger(_server->logs_path);

    //setup server struct
    struct sockaddr_in* sd = &(_server->server_addr);
    sd->sin_family = AF_INET;
    memcpy
    (
        &(_server->server_addr.sin_addr), 
        he->h_addr_list[0], 
        he->h_length
    );
    sd->sin_port = htons(_server->port_num);

    //opens a new socket 
    int* s_fd = &(_server->server_fd);
    *s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*s_fd < 0){
        perr("Error opening socket. Exiting..");
    }

    //binds the local address to the socket
    if(bind(*s_fd,
            (struct sockaddr*) &(_server->server_addr), 
            sizeof(struct sockaddr_in)) < 0
        ){
        perr("Error binding local address to the socket. Exiting..");
    }
    Log("Opening and biding the socket was a success.");
    Log("\nAddress: %s\nPort: %u\nRoot path: %s\nLog path: %s",
        inet_ntoa(sd->sin_addr), _server->port_num, 
        _server->fc.f_directory, _server->logs_path);
}

void 
listen_server(f_server* _server)
{
    if (listen(_server->server_fd, 1) == -1) {
        perr("Error listening for client.");
    }
    Log("Listening for new client.");

    _server->fc.fd = accept
    (
        _server->server_fd,
        (struct sockaddr *)&(_server->fc.addr),
        &_server->fc.clilen
    );
    if (_server->server_fd < 0) {
        perr("Error accepting cliet.");
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
    Instruction ins;
    BYTE        ins_buffer[INS_SIZE];
    int         n;
    int         result;
    while(1){
        //read instruction
        n = read_data(fc->fd, ins_buffer, INS_SIZE);

        //check in recieved instruction
        if(n == 0){
            close(fc->fd);
            clean_client(_fs);
            Log("Client disconnected from the server.");
            return;
        }

        //parse instruction
        n = init_instruction(&ins, ins_buffer);
        if(n == -1) {
            Log("ERROR in instruction");
            continue;
        }

        switch(ins.flag){
            case if_PUSH:{
                result = f_push(_fs, &ins);
                break;
            }
            case if_GET:{
                result = f_get(_fs, &ins);
                break;
            }
            case if_REM:{
                result = f_rm(_fs, &ins);
                break;
            }
            case if_DIR:{ 
                result = f_dir(_fs, &ins);
                break;
            }
            case if_AUTH:{
                result = f_auth(_fs, &ins);
                break;
            }
            case if_GO:{
                result = f_go(_fs, &ins);
                break;
            }
            case if_REV:{
                result = f_rev(_fs, &ins);
                break;
            }
            case if_PATH:{
                result = f_path(_fs, &ins);
                break;
            }
            case if_MKFD: {
                result = f_mkfd(_fs, &ins);
                break;
            }
            case if_RMFD: {
                result = f_rmfd(_fs, &ins);
                break;
            }
            default:{
                break;
            }
        }

        //error in execution (-1)
        if(result == -1 && 
                !(ins.flag == if_PUSH || ins.flag == if_AUTH)){
            n = send_resp(fc, FAIL);
            if (n == 0) {
                result = 0;
            }
        //success in execution (1)
        } else if(result == 1 && 
                !(ins.flag == if_PUSH || ins.flag == if_AUTH)){
            n = send_resp(fc, SUCCESS);
            if (n == 0) {
                result = 0;
            }
        }
        //client disconnected (0)
        if(result == 0){
            close(fc->fd);
            clean_client(_fs);
            Log("Client disconnected from the server.");
            return;
        }
    }
}

void 
close_server(f_server *_fs)
{
    shutdown(_fs->server_fd, SHUT_RDWR);
    close(_fs->fc.fd);
    close(_fs->server_fd);
}

static void 
clean_client(f_server *_fs)
{
    f_client* _fc = &(_fs->fc);
    memset(_fc, 0, sizeof(f_client));
    size_t root_len = strlen(_fs->root_path);
    strcpy(_fc->f_directory, _fs->root_path);
    _fc->fdir_len = root_len;
    _fc->root_end = root_len;
}

//#################################################################

static int     
f_push(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char    path_buff[BUFF_SIZE];
    int     n;
    char*   temp_ptr = NULL;

    if(!_fc->auth) return -1;

    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) return -1;

    _ins->fptr = fopen(path_buff, "w");
    if (_ins->fptr == NULL) return -1;

    //1.Send confirmation
    n = send_resp(_fc, SUCCESS);
    if (n == 0) {
        fclose(_ins->fptr);
        return 0;
    }


    //2.Read file
    n = read_file(_ins->fptr, _fc, _ins->file_size);
    if (n == 0) {
        fclose(_ins->fptr);
        remove(_ins->arg0);
        return 0;
    }
    else {
        fclose(_ins->fptr);
        return 1;
    }
}

static int    
f_get(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    BYTE        size_buff[UI32_B];
    char        path_buff[BUFF_SIZE];
    char*       temp_ptr = NULL;
    uint32_t    f_size;

    if(!_fc->auth) return -1;
    
    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) return -1;

    _ins->fptr = fopen(path_buff, "rb");
    if (_ins->fptr == NULL) return -1;

    f_size = file_size(_ins->fptr);
    if(f_size == 0 || f_size > MAX_FILE_SIZE){
        fclose(_ins->fptr);
        return -1;
    }

    //1. Send confirmation TODO

    //2. send file size
    memcpy(size_buff, &f_size, UI32_B);
    send_data(_fc->fd, size_buff, UI32_B);

    //3. Send file
    send_file(_ins->fptr, _fc);
    fclose(_ins->fptr);

    return 1;
}

static int     
f_rm(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char    path_buff[BUFF_SIZE];
    char*   temp_ptr = NULL;

    if(!_fc->auth) return -1;

    temp_ptr = prepare_path(path_buff, _fc, _ins->arg0);
    if (temp_ptr == NULL) return -1;

    if (remove(path_buff) == -1) return -1;
    return 1;
}

static int
f_dir(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    BYTE *buffer = NULL;
    size_t i = 0,
           len = 0,
           send_length = 0,
           cur_len = 0;
    BYTE buffer_len[UI32_B];

    if(!_fc->auth) return -1;

    _ins->dirptr = opendir(_fc->f_directory);
    if (_ins->dirptr == NULL) return -1;

    buffer = dir_contents(_ins);
    if(!buffer) return -1;

    //send data length
    len = strlen((char *)buffer) + 1;
    memcpy(buffer_len, &len, UI32_B);
    send_data(_fc->fd, buffer_len, 4);

    //send data
    cur_len = len;
    while (i != len) {
        send_length = cur_len >= FILE_CHUNK ? FILE_CHUNK : cur_len;
        if (send_length == FILE_CHUNK) {
            cur_len -= FILE_CHUNK;
        }
        else {
            cur_len = 0;
        }
        send_data(_fc->fd, buffer + i, send_length);
        i += send_length;
    }

    if (buffer) {
        free(buffer);
    }
    closedir(_ins->dirptr);
    return 1;
}

static int     
f_auth(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    if (!(_fc->auth)) {
        if (auth_user(_fs->config_path, _ins->arg0, _ins->arg1)) {
            _fc->auth = true;
            //setup key
            uint32_t key = rnd_key();
            memcpy(_fc->key, &key, UI32_B);

            send_resp(_fc, SUCCESS);
            Log("User %s is authenticated.", _ins->arg0);
        } else {
            send_resp(_fc, FAIL);
            Log("Cannot authenticate user %s.", _ins->arg0);
        }
    } else {
        send_resp(_fc, SUCCESS);
    }
    return 1;
}

static int     
f_go(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char            buffer[BUFF_SIZE];
    size_t          dir_len = strlen(_ins->arg0);
    const char*     slash   = "/\0";
    size_t          len     = 0;
    int             valid;

    if(!_fc->auth) return -1;

    //check if size wont reach higher of 255 or dir length is 0
    if((dir_len + _fc->fdir_len + 2 > BUFF_SIZE) || dir_len == 0) return -1;

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
        return 1;
    } else return -1;
}

static int     
f_rev(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char buffer[BUFF_SIZE];
    size_t dir_len      = _fc->fdir_len;
    int counter         = 0;
    int valid;
    int buffer_len;

    if(!_fc->auth) return -1;

    //if current directory is root dont go back
    if(_fc->fdir_len == _fc->root_end) return -1;

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

        return 1;
    } else return -1;
}

static int     
f_path(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    BYTE size_buff[UI32_B];
    uint32_t size = (_fc->fdir_len) + 1;

    if(!_fc->auth) return -1;

    memcpy(size_buff, &(size), UI32_B);
    send_data(_fc->fd, size_buff, UI32_B);
    send_data(_fc->fd, (BYTE *)_fc->f_directory, size);

    return 1;
}

static int     
f_mkfd(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char buffer[BUFF_SIZE];

    if(!_fc->auth) return -1;

    if(prepare_path(buffer, _fc, _ins->arg0) == NULL) return -1;
    int result = mkdir(buffer, 0700);
    if(result == -1) return -1;

    return 1;
}

static int     
f_rmfd(f_server* _fs, Instruction* _ins)
{
    f_client* _fc = &(_fs->fc);
    char buffer[BUFF_SIZE];

    if(!_fc->auth) return -1;

    if(prepare_path(buffer, _fc, _ins->arg0) == NULL) return -1; 
    int result = remove_directory(buffer);
    if(result == -1) return -1;

    return 1;
}


//#######################HELEPRS############################

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
    if((file_len + dir_len + 2 > BUFF_SIZE) || file_len == 0){
        return NULL;
    }

    //if backslash doesnt exist add it
    if(_fc->f_directory[dir_len - 1] != '/') {
        _fc->f_directory[dir_len++] = '/';
        _fc->f_directory[dir_len] = '\0'; 
        _fc->fdir_len++; dir_len++;
    }

    memcpy(_src, _fc->f_directory, dir_len);
    memcpy(_src + dir_len, _file, file_len + 1);

    return _src;
}

static int
send_resp (f_client* _fc, uint8_t _flag)
{
    BYTE response[5];

    memcpy(response, _fc->key, UI32_B);
    response[4] = _flag;
    send_data(_fc->fd, response, 5);

    return 1;
}
