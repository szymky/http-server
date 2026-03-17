
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>

#include "http_server.h"

void handle_root(int client_socket) {
    const char* page = 
        "<!DOCTYPE html><html><head><title>index page</title></head><body><h1>Index page</h1><p>Server serving</p></body></html>";

    http_server_response http_response; 
    build_response(
            &http_response, 
            200, 
            "OK", 
            "text/html", 
            strlen(page), 
            page);
    const char* response = make_response(&http_response);
    send(client_socket, response, strlen(response), 0);
    

    free((void*)response);
}



int main(int argc, char **argv) {
   

    http_server server;
    if ( !setup_http_server(&server, 8080) ){
        perror("[-] Failed to setup http server");
        exit(EXIT_FAILURE);
    }

    http_server_register(&server, GET(/), handle_root);

    while ( true ) {

        if (accept_request(&server)) {
            perror("[-] Failed to accept request");
            break;
        }

        handle_request(&server);
    }

    printf("\n[+] Server shutting down..\n");

    close(server.server_socket);
    
    return 0;
}
