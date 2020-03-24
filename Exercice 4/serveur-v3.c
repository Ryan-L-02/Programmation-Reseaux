#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>

int main(int argc, char *argv[])
{
    int sock_web, res_web;
    int sock_log, res_log;
    struct sockaddr_in add_web;
    struct sockaddr_in add_log;
    int sock_web_service;
    int sock_log_service;
    socklen_t taille_web = sizeof(add_web);
    socklen_t taille_log = sizeof(add_log);

    sock_web = socket(AF_INET, SOCK_STREAM, 0);
    sock_log = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_web == -1)
    {
        printf("Erreur creation socket");
        close(sock_web);
        exit(1);
    }

    if (sock_log == -1)
    {
        printf("Erreur creation socket");
        close(sock_log);
        exit(1);
    }

    add_web.sin_family = AF_INET;
    add_web.sin_port = htons(atoi(argv[1]));
    add_web.sin_addr.s_addr = INADDR_ANY;

    add_log.sin_family = AF_INET;
    add_log.sin_port = htons(atoi(argv[2]));
    add_log.sin_addr.s_addr = INADDR_ANY;

    res_web = bind(sock_web, (struct sockaddr *)&add_web, sizeof(add_web));
    res_log = bind(sock_log, (struct sockaddr *)&add_log, sizeof(add_log));

    if (res_web == -1)
    {
        printf("Error bind");
        exit(2);
    }

    if (res_log == -1)
    {
        printf("Error bind");
        exit(2);
    }

    fd_set groupe1;
    
    while (1)
    {
        FD_ZERO(&groupe1); /*initialise le groupe1*/
        FD_SET(sock_web, &groupe1);
        FD_SET(sock_log, &groupe1);
        select(sock_log + 1, &groupe1, NULL, NULL, 0);

        if (FD_ISSET(sock_web, &groupe1) == 1)
        {
            sock_web_service = accept(sock_web, (struct sockaddr *)&add_web, (socklen_t *)&taille_web);

            if (sock_web_service < 0)
            {
                printf("Error accept");
                close(sock_web_service);
                exit(3);
            }
            else
            {
                read(sock_web_service, &groupe1, sizeof(groupe1));
            }
        }
        if (FD_ISSET(sock_log, &groupe1) == 1)
        {
            sock_log_service = accept(sock_log, (struct sockaddr *)&add_log, (socklen_t *)&taille_log);

            if (sock_log_service < 0)
            {
                printf("Error accept");
                close(sock_log_service);
                exit(3);
            }
            else
            {
                read(sock_log_service, &groupe1, sizeof(groupe1));
            }
        }
    }

    return 0;
}