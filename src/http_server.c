
#include "http_server.h"

bool setup_http_server(http_server* server, int port) {
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

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    recv(client_socket, buffer, sizeof(buffer), 0);
    if (strncmp(buffer, "GET /hello", 10) == 0) {
        handle_hello(client_socket);
    } else if (strncmp(buffer, "GET /", 5) == 0) {
        handle_root(client_socket);
    } else {
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        send(client_socket, response, strlen(response), 0);
    }
    close(client_socket);
}

bool accept_request (http_server* server) {
    server->client_socket = accept(server->server_socket, (struct  sockaddr *)&server->client_addr, &server->addr_len);
    return server->client_socket < 0;
}
