#!/bin/bash

# Chemins des fichiers sources
SERVER_SOURCE="serveur.c"
CLIENT_SOURCE="client.c"
SERVER_EXEC="serveurTCP"
CLIENT_EXEC="clientTCP"

# Commande de compilation
echo "Compilation des fichiers sources..."
gcc -o $SERVER_EXEC $SERVER_SOURCE -lpthread
if [ $? -ne 0 ]; then
  echo "Erreur: La compilation du serveur a échoué."
  exit 1
fi

gcc -o $CLIENT_EXEC $CLIENT_SOURCE `pkg-config --cflags --libs gtk+-3.0`
if [ $? -ne 0 ]; then
  echo "Erreur: La compilation du client a échoué."
  exit 1
fi

echo "Compilation réussie."

# Options d'exécution
echo
echo "Options disponibles :"
echo "1) Lancer le serveur"
echo "2) Lancer le client"
echo "3) Quitter"
read -p "Entrez votre choix : " choice

case $choice in
  1)
    echo "Lancement du serveur..."
    ./$SERVER_EXEC
    ;;
  2)
    echo "Lancement du client..."
    ./$CLIENT_EXEC
    ;;
  3)
    echo "Quitter."
    exit 0
    ;;
  *)
    echo "Choix invalide. Veuillez réessayer."
    ;;
esac

