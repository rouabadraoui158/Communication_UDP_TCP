#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>       // Pour les fonctions time et localtime
#include <dirent.h>     // Pour les fonctions opendir, readdir, closedir

#define PORT 5342
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0 || strcmp(buffer, "exit") == 0) break;

        printf("Reçu du client : %s\n", buffer);

        // Traiter les requêtes du client
        if (strcmp(buffer, "1") == 0) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            snprintf(buffer, BUFFER_SIZE, "Date: %02d/%02d/%04d Time: %02d:%02d:%02d",
                     t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
        } else if (strcmp(buffer, "2") == 0) {
            DIR *d = opendir(".");
            if (d) {
                struct dirent *dir;
                buffer[0] = '\0';
                while ((dir = readdir(d)) != NULL) {
                    strcat(buffer, dir->d_name);
                    strcat(buffer, "\n");
                }
                closedir(d);
            } else {
                snprintf(buffer, BUFFER_SIZE, "Impossible d'ouvrir le répertoire.");
            }
        } else if (strcmp(buffer, "3") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            recv(client_socket, buffer, BUFFER_SIZE, 0); // Recevoir le nom du fichier
            FILE *file = fopen(buffer, "r");
            if (file) {
                memset(buffer, 0, BUFFER_SIZE);
                fread(buffer, 1, BUFFER_SIZE - 1, file);
                fclose(file);
            } else {
                snprintf(buffer, BUFFER_SIZE, "Fichier introuvable.");
            }
        } else {
            snprintf(buffer, BUFFER_SIZE, "Option invalide.");
        }

        send(client_socket, buffer, BUFFER_SIZE, 0);
    }

    close(client_socket);
    printf("Client déconnecté.\n");
    exit(0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Erreur lors du bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("Erreur lors du listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur prêt sur le port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket == -1) {
            perror("Erreur lors de l'acceptation d'une connexion");
            continue;
        }

        if (fork() == 0) { // Crée un processus fils pour chaque client
            close(server_fd);
            handle_client(client_socket);
        } else {
            close(client_socket);
        }
    }

    close(server_fd);
    return 0;
}

