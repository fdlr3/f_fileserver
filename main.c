#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/file_manager.h"
#include "src/server.h"
#include "src/logger.h"
#define PORT_N 1555

int main(){
    //setup logger
    start_logger();
    f_server server;
    start_server(PORT_N, &server);

    while(1){
        server.listen_server(&server);
        server_IO(&server);
    }

    close_server(&server);
    return 0;
}


