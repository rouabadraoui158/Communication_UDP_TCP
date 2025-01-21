#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 8080
#define NMAX 100

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int n, i, buffer[NMAX];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");
    srand(time(0));
    while (1) {
        // Receive n from client
        recvfrom(sockfd, &n, sizeof(n), 0, (struct sockaddr*)&clientAddr, &addrLen);
        printf("Received n: %d\n", n);

        // Generate n random numbers
        
        for (i = 0; i < n; i++) {
            buffer[i] = rand() % 100;
        }

        // Send n random numbers back to client
        sendto(sockfd, buffer, n * sizeof(int), 0, (struct sockaddr*)&clientAddr, addrLen);
    }

    close(sockfd);
    return 0;
}
