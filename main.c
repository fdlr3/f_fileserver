#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/file_manager.h"
#include "src/server.h"
#include "src/logger.h"
#define PORT_N 1550

int main(){
    //setup logger
    start_logger();
    f_server server;
    start_server(PORT_N, &server);

    while(1){
        server.listen_server(&server);
        server_IO(&server.fc);
    }

    close_server(&server);
    //temp
    return 0;
}


