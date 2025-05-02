#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

// Function to handle client requests
void *handle_client(void *client_sock) {
    int sock = *(int *)client_sock;
    free(client_sock);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Read the request from the client
    bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(sock);
        return NULL;
    }
    
    buffer[bytes_received] = '\0';  // Null terminate the string
    
    // Print the request (optional)
    printf("Received request:\n%s\n", buffer);

    // HTTP Response
    const char *http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "<html><body><h1>Hello, World!</h1></body></html>";

    // Send the HTTP response
    send(sock, http_response, strlen(http_response), 0);

    // Close the connection
    close(sock);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create the server socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, BACKLOG) < 0) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept and handle client connections
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;  // Continue accepting new connections
        }

        // Allocate memory for the client socket
        int *client_sock_ptr = malloc(sizeof(int));
        if (client_sock_ptr == NULL) {
            perror("malloc");
            close(client_sock);
            continue;
        }
        *client_sock_ptr = client_sock;

        // Create a new thread to handle the client request
        if (pthread_create(&thread_id, NULL, handle_client, client_sock_ptr) != 0) {
            perror("pthread_create");
            free(client_sock_ptr);
            close(client_sock);
        }

        // Detach the thread so that resources are released automatically when the thread ends
        pthread_detach(thread_id);
    }

    // Close the server socket (this part will never be reached in this example)
    close(server_sock);
    return 0;
}
