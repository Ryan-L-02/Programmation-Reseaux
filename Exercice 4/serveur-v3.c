#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#define ERROR -1

int main(int argc, char *argv[])
{
    int sock_web, sock_log;
    struct sockaddr_in add_web;
    struct sockaddr_in add_log;
    int sock_web_service;
    int sock_log_service;
    socklen_t taille_web = sizeof(add_web);
    socklen_t taille_log = sizeof(add_log);

    sock_web = socket(AF_INET, SOCK_STREAM, 0);
    sock_log = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_web == ERROR)
    {
        printf("Erreur creation socket web\n");
        perror("socket()");
        exit(errno);
    }

    if (sock_log == ERROR)
    {
        printf("Erreur creation socket log\n");
        perror("socket()");
        exit(errno);
    }

    add_web.sin_family = AF_INET;
    add_web.sin_port = htons(atoi(argv[1]));
    add_web.sin_addr.s_addr = INADDR_ANY;

    add_log.sin_family = AF_INET;
    add_log.sin_port = htons(atoi(argv[2]));
    add_log.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_web, (struct sockaddr *)&add_web, sizeof(add_web)) == ERROR)
    {
        printf("Erreur bind web\n");
        perror("bind()");
        exit(errno);
    }

    if (bind(sock_log, (struct sockaddr *)&add_log, sizeof(add_log)) == ERROR)
    {
        printf("Erreur bind log\n");
        perror("bind()");
        exit(errno);
    }

    fd_set groupe1;

    while (1)
    {
        FD_ZERO(&groupe1); /*initialise le groupe1*/
        FD_SET(sock_web, &groupe1);
        FD_SET(sock_log, &groupe1);

        if (select(sock_log + 1, &groupe1, NULL, NULL, 0) < 0)
        {
            perror("select()");
            exit(errno);
        }

        if (FD_ISSET(sock_web, &groupe1) == 1)
        {
            sock_web_service = accept(sock_web, (struct sockaddr *)&add_web, (socklen_t *)&taille_web);

            if (sock_web_service == ERROR)
            {
                printf("Erreur accept web\n");
                perror("accept()");
                exit(errno);
            }
            else
            {
                if (read(sock_web_service, &groupe1, sizeof(groupe1)) == ERROR)
                {
                    printf("Erreur read web\n");
                    perror("read()");
                    exit(errno);
                }
            }
        }
        if (FD_ISSET(sock_log, &groupe1) == 1)
        {
            sock_log_service = accept(sock_log, (struct sockaddr *)&add_log, (socklen_t *)&taille_log);

            if (sock_log_service == ERROR)
            {
                printf("Erreur accept log\n");
                perror("accept()");
                exit(errno);
            }
            else
            {
                if (read(sock_log_service, &groupe1, sizeof(groupe1)) == ERROR)
                {
                    printf("Erreur read log\n");
                    perror("read()");
                    exit(errno);
                }
            }
        }
    }

    return 0;
}