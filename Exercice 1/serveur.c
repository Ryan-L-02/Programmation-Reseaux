#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sock, res, PID_client, PID_serveur;
    char message1[50];
    char message2[50]="Transmission OK";
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        printf("Erreur creation socket");
        exit(1);
    }

    add_IP.sin_family = AF_INET;
    add_IP.sin_port = htons(atoi(argv[1]));
    add_IP.sin_addr.s_addr = INADDR_ANY;

    res = bind(sock, (struct sockaddr *)&add_IP, sizeof(add_IP));

    if (res == -1)
    {
        printf("Erreur bind");
        exit(2);
    }

    PID_serveur=getpid();

    recvfrom(sock, message1, 50, 0, (struct sockaddr *)&add_IP, &taille);
    recvfrom(sock, &PID_client, sizeof(PID_client),0, (struct sockaddr*) &add_IP, &taille);

    printf("Message reçu par le serveur : %s\n", message1);
    printf("PID reçu par le serveur : %d\n", PID_client);

    sendto(sock, message2, 50, 0, (struct sockaddr *)&add_IP, sizeof(add_IP));
    sendto(sock, &PID_serveur, sizeof(PID_serveur), 0, (struct sockaddr *) & add_IP, sizeof (add_IP));

    return 0;
}