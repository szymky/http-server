
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdbool.h>



#define GET(path) "GET " #path


#define PORT 8080
#define BUFFER_SIZE 1024

void handle_hello(int client_socket) {
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n";
    send(client_socket, response, strlen(response), 0);
}




void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];

    recv(client_socket, buffer, sizeof(buffer), 0);

    if (strncmp(buffer, "GET /hello", 10) == 0) {

        handle_hello(client_socket);


    } else if (strncmp(buffer, "GET /headers", 12) == 0) {
        // get headers
    } else {
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}


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
