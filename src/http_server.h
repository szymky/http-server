#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H 

#include "utils/HashMap.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define GET(path)       "GET "  #path
#define PUT(path)       "PUT " #path
#define HEAD(path)      "HEAD " #path
#define POST(path)      "POST " #path
#define DELETE(path)    "DELETE " #path
#define CONNECT(path)   "CONNECT " #path
#define TRACE(path)     "TRACE " #path
#define PATCH(path)     "PATCH " #path

#define HTTP_VERSION "HTTP/1.1"
#define BUFFER_SIZE 1024

typedef struct {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    HashMap *routes;
} http_server;

typedef struct {
    const char* http_version;
    uint32_t response_code;
    const char* response_text;
    const char* content_type;
    size_t content_length;
    const char* content;
} http_server_response;


bool setup_http_server(http_server* server, int port);
void build_response(
        http_server_response* response,
        uint32_t response_code,
        const char* response_text,
        const char* content_type,
        size_t content_length,
        const char* content
        );

char* make_response(const http_server_response* response);
void handle_hello(int client_socket);
void handle_root(int client_socket);
void handle_request(http_server* server);
bool accept_request (http_server* server);

void http_server_register(http_server* server, const char* path, router_handler handler);

#endif
