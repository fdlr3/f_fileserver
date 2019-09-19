#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/fdefines.h"
#include "src/file_manager.h"
#include "src/server.h"
#include "src/logger.h"
#define PORT_N 1553

int main(int argc, char **argv){
    if(argc != 2) {
        puts("Error in argument count. Exiting");
        exit(EXIT_SUCCESS);
    }
    f_server server;
    start_server(&server, argv[1]);

    while(1){
        listen_server(&server);
        server_IO(&server);
    }

    close_server(&server);
    return 0;
}


