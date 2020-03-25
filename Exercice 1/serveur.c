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
    char message2[50] = "Transmission OK";
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    add_IP.sin_family = AF_INET;
    add_IP.sin_port = htons(atoi(argv[1]));
    add_IP.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&add_IP, sizeof(add_IP)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    PID_serveur = getpid();

    if (recvfrom(sock, message1, 50, 0, (struct sockaddr *)&add_IP, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }
    if (recvfrom(sock, &PID_client, sizeof(PID_client), 0, (struct sockaddr *)&add_IP, &taille) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }

    printf("Message reçu par le serveur : %s\n", message1);
    printf("PID reçu par le serveur : %d\n", PID_client);

    if (sendto(sock, message2, 50, 0, (struct sockaddr *)&add_IP, sizeof(add_IP)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }
    if (sendto(sock, &PID_serveur, sizeof(PID_serveur), 0, (struct sockaddr *)&add_IP, sizeof(add_IP)) < 0)
    {
        perror("sendto()");
        exit(errno);
    }

    return 0;
}