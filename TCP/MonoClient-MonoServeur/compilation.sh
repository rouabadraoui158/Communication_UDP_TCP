#!/bin/bash

# Compilation des fichiers source
gcc -o serveurTCP serveurTCP.c -pthread $(pkg-config --cflags --libs gtk+-3.0) -lm -lpthread
gcc -o clientTCP clientTCP.c $(pkg-config --cflags --libs gtk+-3.0) -lm -lpthread

