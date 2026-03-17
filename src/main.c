
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

#include "http_server.h"

#include "utils/HashMap.h"




int main(void) {

    
    http_server server;
    if ( !setup_http_server(&server, 8080) ){
        perror("[-] Failed to setup http server");
        exit(EXIT_FAILURE);
    }
    while ( true ) {
        if ( accept_request(&server) ) {
            perror("[-] Failed to accept request");
            exit(EXIT_FAILURE);
        }
        handle_request(server.client_socket);
    }
    close(server.server_socket);
    
    return 0;
}
