#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "./simpleSocketAPI.h"

#define SERVADDR "127.0.0.1" // Définition de l'adresse IP d'écoute
#define SERVPORT "0"         // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1          // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 1024    // Taille du tampon pour les échanges de données
#define MAXHOSTLEN 64        // Taille d'un nom de machine
#define MAXPORTLEN 64        // Taille d'un numéro de port

int main() {
    int ecode;                      // Code retour des fonctions
    char serverAddr[MAXHOSTLEN];    // Adresse du serveur
    char serverPort[MAXPORTLEN];    // Port du server
    int descSockRDV;                // Descripteur de socket de rendez-vous
    int descSockCOM;                // Descripteur de socket de communication
    struct addrinfo hints;          // Contrôle la fonction getaddrinfo
    struct addrinfo *res;           // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo; // Informations sur la connexion de RDV
    struct sockaddr_storage from;   // Informations sur le client connecté
    socklen_t len;                  // Variable utilisée pour stocker les
                                    // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];      // Tampon de communication entre le client et le serveur

    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1)
    {
        perror("Erreur création socket RDV\n");
        exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;     // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_family = AF_INET;       // seules les adresses IPv4 seront présentées par
                                     // la fonction getaddrinfo

    // Récupération des informations du serveur
    ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
    if (ecode)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
    }
    // Publication de la socket
    ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
    if (ecode == -1)
    {
        perror("Erreur liaison de la socket de RDV");
        exit(3);
    }
    // Nous n'avons plus besoin de cette liste chainée addrinfo
    freeaddrinfo(res);

    // Récupération du nom de la machine et du numéro de port pour affichage à l'écran
    len = sizeof(struct sockaddr_storage);
    ecode = getsockname(descSockRDV, (struct sockaddr *)&myinfo, &len);
    if (ecode == -1) {
        perror("SERVEUR: getsockname");
        exit(4);
    }

    ecode = getnameinfo((struct sockaddr *)&myinfo, sizeof(myinfo), serverAddr, MAXHOSTLEN,
                        serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
    if (ecode != 0) {
        fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
        exit(4);
    }
    printf("L'adresse d'ecoute est: %s\n", serverAddr);
    printf("Le port d'ecoute est: %s\n", serverPort);

    // Definition de la taille du tampon contenant les demandes de connexion
    ecode = listen(descSockRDV, LISTENLEN);
    if (ecode == -1) {
        perror("Erreur initialisation buffer d'écoute");
        exit(5);
    }

    len = sizeof(struct sockaddr_storage);
    // Attente connexion du client
    // Lorsque demande de connexion, creation d'une socket de communication avec le client
    descSockCOM = accept(descSockRDV, (struct sockaddr *)&from, &len);
    if (descSockCOM == -1) {
        perror("Erreur accept\n");
        exit(6);
    }

    /*****
    * Testez de mettre 220 devant BLABLABLA ...
    ***/
    strcpy(buffer, "220: Connexion au proxy OK !\n");
    write(descSockCOM, buffer, strlen(buffer));
    memset(buffer,0,MAXBUFFERLEN);

    /*******
     *
     * A vous de continuer !
     *
     * *****/

    char username[MAXBUFFERLEN]; // Nom d'utilisateur
    char password[MAXBUFFERLEN]; // Mot de passe 
    char bufferv2[MAXBUFFERLEN]; // Tampon de communication entre le client et le serveur
    char hostname[MAXHOSTLEN]; // Nom de la machine
    char port[] = "21";     // Port du serveur FTP  (21 par défaut)

    // Lecture du message du client
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN); // Lecture du message du client
    printf("Message reçu du client : \n%s\n", buffer); // Affichage du message reçu

    // Récupération du nom d'utilisateur et du nom de la machine
    sscanf(buffer, "%[^@]@%[^\n]", username, hostname); // Récupération du nom d'utilisateur et du nom de la machine
    hostname[strlen(hostname)-1] = '\0'; // Suppression du caractère de fin de ligne

    printf("Username : %s\n", username); // Affichage du nom d'utilisateur
    printf("Hostname : %s\n", hostname); // Affichage du nom de la machine

    // Création de la socket de communication CTRL
    int descSockFTPCRTL;  // Descripteur de socket

    //  Connexion au serveur FTP
    ecode = connect2Server(hostname, port, &descSockFTPCRTL); // Connexion au serveur FTP
    if (ecode == -1) { // Si erreur
        perror("Problème de connexion au serveur FTP\n"); exit(3); // On quitte
    }

    // Réception de la réponse du serveur
    ecode = read(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Réception de la réponse du serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception d'une réponse serveur\n"); // On quitte
        exit(4); // On quitte
    }
    buffer[ecode] = '\0'; //   
    printf("Message reçu du serveur : %s\n", buffer); // Affichage du message reçu

    // Envoi de la commande USER au serveur
    strcat(username, "\r\n"); // Ajout du caractère de fin de ligne
    ecode = write(descSockFTPCRTL, username, strlen(buffer)); //  Envoi de la commande USER au serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur de l'envoi  de la commande USER\n"); // On quitte
        exit(4); // On quitte
    }

    // Réception de la réponse du serveur
    ecode = read(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Réception de la réponse du serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception de la réponse serveur\n"); // On quitte
        exit(4); // On quitte
    }
    buffer[ecode] = '\0'; // Ajout un caractère de fin de chaîne (NULL) à la fin de la chaîne de caractères "buffer"
    ecode = write(descSockCOM, buffer, strlen(buffer)); // Envoi de la réponse au client
    if (ecode == -1) { // Si erreur
        perror("Erreur d'envoi de la réponse au client\n"); // On quitte
        exit(5); // On quitte
    }

    // Lecture du mot de passe
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN); // Lecture du mot de passe
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception de la réponse client\n"); // On quitte
    }
    buffer[ecode] = '\0'; // Ajout un caractère de fin de chaîne (NULL) à la fin de la chaîne de caractères "buffer"
    printf("Message reçu du client : %s\n", buffer); // Affichage du message reçu

    // Envoi de la commande PASS au serveur
    ecode = write(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Envoi de la commande PASS au serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur d'envoi d'une réponse au serveur\n"); // On quitte
        exit(5); // On quitte
    }

    // Réception de la réponse du serveur
    ecode = read(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Réception de la réponse du serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception d'une réponse serveur\n"); //  On quitte
        exit(4); // On quitte
    }
    buffer[ecode] = '\0'; // Ajout un caractère de fin de chaîne (NULL) à la fin de la chaîne de caractères "buffer"
    ecode = write(descSockCOM, buffer, strlen(buffer)); // Envoi de la réponse au client

    if (ecode == -1) { // Si erreur
        perror("Erreur d'envoi d'une réponse au client\n"); // On quitte
        exit(5); // On quitte
    }

    // Lecture de la commande du client, envoi d'un message serveur
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN); // Lecture de la commande du client
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception d'une réponse serveur\n"); // On quitte
        exit(4); // On quitte
    }
    buffer[ecode] = '\0'; // Ajout un caractère de fin de chaîne (NULL) à la fin de la chaîne de caractères "buffer"
    write(descSockFTPCRTL, buffer, strlen(buffer)); // Envoi de la commande au serveur
    printf("Message envoyé au serveur : %s\n", buffer); // Affichage du message envoyé

    // Réception de la réponse du serveur
    ecode = read(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Réception de la réponse du serveur
    if (ecode == -1) { // Si erreur
        perror("Erreur de réception d'une réponse serveur\n"); // On quitte
        exit(4); // On quitte
    }
    buffer[ecode] = '\0'; // Ajout un caractère de fin de chaîne (NULL) à la fin de la chaîne de caractères "buffer"
    ecode = write(descSockCOM, buffer, strlen(buffer)); // Envoi de la réponse au client
    if (ecode == -1) { //  Si erreur
        perror("Erreur d'envoi d'une réponse au client\n"); // On quitte
        exit(5); // On quitte
    }

    // Envoi de la commande PASV au serveur
    char commands[MAXBUFFERLEN]; // Tampon de communication entre le client et le serveur
    strcpy(commands, "PASV\r\n"); // Commande à envoyer au serveur
    ecode = write(descSockFTPCRTL, commands, strlen(commands)); // Envoi de la commande au serveur
    if (ecode == -1) { // Gestion des erreurs
        perror("Problème d'envoi de la commande PASV\r\n"); exit(3); // On quitte le programme
    }

    // Réception de la réponse du serveur
    ecode = read(descSockFTPCRTL, buffer, MAXBUFFERLEN); // Lecture de la réponse du serveur
    if (ecode == -1) { // Gestion des erreurs
        perror("Problème de lecture\n"); exit(3); // On quitte le programme
    }
    buffer[ecode] = '\0'; // On termine la chaîne de caractères
    ecode = write(descSockCOM, buffer, strlen(buffer));, // On envoie la réponse au client
    
    // On récupère l'adresse IP et le port du serveur FTP
    char ipFTP[MAXHOSTLEN]; // Adresse IP du serveur FTP
    char portFTP[MAXPORTLEN]; // Port du serveur FTP
    int w,x,y,z,port1,port2; // Variables temporaires pour l'extraction de l'adresse IP et du port

    // Extraction de l'adresse IP et du port
    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &w, &x, &y, &z, &port1, &port2); // Extraction des données
    sprintf(ipFTP, "%d.%d.%d.%d", w, x, y, z); // Construction de l'adresse IP
    int valeur = port1 * 256 + port2; // Calcul du port
    sprintf(portFTP, "%d", valeur); //  Construction du port

    printf("IP : %s\n", ipFTP); // Affichage de l'adresse IP
    printf("Port : %s\n\n", portFTP); // Affichage du port

/*

    // Création de la socket de communication avec le serveur FTP
    
    // Connect to data channel
    int ecode
    ecode = connect2Server(ipFTP, portFTP, &descSockFTPDATA);
    if (ecode == -1) {
        perror("Error connecting to data channel\n");
    exit(3);
}   


    // Envoi de la commande LIST au serveur FTP
    
    // Send LIST command to FTP server
    strcpy(buffer, "Connected to server!\n");
    ecode = write(descSockCOM, buffer, MAXBUFFERLEN);
    if (ecode == -1) {
        perror("Error sending connection confirmation\n");
        exit(3);
}

*/
    
    // Fermeture de la connexion
    close(descSockCOM); // Fermeture de la socket de communication client - proxy
    close(descSockFTPCRTL); // Fermeture de la socket de communication proxy - serveur FTP
    close(descSockRDV); // Fermeture de la socket de rendez-vous
}