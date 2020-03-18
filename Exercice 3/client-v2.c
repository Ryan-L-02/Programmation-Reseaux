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
    int sock, res;
    int res_write, res_read, res_connect;
    char buffer[5000];
    char request[56000];
    struct sockaddr_in add_src;
    struct sockaddr_in add_dest;
    struct hostent *hp;
    int taille = sizeof(add_dest);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        printf("Erreur creation socket");
        exit(1);
    }

    add_src.sin_family = AF_INET;
    add_src.sin_port = htons(10000);
    add_src.sin_addr.s_addr = INADDR_ANY;

    res = bind(sock, (struct sockaddr *)&add_src, sizeof(add_src));

    if (res == -1)
    {
        printf("Erreur bind");
        exit(2);
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

    res_connect = connect(sock, (struct sockaddr *)&add_dest, taille);
    if (res_connect == -1)
    {
        printf("Erreur connect");
        exit(4);
    }

    res_write = write(sock, request, sizeof(request));
    if (res_write == -1)
    {
        printf("Erreur write");
        exit(5);
    }

    res_read = read(sock, buffer, sizeof(buffer));
    if (res_read == -1)
    {
        printf("Erreur read");
    }

    write(sock, request, sizeof(request));

    while (read(sock, buffer, sizeof(buffer) > 0))
    {
        printf("%s\n", buffer);
    }
    close(sock);
    return 0;
}