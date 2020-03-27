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
    /*Création d'un client utilisant le protocole UDP*/
    int sock, PID_client, PID_serveur;
    char message1[100];
    char message2[100];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    socklen_t taille = sizeof(add_dest);

    memset(add_src.sin_zero, 0, 8);

    /*Création de la socket et gestion de l'erreur*/
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    /*On configure la partie réseau, comme l'adresse IP et le port*/
    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    /*On récupère la chaîne de caractère placé en argument*/
    strcpy(message1, argv[3]);

    /*On lie la socket à la partie "réseau" (IP-Port), tout en gérant le code de retour*/
    if (bind(sock, (struct sockaddr *)&add_src, sizeof(add_src)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    /*On associe la chaîne de caractère placé en argument à l'hôte que l'on souhaite atteindre*/
    hp = gethostbyname(argv[1]);
    if (hp == NULL)
    {
        printf("Serveur introuvable");
        exit(1);
    }

    /*On associe l'hôte avec une adresse et un port*/
    memcpy(&add_dest.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
    add_dest.sin_port = htons(atoi(argv[2]));
    add_dest.sin_family = AF_INET;

    /*On récupère notre PID*/
    PID_client = getpid();

    /*On envoie la chaîne de caractère qui a été placé en argument, tout en gérant le code de retour*/
    if (sendto(sock, message1, 50, 0, (struct sockaddr *)&add_dest, sizeof(add_dest)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    /*On envoie notre PID, tout en gérant le code de retour*/
    if (sendto(sock, &PID_client, sizeof(PID_client), 0, (struct sockaddr *)&add_dest, sizeof(add_dest)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    /*On reçoit le message envoyé par le serveur, tout en gérant le code de retour*/
    if (recvfrom(sock, message2, 50, 0, (struct sockaddr *)&add_dest, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    /*On reçoit le PID du serveur, tout en gérant le code de retour*/
    if (recvfrom(sock, &PID_serveur, sizeof(PID_serveur), 0, (struct sockaddr *)&add_dest, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    /*On affiche sur le terminal les infos envoyés par le serveur*/
    printf("Message envoyé par le serveur : %s\n", message2);
    printf("PID envoyé par le serveur : %d\n", PID_serveur);

    /*On ferme la socket avant de terminer le programme*/
    close(sock);
    return 0;
}