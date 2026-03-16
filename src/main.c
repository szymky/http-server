
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdbool.h>


// Dynamic array
#define DYN_ARR_DEFAULT_SIZE 10 

typedef struct DynArr {

    void* data;    
    size_t item_count;
    size_t data_size;
    size_t size;

    void (*push)(struct DynArr*,void*); 
    void (*free)(struct DynArr*);
    
} DynArr;

static void DynArr_push(DynArr* arr, void* data) {

    if ( (arr->size - arr->item_count) < 0 ) {
       
        arr->size += (DYN_ARR_DEFAULT_SIZE * arr->data_size);
        arr->data = realloc(arr->data, arr->size);
    }

    memcpy(arr->data+(arr->item_count*arr->data_size), data, arr->data_size);
    arr->item_count++;

}

static void DynArr_free(DynArr* arr) {
    free(arr);
}


void DynArr_init(DynArr* dynArr, size_t data_size) {
    dynArr->push = DynArr_push;
    dynArr->free = DynArr_free;
    dynArr->data_size = data_size;
    dynArr->item_count = 0;

    dynArr->data = malloc(DYN_ARR_DEFAULT_SIZE*data_size);
    dynArr->size = DYN_ARR_DEFAULT_SIZE;
}
// end dynamic array


// http server 
#define GET(path) "GET " #path

#define HTTP_VERSION "HTTP/1.1"


typedef struct {
    
} http_server;

typedef struct {

    const char* http_version;
    uint32_t response_code;
    const char* response_text;

    const char* content_type;
    size_t content_length;

    const char* content;

} http_server_response;

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

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_hello(int client_socket) {
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n";
    send(client_socket, response, strlen(response), 0);
}

void handle_root(int client_socket) {
    const char* page = "<!DOCTYPE html><html><head><title>index page</title></head><body><h1>Index page</h1><p>Server serving</p></body></html>";

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
// end http server 


// TODO: abstract it away
int main(void) {


    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while ( true ) {

        client_socket = accept(server_socket, (struct  sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        handle_request(client_socket);

    }

    close(server_socket);
    
    return 0;
}
