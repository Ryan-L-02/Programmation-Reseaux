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
    int sock, PID_client, PID_serveur;
    char message1[50];
    char message2[50];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    socklen_t taille = sizeof(add_dest);

    memset(add_src.sin_zero, 0, 8);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    strcpy(message1, argv[3]);

    if (bind(sock, (struct sockaddr *)&add_src, sizeof(add_src)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    hp = gethostbyname(argv[1]);

    if (hp == NULL)
    {
        printf("Serveur introuvable");
        exit(1);
    }

    memcpy(&add_dest.sin_addr.s_addr, hp->h_addr_list[0], hp->h_length);
    add_dest.sin_port = htons(atoi(argv[2]));
    add_dest.sin_family = AF_INET;

    PID_client = getpid();

    if (sendto(sock, message1, 50, 0, (struct sockaddr *)&add_dest, sizeof(add_dest)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }
    if (sendto(sock, &PID_client, sizeof(PID_client), 0, (struct sockaddr *)&add_dest, sizeof(add_dest)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    if (recvfrom(sock, message2, 50, 0, (struct sockaddr *)&add_dest, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }
    if (recvfrom(sock, &PID_serveur, sizeof(PID_serveur), 0, (struct sockaddr *)&add_dest, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    printf("Message envoyé par le serveur : %s\n", message2);
    printf("PID envoyé par le serveur : %d\n", PID_serveur);

    return 0;
}