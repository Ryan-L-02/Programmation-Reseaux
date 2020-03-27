#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define ERROR -1

int main(int argc, char *argv[])
{
    /*Création d'un serveur utilisant le protocole TCP*/
    int sock, sock_service;
    char message[1000];
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    /*Création de la socket et gestion de l'erreur*/
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    /*On configure la partie réseau, comme l'adresse IP et le port (qui est donné en argument)*/
    add_IP.sin_family = AF_INET;
    add_IP.sin_port = htons(atoi(argv[1]));
    add_IP.sin_addr.s_addr = INADDR_ANY;

    /*On lie la socket à la partie "réseau" (IP-Port), tout en gérant le code de retour*/
    if (bind(sock, (struct sockaddr *)&add_IP, sizeof(add_IP)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    /*On indique à la socket qu'elle doit être à l'écoute de potentielles connexions, tout en gérant le code de retour*/
    if (listen(sock, 5) == ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    /*Boucle infini, lorsqu'une connexion est terminé, on en attend une nouvelle sans terminer le programme*/
    while (1)
    {
        /*On accepte une connexion au serveur, tout en récupérant les infos du client (IP...), tout en gérant le code de retour*/
        sock_service = accept(sock, (struct sockaddr *)&add_IP, &taille);
        if (sock_service == ERROR)
        {
            perror("accept()");
            exit(errno);
        }

        /*On lit le contenu de la requête du client, tout en gérant le code de retour*/
        if (read(sock_service, &message, sizeof(message)) == ERROR)
        {
            perror("read()");
            exit(errno);
        }

        /*On affiche sur le terminal le contenu de la requête*/
        printf("%s", message);

        /*On ferme la connexion*/
        close(sock_service);
    }

    /*On ferme la socket avant de terminer le programme*/
    close(sock);
    return 0;
}