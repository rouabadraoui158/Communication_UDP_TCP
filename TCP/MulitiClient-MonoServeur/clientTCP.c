#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5342
#define BUFFER_SIZE 1024

int client_socket;

// Fonction pour envoyer une requête et afficher le résultat
void send_request(GtkWidget *widget, gpointer data) {
    char *request = (char *)data;
    char buffer[BUFFER_SIZE] = {0};

    if (send(client_socket, request, strlen(request), 0) == -1) {
        perror("Erreur lors de l'envoi de la requête");
        return;
    }

    if (strcmp(request, "3") == 0) {
        GtkWidget *dialog = gtk_file_chooser_dialog_new(
            "Choisir un fichier", NULL,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "Annuler", GTK_RESPONSE_CANCEL,
            "Ouvrir", GTK_RESPONSE_ACCEPT,
            NULL
        );

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            send(client_socket, filename, strlen(filename), 0);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
    }

    if (recv(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
        perror("Erreur lors de la réception des données");
        return;
    }

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "%s", buffer
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Fonction pour afficher l'interface principale
void show_main_interface(GtkWidget *widget, gpointer data) {
    GtkWidget *auth_window = GTK_WIDGET(data);
    gtk_widget_destroy(auth_window); // Détruire la fenêtre d'authentification

    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Client Multi-Serveur");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 300, 200);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(main_window), grid);

    GtkWidget *button1 = gtk_button_new_with_label("Demander la date et l'heure");
    g_signal_connect(button1, "clicked", G_CALLBACK(send_request), "1");
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);

    GtkWidget *button2 = gtk_button_new_with_label("Liste des fichiers");
    g_signal_connect(button2, "clicked", G_CALLBACK(send_request), "2");
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);

    GtkWidget *button3 = gtk_button_new_with_label("Contenu d'un fichier");
    g_signal_connect(button3, "clicked", G_CALLBACK(send_request), "3");
    gtk_grid_attach(GTK_GRID(grid), button3, 0, 2, 1, 1);

    GtkWidget *button_exit = gtk_button_new_with_label("Quitter");
    g_signal_connect_swapped(button_exit, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_grid_attach(GTK_GRID(grid), button_exit, 0, 3, 1, 1);

    gtk_widget_show_all(main_window);
}

// Fonction pour gérer l'authentification
void on_login_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    GtkEntry *username_entry = GTK_ENTRY(entries[0]);
    GtkEntry *password_entry = GTK_ENTRY(entries[1]);
    GtkWidget *auth_window = GTK_WIDGET(entries[2]);

    const gchar *username = gtk_entry_get_text(username_entry);
    const gchar *password = gtk_entry_get_text(password_entry);

    if (strcmp(username, "admin") == 0 && strcmp(password, "password") == 0) {
        show_main_interface(auth_window, auth_window);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(
            NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Nom d'utilisateur ou mot de passe incorrect !"
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

// Fonction principale
int main(int argc, char *argv[]) {
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erreur lors de la création du socket");
        return EXIT_FAILURE;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Adresse invalide ou non supportée");
        return EXIT_FAILURE;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connexion au serveur échouée");
        return EXIT_FAILURE;
    }

    gtk_init(&argc, &argv);

    GtkWidget *auth_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(auth_window), "Authentification");
    gtk_window_set_default_size(GTK_WINDOW(auth_window), 300, 150);

    GtkWidget *auth_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(auth_window), auth_grid);

    GtkWidget *username_label = gtk_label_new("Nom d'utilisateur :");
    GtkWidget *username_entry = gtk_entry_new();
    GtkWidget *password_label = gtk_label_new("Mot de passe :");
    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    GtkWidget *login_button = gtk_button_new_with_label("Se connecter");
    GtkWidget *entries[] = {username_entry, password_entry, auth_window};
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), entries);

    gtk_grid_attach(GTK_GRID(auth_grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(auth_grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(auth_grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(auth_grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(auth_grid), login_button, 0, 2, 2, 1);

    gtk_widget_show_all(auth_window);
    gtk_main();

    close(client_socket);
    return 0;
}

