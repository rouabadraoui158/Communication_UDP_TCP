#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock = -1;

// Fonctions utilitaires
void connect_to_server(const char *server_ip) {
    struct sockaddr_in server_address;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur: Création du socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Erreur: Connexion au serveur");
        exit(EXIT_FAILURE);
    }

    printf("Connexion au serveur réussie.\n");
}

void send_request(const char *request, GtkTextBuffer *buffer) {
    char response[BUFFER_SIZE];
    send(sock, request, strlen(request), 0);

    bzero(response, BUFFER_SIZE);
    recv(sock, response, sizeof(response), 0);

    gtk_text_buffer_set_text(buffer, response, -1);
}

// Callbacks pour la seconde interface (services)
void on_service_clicked(GtkButton *button, gpointer data) {
    GtkTextBuffer *buffer = (GtkTextBuffer *)data;
    const char *command = gtk_button_get_label(button);

    if (strcmp(command, "Date/Heure") == 0) {
        send_request("date", buffer);
    } else if (strcmp(command, "Liste des fichiers") == 0) {
        send_request("list", buffer);
    }
}

void on_quit_clicked(GtkButton *button, gpointer data) {
    close(sock);
    gtk_main_quit();
}

// Interface principale après authentification réussie
void create_main_interface() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client TCP - Services");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(on_quit_clicked), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Boutons des services
    GtkWidget *services_button = gtk_button_new_with_label("Date/Heure");
    GtkWidget *files_button = gtk_button_new_with_label("Liste des fichiers");
    GtkWidget *quit_button = gtk_button_new_with_label("Quitter");

    gtk_box_pack_start(GTK_BOX(vbox), services_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), files_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), quit_button, FALSE, FALSE, 0);

    // Zone d'affichage des résultats
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Connexion des boutons aux callbacks
    g_signal_connect(services_button, "clicked", G_CALLBACK(on_service_clicked), buffer);
    g_signal_connect(files_button, "clicked", G_CALLBACK(on_service_clicked), buffer);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit_clicked), NULL);

    gtk_widget_show_all(window);
}

// Callback pour l'authentification
void on_authenticate_clicked(GtkButton *button, gpointer data) {
    GtkEntry **entries = (GtkEntry **)data;
    const char *username = gtk_entry_get_text(entries[0]);
    const char *password = gtk_entry_get_text(entries[1]);

    char auth_message[BUFFER_SIZE];
    snprintf(auth_message, sizeof(auth_message), "auth %s %s", username, password);
    send(sock, auth_message, strlen(auth_message), 0);

    char response[BUFFER_SIZE];
    bzero(response, BUFFER_SIZE);
    recv(sock, response, sizeof(response), 0);

    if (strcmp(response, "OK") == 0) {
        gtk_widget_destroy(GTK_WIDGET(entries[2])); // Fermer l'interface d'authentification
        create_main_interface();                   // Ouvrir l'interface principale
    } else {
        GtkTextBuffer *buffer = (GtkTextBuffer *)entries[3];
        gtk_text_buffer_set_text(buffer, "Échec de l'authentification. Réessayez.", -1);
    }
}

// Interface d'authentification
void create_auth_interface() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client TCP - Authentification");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(on_quit_clicked), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Nom d'utilisateur");
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 0);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 0);

    GtkWidget *auth_button = gtk_button_new_with_label("Authentification");
    gtk_box_pack_start(GTK_BOX(vbox), auth_button, FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    GtkEntry *entries[4] = {GTK_ENTRY(username_entry), GTK_ENTRY(password_entry), window, (GtkEntry *)buffer};
    g_signal_connect(auth_button, "clicked", G_CALLBACK(on_authenticate_clicked), entries);

    gtk_widget_show_all(window);
}

// Fonction principale
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Connexion au serveur
    connect_to_server("127.0.0.1");

    // Création de l'interface d'authentification
    create_auth_interface();

    gtk_main();
    return 0;
}

