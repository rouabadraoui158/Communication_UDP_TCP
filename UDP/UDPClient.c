#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define NMAX 100

int main() {
    int sockfd;
    struct sockaddr_in serverAddr;
    int n, i;
    socklen_t addrLen = sizeof(serverAddr);
    int buffer[NMAX];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // Generate a random number n
    srand(time(0));
    n = rand() % NMAX + 1;
    printf("Generated random number: %d\n", n);

    // Send n to the server
    sendto(sockfd, &n, sizeof(n), 0, (struct sockaddr*)&serverAddr, addrLen);

    // Receive n random numbers from the server
    recvfrom(sockfd, buffer, sizeof(int) *n, 0, (struct sockaddr*)&serverAddr, &addrLen);

    printf("Received %d random numbers from server:\n", n);
    for (i = 0; i < n; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    close(sockfd);
    return 0;
}

