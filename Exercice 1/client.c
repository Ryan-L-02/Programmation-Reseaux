#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    int sock, res;
    char message1[50];
    char message2[50];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    socklen_t taille = sizeof(add_dest);

    memset(add_src.sin_zero, 0, 8);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        printf("Erreur creation socket");
        exit(1);
    }

    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    strcpy(message1, argv[3]);

    res = bind(sock, (struct sockaddr *)&add_src, sizeof(add_src));

    if (res == -1)
    {
        printf("Erreur bind");
        exit(2);
    }

    hp = gethostbyname(argv[1]);

    if (hp == NULL)
    {
        printf("Serveur introuvable");
        exit(3);
    }

    memcpy(&add_dest.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
    add_dest.sin_port = htons(atoi(argv[2]));
    add_dest.sin_family = AF_INET;

    sendto(sock, message1, 50, 0, (struct sockaddr *)&add_dest, sizeof(add_dest));
    
    recvfrom(sock, message2, 50, 0, (struct sockaddr *)&add_dest, &taille);

    printf("Message envoyé par le serveur : %s\n", message2);

    return 0;
}