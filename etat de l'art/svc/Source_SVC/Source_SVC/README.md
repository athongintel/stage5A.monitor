# Pré-requis

Dépendances nécéssaire : OpenSSL, build-essential
Pour debian/ubuntu :  sudo apt-get install libssl-dev build-essential

# Compilation
À la racine du dossier executer la commande make, les binaires sont créés dans le fichier bin/

# Usage
Un programme de demonstration d'utilisation de la librairie est donné, il permet l'envoi d'un fichier du serveur vers le client. 
Les certificats et les clés de démonstration utilisé pour l'authentification du client et du serveur doivent être placé dans un dossier .PKI à la racine du home directory (~/.PKI). Pour cela utiliser les commandet mkdir ~/.PKI/ et cp PKI/* ~/.PKI/
## Serveur
Utilisation : ./server <port> <path_to_file>
Les arguments sont obligatoires :
port : Le port d'écoute du serveur
path_to_file : le chemin vers le fichier qui sera envoyé au client qui se connecte.

## Client
Utilisation: ./client <ip_address> <port> <path_to_file>

Les arguments sont obligatoires :
ip_address : Adresse ip du serveur (PAS de FQDN)
port : Le port du serveur
path_to_file : Le chemin ou sera enregistrer le fichier reçu.

#Dossier Tests 
Ce dossier contient les programmes client et serveur utilisant le protocole TLS. Ces programmes ont été utilisés pour faire les tests de comparaison avec le protocole SVC.