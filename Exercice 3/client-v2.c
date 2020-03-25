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
    int sock;
    char buffer[5000];
    char request[56000];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    int taille = sizeof(add_dest);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == ERROR)
    {
        perror("socket()");
        exit(errno);
    }

    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&add_src, sizeof(add_src)) == ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    hp = (struct hostent *)gethostbyname(argv[1]);

    if (hp == NULL)
    {
        printf("Serveur introuvable");
        exit(3);
    }

    memcpy((void *)&add_dest.sin_addr.s_addr, (void *)hp->h_addr_list[0], (size_t)hp->h_length);
    add_dest.sin_port = htons(atoi(argv[2]));
    add_dest.sin_family = AF_INET;

    sprintf(request, "GET /%s HTTP/1.1\nHOST: %s:%d\nConnection: keep-alive\r\n\r\n", argv[3], argv[1], atoi(argv[2]));
    printf("Requête du client: \n%s\n", request);

    if (connect(sock, (struct sockaddr *)&add_dest, taille) == ERROR)
    {
        perror("connect()");
        exit(errno);
    }

    if (write(sock, request, sizeof(request)) == ERROR)
    {
        perror("write()");
        exit(errno);
    }

    if (read(sock, buffer, sizeof(buffer)) == ERROR)
    {
        perror("read()");
        exit(errno);
    }

    printf("%s", buffer);

    close(sock);
    return 0;
}