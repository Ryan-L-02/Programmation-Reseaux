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
    int sock, res, sock_service, n = 1;
    char message[50];
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        printf("Erreur creation socket");
        close(sock);
        exit(1);
    }

    add_IP.sin_family = AF_INET;
    add_IP.sin_port = htons(atoi(argv[1]));
    add_IP.sin_addr.s_addr = INADDR_ANY;

    res = bind(sock, (struct sockaddr *)&add_IP, sizeof(add_IP));

    if (res == -1)
    {
        printf("Error bind");
        exit(2);
    }

    listen(sock, 5);

    sock_service = accept(sock, (struct sockaddr *)&add_IP, (socklen_t *)&taille);

    if (sock_service < 0)
    {
        printf("Error accept");
        close(sock_service);
        exit(3);
    }
    else
    {
        while (n > 0)
        {
            n = read(sock_service, &message, sizeof(message));
            printf("%s", message);
        }
    }

    printf("\n");
    return 0;
}