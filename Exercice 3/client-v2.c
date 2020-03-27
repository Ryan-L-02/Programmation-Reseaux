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
    /*Création d'un client utilisant le protocole TCP*/
    int sock;
    char buffer[5000];
    char request[56000];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    int taille = sizeof(add_dest);

    /*Création de la socket et gestion de l'erreur*/
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    /*On configure la partie réseau, comme l'adresse IP et le port*/
    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    /*On lie la socket à la partie "réseau" (IP-Port), tout en gérant le code de retour*/
    if (bind(sock, (struct sockaddr *)&add_src, sizeof(add_src)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    /*On associe la chaîne de caractère placé en argument à l'hôte que l'on souhaite atteindre*/
    hp = (struct hostent *)gethostbyname(argv[1]);
    if (hp == NULL)
    {
        printf("Serveur introuvable");
        exit(3);
    }

    /*On associe l'hôte avec une adresse et un port (qui a été placé en argument lors de l'exécution)*/
    memcpy((void *)&add_dest.sin_addr.s_addr, (void *)hp->h_addr_list[0], (size_t)hp->h_length);
    add_dest.sin_port = htons(atoi(argv[2]));
    add_dest.sin_family = AF_INET;

    /*Création de la requête HTTP qui va être envoyé au serveur*/
    if (argv[3] == NULL)
    {
        sprintf(request, "GET / HTTP/1.1\r\nHOST: %s:%d\r\nConnection: keep-alive\r\n\r\n", argv[1], atoi(argv[2]));
    }
    else
    {
        sprintf(request, "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nConnection: keep-alive\r\n\r\n", argv[3], argv[1], atoi(argv[2]));
    }

    /*On affiche sur le terminal la requête que l'on a envoyé au serveur*/
    printf("Requête du client: \n%s\n", request);

    /*On établi la connexion avec le serveur, tout en gérant le code de retour*/
    if (connect(sock, (struct sockaddr *)&add_dest, taille) == ERROR)
    {
        perror("connect()");
        exit(errno);
    }

    /*On envoie notre requête au serveur, tout en gérant le code de retour*/
    if (write(sock, request, sizeof(request)) == ERROR)
    {
        perror("write()");
        exit(errno);
    }

    /*On lit la réponse du serveur, tout en gérant le code de retour*/
    if (read(sock, buffer, sizeof(buffer)) == ERROR)
    {
        perror("read()");
        exit(errno);
    }

    /*On affiche sur le terminal la réponse du serveur*/
    printf("%s", buffer);

    /*On ferme la socket avant de terminer le programme*/
    close(sock);
    return 0;
}