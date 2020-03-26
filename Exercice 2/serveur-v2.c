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
    int sock, sock_service;
    char message[1000];
    struct sockaddr_in add_IP;
    socklen_t taille = sizeof(add_IP);

    sock = socket(AF_INET, SOCK_STREAM, 0);

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

    if (listen(sock, 5) == ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    sock_service = accept(sock, (struct sockaddr *)&add_IP, (socklen_t *)&taille);

    if (sock_service == ERROR)
    {
        perror("accept()");
        exit(errno);
    }

    if (read(sock_service, &message, sizeof(message)) == ERROR)
    {
        perror("read()");
        exit(errno);
    }
    printf("%s", message);

    close(sock);
    return 0;
}