#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/fdefines.h"
#include "src/file_manager.h"
#include "src/server.h"
#include "src/logger.h"
#define PORT_N 1553

int main(int argc, char **argv){
    if(argc != 4) {
        puts("Error in argument count. Exiting");
        exit(EXIT_SUCCESS);
    }
    //setup logger
    start_logger();
    f_server server;
    start_server(&server, argv[1], argv[2], argv[3]);

    while(1){
        listen_server(&server);
        server_IO(&server);
    }

    close_server(&server);
    return 0;
}


