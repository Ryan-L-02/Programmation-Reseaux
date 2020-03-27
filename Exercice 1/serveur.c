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
    /*Création d'un serveur utilisant le protocole UDP*/
    int sock, PID_client, PID_serveur;
    char message[100];
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    /*Création de la socket et gestion de l'erreur*/
    sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    /*On récupère notre PID*/
    PID_serveur = getpid();

    /*On reçoit le message envoyé par le client, tout en gérant le code de retour*/
    if (recvfrom(sock, message, 50, 0, (struct sockaddr *)&add_IP, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    /*On reçoit le PID du client, tout en gérant le code de retour*/
    if (recvfrom(sock, &PID_client, sizeof(PID_client), 0, (struct sockaddr *)&add_IP, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    /*On affiche sur le terminal les infos envoyés par le client*/
    printf("Message reçu par le serveur : %s\n", message);
    printf("PID reçu par le serveur : %d\n", PID_client);

    /*On envoie la chaîne de caractère qu'on a reçu, tout en gérant le code de retour*/
    if (sendto(sock, message, 50, 0, (struct sockaddr *)&add_IP, sizeof(add_IP)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    /*On envoie notre PID, tout en gérant le code de retour*/
    if (sendto(sock, &PID_serveur, sizeof(PID_serveur), 0, (struct sockaddr *)&add_IP, sizeof(add_IP)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    /*On ferme la socket avant de terminer le programme*/
    close(sock);
    return 0;
}