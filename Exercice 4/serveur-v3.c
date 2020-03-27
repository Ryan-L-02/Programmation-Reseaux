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
#include <time.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <sys/stat.h> /*open() fstat()*/
#include <fcntl.h>    /*open()*/

#define ERROR -1
#define WEB 1
#define LOG 2
#define IPv4 16
#define BUFFER_SIZE 100
#define TAILLE 1000

void request_file(char buffer[], char fichier[])
{
    int i = 0, j = 0;
    for (i = 5; i < BUFFER_SIZE; i++)
    {
        fichier[j] = buffer[i];
        j++;

        if (buffer[i + 1] == 'H' && buffer[i + 2] == 'T' && buffer[i + 3] == 'T' && buffer[i + 4] == 'P')
        {
            break;
        }
    }
}

void create_log()
{
    remove("log_file.html");
    FILE *fichier = NULL;
    fichier = fopen("log_file.html", "a+");
    if (fichier != NULL)
    {
        fprintf(fichier, "<!DOCTYPE html>\n<html lang=\"fr\">\n<meta charset=\"UTF-8\">\n<head>\n<title>\nExerice 4 - LOG\n</title>\n</head>\n<body>\n<center>\n<h1 style=\"font-family:verdana;\">LOG FILE</h1>\n</center>\n</body>\n</html>\n\n");
    }
    fclose(fichier);
}

void write_log(char buffer[], struct sockaddr_in addIP, int web_log)
{
    FILE *fichier = NULL;
    struct sockaddr_in *add_client_v4;
    struct in_addr add_ip_v4;
    char host[IPv4];
    time_t heure;
    struct tm *heure_info;
    char page[BUFFER_SIZE];

    fichier = fopen("log_file.html", "a+");

    if (fichier != NULL)
    {
        add_client_v4 = (struct sockaddr_in *)&addIP;
        add_ip_v4 = add_client_v4->sin_addr;
        inet_ntop(AF_INET, &add_ip_v4, host, IPv4);

        time(&heure);
        heure_info = localtime(&heure);

        if (web_log == WEB)
        {
            fprintf(fichier, "<p style=\"font-family:verdana;\"><strong>Adresse IP :</strong> ");
            fprintf(fichier, "%s", host);
            fprintf(fichier, " - <strong>Date :</strong> ");
            fprintf(fichier, "%s", asctime(heure_info));

            request_file(buffer, page);

            if (page[0] == ' ')
            {
                fprintf(fichier, "- <strong>Requête :</strong> /index.html");
            }
            else
            {
                fprintf(fichier, "- <strong>Requête :</strong> /");
                fprintf(fichier, "%s", page);
            }

            fprintf(fichier, "</p>");
            fprintf(fichier, "\n\n");
            fclose(fichier);
        }

        if (web_log == LOG)
        {
            fprintf(fichier, "<p style=\"font-family:verdana;\"><strong>Adresse IP :</strong> ");
            fprintf(fichier, "%s", host);
            fprintf(fichier, " - <strong>Date :</strong> ");
            fprintf(fichier, "%s", asctime(heure_info));
            fprintf(fichier, "- <strong>Requête :</strong> /log_file.html");
            fprintf(fichier, "</p>");
            fprintf(fichier, "\n\n");
            fclose(fichier);
        }
    }
    else
    {
        printf("Impossible d'ouvrir le fichier log_file.html\n");
    }
}

void reponseHTTP(char buffer[], int web_log, char response[])
{
    int fichier;
    char page[BUFFER_SIZE];

    if (web_log == WEB)
    {
        request_file(buffer, page);

        if (page[0] == ' ' || strcmp(page, "index.html") == 1)
        {
            fichier = open("index.html", O_RDONLY);
        }
        else
        {
            fichier = open("error.html", O_RDONLY);
        }
    }

    if (web_log == LOG)
    {
        fichier = open("log_file.html", O_RDONLY);
    }

    struct stat file_stat;

    if (fstat(fichier, &file_stat) < 0)
    {
        perror("fstat()");
        exit(errno);
    }

    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type : text/HTML\r\nContent-Length : %ld\r\n\r\n", file_stat.st_size);

    close(fichier);
}

int main(int argc, char *argv[])
{
    int sock_web, sock_web_service;
    int sock_log, sock_log_service;
    struct sockaddr_in add_web;
    struct sockaddr_in add_log;
    socklen_t taille_web = sizeof(add_web);
    socklen_t taille_log = sizeof(add_log);
    char buffer_web[BUFFER_SIZE];
    char buffer_log[BUFFER_SIZE];

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

    if (listen(sock_web, 5) == ERROR)
    {
        printf("Erreur listen web\n");
        perror("listen()");
        exit(errno);
    }

    if (listen(sock_log, 5) == ERROR)
    {
        printf("Erreur listen log\n");
        perror("listen()");
        exit(errno);
    }

    fd_set groupe1;
    create_log();

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
            sock_web_service = accept(sock_web, (struct sockaddr *)&add_web, &taille_web);

            if (sock_web_service == ERROR)
            {
                printf("Erreur accept web\n");
                perror("accept()");
                exit(errno);
            }

            if (read(sock_web_service, &buffer_web, sizeof(buffer_web)) == ERROR)
            {
                printf("Erreur read web\n");
                perror("read()");
                exit(errno);
            }

            write_log(buffer_web, add_web, WEB);

            char response[TAILLE];
            reponseHTTP(buffer_web, WEB, response);

            if (send(sock_web_service, response, strlen(response), 0) < 0)
            {
                perror("send()");
                exit(errno);
            }

            FILE *fichier = NULL;
            char name[BUFFER_SIZE] = "";
            request_file(buffer_web, name);
            if (name[0] == ' ' || strcmp(name, "index.html") == 1)
            {
                fichier = fopen("index.html", "r");
            }
            else
            {
                fichier = fopen("error.html", "r");
            }

            char chaine[TAILLE];
            if (fichier != NULL)
            {
                while (fgets(chaine, TAILLE, fichier) != NULL)
                {
                    if (send(sock_web_service, chaine, strlen(chaine), 0) < 0)
                    {
                        perror("send()");
                        exit(errno);
                    }
                }

                fclose(fichier);
            }

            close(sock_web_service);
        }

        if (FD_ISSET(sock_log, &groupe1) == 1)
        {
            sock_log_service = accept(sock_log, (struct sockaddr *)&add_log, &taille_log);

            if (sock_log_service == ERROR)
            {
                printf("Erreur accept log\n");
                perror("accept()");
                exit(errno);
            }

            if (read(sock_log_service, &buffer_log, sizeof(buffer_log)) == ERROR)
            {
                printf("Erreur read log\n");
                perror("read()");
                exit(errno);
            }

            write_log(buffer_log, add_log, LOG);

            char response[TAILLE];
            reponseHTTP(buffer_log, LOG, response);

            if (send(sock_log_service, response, strlen(response), 0) < 0)
            {
                perror("send()");
                exit(errno);
            }

            FILE *fichier = NULL;
            fichier = fopen("log_file.html", "r");
            char chaine[TAILLE] = "";
            if (fichier != NULL)
            {
                while (fgets(chaine, TAILLE, fichier) != NULL)
                {
                    if (send(sock_log_service, chaine, strlen(chaine), 0) < 0)
                    {
                        perror("send()");
                        exit(errno);
                    }
                }

                fclose(fichier);
            }

            close(sock_log_service);
        }
    }

    close(sock_web);
    close(sock_log);
    return 0;
}