
#ifndef MHTTP_SERVER_H
#define MHTTP_SERVER_H

#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

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




#define INITIAL_SIZE 16
#define LOAD_FACTOR_THRESHOLD 0.75

typedef void (*router_handler)(int);

typedef struct Node {
    char *key;
    router_handler value;
    struct Node *next;
} Node;

typedef struct {
    Node **buckets;
    int size;
    int count;
} HashMap;

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

HashMap      *hm_create(void);
void          hm_put(HashMap *map, const char *key, router_handler value);
router_handler hm_get(HashMap *map, const char *key);
void          hm_delete(HashMap *map, const char *key);
void          hm_free(HashMap *map);

bool setup_http_server(http_server* server, int port);
void build_response(
        http_server_response* response,
        uint32_t response_code,
        const char* response_text,
        const char* content_type,
        size_t content_length,
        const char* content
        );

char *make_response(const http_server_response* response);
void handle_hello(int client_socket);
void handle_root(int client_socket);
void handle_request(http_server* server);
bool accept_request (http_server* server);
void send_and_free_response(int fd, char *response);

void http_server_register(http_server* server, const char* path, router_handler handler);

#ifdef MHTTP_SERVER_IMPLEMENTATION

static unsigned int hm__hash(const char *key, int size) {
    unsigned int h = 0;
    while (*key) {
        h = (h * 31) + *key++;
    }
    return h % size;
}

static void hm__resize(HashMap *map) {
    int old_size = map->size;
    Node **old_buckets = map->buckets;
    map->size *= 2;
    map->buckets = (Node**)calloc(map->size, sizeof(Node *));
    map->count = 0;
    for (int i = 0; i < old_size; i++) {
        Node *current = old_buckets[i];
        while (current) {
            Node *next = current->next;
            unsigned int index = hm__hash(current->key, map->size);
            current->next = map->buckets[index];
            map->buckets[index] = current;
            current = next;
            map->count++;
        }
    }
    free(old_buckets);
}

HashMap *hm_create(void) {
    HashMap *map = (HashMap*)malloc(sizeof(HashMap));
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = (Node**)calloc(map->size, sizeof(Node *));
    return map;
}

void hm_put(HashMap *map, const char *key, router_handler value) {
    if ((float)(map->count + 1) / map->size > LOAD_FACTOR_THRESHOLD)
        hm__resize(map);
    unsigned int index = hm__hash(key, map->size);
    Node *current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }
    Node *node = (Node*)malloc(sizeof(Node));
    node->key = strdup(key);
    node->value = value;
    node->next = map->buckets[index];
    map->buckets[index] = node;
    map->count++;
}

router_handler hm_get(HashMap *map, const char *key) {
    unsigned int index = hm__hash(key, map->size);
    Node *current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0)
            return current->value;
        current = current->next;
    }
    return NULL;
}

void hm_delete(HashMap *map, const char *key) {
    unsigned int index = hm__hash(key, map->size);
    Node *current = map->buckets[index];
    Node *prev = NULL;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) prev->next = current->next;
            else map->buckets[index] = current->next;
            free(current->key);
            free(current);
            map->count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void hm_free(HashMap *map) {
    for (int i = 0; i < map->size; i++) {
        Node *current = map->buckets[i];
        while (current) {
            Node *tmp = current;
            current = current->next;
            free(tmp->key);
            free(tmp);
        }
    }
    free(map->buckets);
    free(map);
}

bool 
setup_http_server(http_server *server, int port) {
    
    server->routes = hm_create();

    server->addr_len = sizeof(server->client_addr);
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket == -1) {
        return false;    
    }

    int opt = 1;
    setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    if (bind(server->server_socket, (struct sockaddr *)&server->server_addr, sizeof(server->server_addr)) < 0) {
        close(server->server_socket);
        return false;     
    }
    if (listen(server->server_socket, 10) < 0) {
        close(server->server_socket);
        return false;
    }
    return true;
}

void 
build_response(
        http_server_response* response,
        uint32_t response_code,
        const char *response_text,
        const char *content_type,
        size_t content_length,
        const char *content
        ) {
    response->http_version = HTTP_VERSION;
    response->response_code = response_code;
    response->response_text = response_text;
    response->content_type = content_type;
    response->content_length = content_length;
    response->content = content;
}

char
*make_response(const http_server_response *response) {
    size_t needed = snprintf(NULL, 0, "%s %d %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n%s", 
            response->http_version, 
            response->response_code, 
            response->response_text, 
            response->content_type, 
            response->content_length,
            response->content);
    char* buffer = (char*)malloc(needed + 1);
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

void 
handle_request(http_server *server) {
    char buffer[BUFFER_SIZE];
    recv(server->client_socket, buffer, sizeof(buffer), 0);

    // TODO: parse the response to a struct 

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

    router_handler func = hm_get(server->routes, buffer);
    if (func == NULL) {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        send(server->client_socket, response, strlen(response), 0);
    } else {
        func(server->client_socket);
    }

    close(server->client_socket);
}

bool 
accept_request (http_server *server) {
    server->client_socket = accept(server->server_socket, (struct  sockaddr *)&server->client_addr, &server->addr_len);
    return server->client_socket >= 0;
}

void 
http_server_register(http_server *server, const char *path, router_handler handler) {
    hm_put(server->routes, path, handler);
}

void 
send_and_free_response(int fd, char *response) {
    send(fd, response, strlen(response), 0);
    free(response);
}


#endif // MHTTP_SERVER_IMPLEMENTATION
#endif // MHTTP_SERVER_H
