#!/bin/bash

# Compilation du client
gcc -o clientTCP clientTCP.c $(pkg-config --cflags --libs gtk+-3.0)

# Vérification si la compilation du client a réussi
if [ $? -eq 0 ]; then
    echo "Compilation du client réussie."
else
    echo "Erreur lors de la compilation du client."
    exit 1
fi

# Compilation du serveur avec les threads
gcc -o serveurTCP serveurTCP.c -pthread

# Vérification si la compilation du serveur a réussi
if [ $? -eq 0 ]; then
    echo "Compilation du serveur réussie."
else
    echo "Erreur lors de la compilation du serveur."
    exit 1
fi

echo "Compilation terminée. Vous pouvez maintenant exécuter le serveur et le client."

