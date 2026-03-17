
#include "http_server.h"
#include "utils/HashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool setup_http_server(http_server* server, int port) {
    
    server->routes = hashmap_create();

    server->addr_len = sizeof(server->client_addr);
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket == -1) {
        return false;    
    }
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    if (bind(server->server_socket, (struct sockaddr *)&server->server_addr, sizeof(server->server_addr)) < 0) {
        return false;     
    }
    if (listen(server->server_socket, 10) < 0) {
        return false;
    }
    return true;
}

void build_response(
        http_server_response* response,
        uint32_t response_code,
        const char* response_text,
        const char* content_type,
        size_t content_length,
        const char* content
        ) {
    response->http_version = HTTP_VERSION;
    response->response_code = response_code;
    response->response_text = response_text;
    response->content_type = content_type;
    response->content_length = content_length;
    response->content = content;
}

char* make_response(const http_server_response* response) {
    size_t needed = snprintf(NULL, 0, "%s %d %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n%s", 
            response->http_version, 
            response->response_code, 
            response->response_text, 
            response->content_type, 
            response->content_length,
            response->content);
    char* buffer = malloc(needed + 1);
    if (!buffer) return NULL;
    snprintf(buffer, needed + 1, "%s %d %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n%s", 
            response->http_version, 
            response->response_code,
            response->response_text, 
            response->content_type, 
            response->content_length,
            response->content);
    return buffer;
}

void handle_hello(int client_socket) {
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n";
    send(client_socket, response, strlen(response), 0);
}

void handle_request(http_server* server) {
    char buffer[BUFFER_SIZE];
    recv(server->client_socket, buffer, sizeof(buffer), 0);

    char *space1 = strchr(buffer, ' ');
    if (!space1) {
        fprintf(stderr, "Invalid request line\n");
        exit(EXIT_FAILURE); // TODO: maybe not kill the server if someone sends an invalid http request xd
    }

    char *space2 = strchr(space1 + 1, ' ');
    if (space2) {
        *space2 = '\0';
    }

    printf("%s\n", buffer);



    router_handler func = hashmap_get(server->routes, buffer);
    if (func == NULL) {
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        send(server->client_socket, response, strlen(response), 0);
    } else {
        func(server->client_socket);
    }

    close(server->client_socket);
}

bool accept_request (http_server* server) {
    server->client_socket = accept(server->server_socket, (struct  sockaddr *)&server->client_addr, &server->addr_len);
    return server->client_socket < 0;
}

void http_server_register(http_server *server, const char *path, router_handler handler) {
    hashmap_put(server->routes, path, handler);
}
