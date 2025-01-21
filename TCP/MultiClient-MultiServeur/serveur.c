#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int client_socket;
    struct sockaddr_in client_address;
} ClientInfo;

void send_date_time(int client_socket) {
    char buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(buffer, sizeof(buffer), "Date et heure: %02d/%02d/%04d %02d:%02d:%02d",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
    send(client_socket, buffer, strlen(buffer), 0);
}

void send_directory_files(int client_socket) {
    char buffer[BUFFER_SIZE];
    DIR *d = opendir(".");
    if (!d) {
        strcpy(buffer, "Erreur: Impossible d'ouvrir le répertoire.\n");
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name);
        send(client_socket, buffer, strlen(buffer), 0);
    }
    closedir(d);
}

void send_file_content(int client_socket, const char *filename) {
    char buffer[BUFFER_SIZE];
    FILE *file = fopen(filename, "r");
    if (!file) {
        snprintf(buffer, sizeof(buffer), "Erreur: Impossible d'ouvrir le fichier %s.\n", filename);
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }
    while (fgets(buffer, sizeof(buffer), file)) {
        send(client_socket, buffer, strlen(buffer), 0);
    }
    fclose(file);
}

void *handle_client(void *arg) {
    ClientInfo client = *(ClientInfo *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    int running = 1;

    while (running) {
        bzero(buffer, BUFFER_SIZE);
        recv(client.client_socket, buffer, sizeof(buffer), 0);

        if (strcmp(buffer, "date") == 0) {
            send_date_time(client.client_socket);
        } else if (strcmp(buffer, "files") == 0) {
            send_directory_files(client.client_socket);
        } else if (strncmp(buffer, "read ", 5) == 0) {
            sscanf(buffer, "read %s", command);
            send_file_content(client.client_socket, command);
        } else if (strcmp(buffer, "quit") == 0) {
            running = 0;
            send(client.client_socket, "Déconnexion.\n", 13, 0);
        } else {
            send(client.client_socket, "Commande inconnue.\n", 20, 0);
        }
    }
    close(client.client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address, client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    printf("Serveur démarré sur le port %d\n", PORT);

    while (1) {
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);

        ClientInfo *client_info = malloc(sizeof(ClientInfo));
        client_info->client_socket = client_socket;
        client_info->client_address = client_address;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, client_info);
        pthread_detach(client_thread);
    }

    close(server_socket);
    return 0;
}

