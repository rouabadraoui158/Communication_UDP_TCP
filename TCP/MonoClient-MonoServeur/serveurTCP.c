#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 1234

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg);
    time_t t;
    struct tm *tm_info;
    char buffer[256];

    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 256, "%d/%m/%Y %H:%M:%S", tm_info);
    send(client_socket, buffer, strlen(buffer), 0);

    close(client_socket);  // Close client socket after communication
    return NULL;
}

int main() {
    int server_socket, *client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    pthread_t thread_id;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(1);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_socket = malloc(sizeof(int));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (*client_socket < 0) {
            perror("Client connection failed");
            free(client_socket);  // Free memory if connection fails
            continue;
        }

        printf("Client connected\n");
        pthread_create(&thread_id, NULL, handle_client, (void*)client_socket);
        pthread_detach(thread_id);  // Detach the thread after creation
    }

    close(server_socket);  // Close the server socket after loop
    return 0;
}

