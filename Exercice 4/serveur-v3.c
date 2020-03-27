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

/*Fonction qui va nous permettre d'isoler le nom du fichier d'une requête HTTP*/
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

/*Fonction qui va être utiliser au début du programme pour créer un fichier de log vide*/
/*L'ancien fichier de log, s'il existe, sera supprimé*/
void create_log()
{
    remove("log_file.html");
    FILE *fichier = NULL;
    fichier = fopen("log_file.html", "a+");

    if (fichier != NULL)
    {
        /*L'entête du fichier de log est à base d'html, pour améliorer la mise en page sur navigateur*/
        fprintf(fichier, "<!DOCTYPE html>\n<html lang=\"fr\">\n<meta charset=\"UTF-8\">\n<head>\n<title>\nExerice 4 - LOG\n</title>\n</head>\n<body>\n<center>\n<h1 style=\"font-family:verdana;\">LOG FILE</h1>\n</center>\n</body>\n</html>\n\n");
        fclose(fichier);
    }
    else
    {
        printf("Impossible de créer le fichier log_file.html\n");
    }
}

/*Cette fonction va nous permettre de remplir le fichier de log*/
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
        /*On récupère l'adresse IP du client*/
        add_client_v4 = (struct sockaddr_in *)&addIP;
        add_ip_v4 = add_client_v4->sin_addr;
        inet_ntop(AF_INET, &add_ip_v4, host, IPv4);

        /*On récupère l'heure de la connexion*/
        time(&heure);
        heure_info = localtime(&heure);

        /*Si c'est une requête sur le port WEB*/
        if (web_log == WEB)
        {
            /*On écrit dans le fichier l'adresse IP puis la date*/
            fprintf(fichier, "<p style=\"font-family:verdana;\"><strong>Adresse IP :</strong> ");
            fprintf(fichier, "%s", host);
            fprintf(fichier, " - <strong>Date :</strong> ");
            fprintf(fichier, "%s", asctime(heure_info));

            /*On récupère le nom du fichier qui a été demandé par le client*/
            request_file(buffer, page);

            /*Si aucun nom de fichier est spécifié par le client, on écrira le nom du fichier par défaut : index.html*/
            /*Sinon, on écrira dans le fichier de log le nom du fichier qui a été demandé*/
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

        /*Si c'est une requête sur le port LOG*/
        if (web_log == LOG)
        {
            /*On écrit l'adresse IP et la date*/
            fprintf(fichier, "<p style=\"font-family:verdana;\"><strong>Adresse IP :</strong> ");
            fprintf(fichier, "%s", host);
            fprintf(fichier, " - <strong>Date :</strong> ");
            fprintf(fichier, "%s", asctime(heure_info));

            /*Comme la connexion a eu lieu sur le port LOG, le seul fichier accessible sera log_file.html*/
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

/*Dans cette fonction, nous allons créer la réponse HTTP qui sera envoyé par le serveur*/
/*avant le contenu de n'importe quel fichier demandé par le client*/
void reponseHTTP(char buffer[], int web_log, char response[])
{
    int fichier;
    char page[BUFFER_SIZE];

    /*Si la connexion a eu lieu sur le port WEB*/
    if (web_log == WEB)
    {
        /*On récupère le nom du fichier qui a été demandé par le client*/
        request_file(buffer, page);

        /*Si aucun nom de fichier est spécifié OU que le fichier demandé est index.html (page par défaut)*/
        /*Sinon, on renvoit la page error.html, qui indique que la page demandé n'existe pas sur le serveur*/
        if (page[0] == ' ' || strcmp(page, "index.html") == 1)
        {
            fichier = open("index.html", O_RDONLY);
        }
        else
        {
            fichier = open("error.html", O_RDONLY);
        }
    }

    /*Si la connexion a eu lieu sur le port LOG*/
    if (web_log == LOG)
    {
        fichier = open("log_file.html", O_RDONLY);
    }

    struct stat file_stat;

    /*On cherche la taille du fichier qui a été demandé par le client*/
    if (fstat(fichier, &file_stat) < 0)
    {
        perror("fstat()");
        exit(errno);
    }

    /*On contruit la réponse HTTP*/
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type : text/HTML\r\nContent-Length : %ld\r\n\r\n", file_stat.st_size);

    close(fichier);
}

int main(int argc, char *argv[])
{
    /*Création d'un serveur TCP qui écoute sur deux ports :*/
    /*Le premier correspond au port WEB*/
    /*Le deuxième correspond au port LOG, et enverra uniquement le fichier de log, peut importe le fichier demandé*/
    int sock_web, sock_web_service;
    int sock_log, sock_log_service;
    struct sockaddr_in add_web;
    struct sockaddr_in add_log;
    socklen_t taille_web = sizeof(add_web);
    socklen_t taille_log = sizeof(add_log);
    char buffer_web[BUFFER_SIZE];
    char buffer_log[BUFFER_SIZE];

    fd_set groupe1;
    FILE *fichier = NULL;
    char response[TAILLE];
    char chaine[TAILLE];
    char name[BUFFER_SIZE];

    /*Création du socket pour la partie WEB et gestion de l'erreur*/
    sock_web = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_web == ERROR)
    {
        printf("Erreur creation socket web\n");
        perror("socket()");
        exit(errno);
    }

    /*Création du socket pour la partie LOG et gestion de l'erreur*/
    sock_log = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_log == ERROR)
    {
        printf("Erreur creation socket log\n");
        perror("socket()");
        exit(errno);
    }

    /*On configure la partie réseau WEB, comme l'adresse IP et le port (qui est donné en argument 1)*/
    add_web.sin_family = AF_INET;
    add_web.sin_port = htons(atoi(argv[1]));
    add_web.sin_addr.s_addr = INADDR_ANY;

    /*On configure la partie réseau LOG, comme l'adresse IP et le port (qui est donné en argument 2)*/
    add_log.sin_family = AF_INET;
    add_log.sin_port = htons(atoi(argv[2]));
    add_log.sin_addr.s_addr = INADDR_ANY;

    /*On lie la socket WEB à la partie "réseau WEB" (IP-Port), tout en gérant le code de retour*/
    if (bind(sock_web, (struct sockaddr *)&add_web, sizeof(add_web)) == ERROR)
    {
        printf("Erreur bind web\n");
        perror("bind()");
        exit(errno);
    }

    /*On lie la socket LOG à la partie "réseau LOG" (IP-Port), tout en gérant le code de retour*/
    if (bind(sock_log, (struct sockaddr *)&add_log, sizeof(add_log)) == ERROR)
    {
        printf("Erreur bind log\n");
        perror("bind()");
        exit(errno);
    }

    /*On indique à la socket WEB qu'elle doit être à l'écoute de potentielles connexions, tout en gérant le code de retour*/
    if (listen(sock_web, 5) == ERROR)
    {
        printf("Erreur listen web\n");
        perror("listen()");
        exit(errno);
    }

    /*On indique à la socket LOG qu'elle doit être à l'écoute de potentielles connexions, tout en gérant le code de retour*/
    if (listen(sock_log, 5) == ERROR)
    {
        printf("Erreur listen log\n");
        perror("listen()");
        exit(errno);
    }

    /*On crée un fichier de log propre*/
    create_log();

    /*Boucle infini, lorsqu'une connexion est terminé, on en attend une nouvelle sans terminer le programme*/
    while (1)
    {
        FD_ZERO(&groupe1); /*initialise le groupe1*/
        FD_SET(sock_web, &groupe1);
        FD_SET(sock_log, &groupe1);

        /*En fonction du port choisi par le client, on va choisir le bon socket*/
        if (select(sock_log + 1, &groupe1, NULL, NULL, 0) < 0)
        {
            perror("select()");
            exit(errno);
        }

        /*Si la connexion a lieu sur le port WEB*/
        if (FD_ISSET(sock_web, &groupe1) == 1)
        {
            /*On accepte une connexion au serveur, et on récupère les infos du client (IP...), tout en gérant le code de retour*/
            sock_web_service = accept(sock_web, (struct sockaddr *)&add_web, &taille_web);
            if (sock_web_service == ERROR)
            {
                printf("Erreur accept web\n");
                perror("accept()");
                exit(errno);
            }

            /*On lit le contenu de la requête du client, tout en gérant le code de retour*/
            if (read(sock_web_service, &buffer_web, sizeof(buffer_web)) == ERROR)
            {
                printf("Erreur read web\n");
                perror("read()");
                exit(errno);
            }

            /*Comme une connexion a eu lieu, on va l'écrire dans le fichier de log*/
            write_log(buffer_web, add_web, WEB);

            /*On construit l'entête de la réponse qui sera envoyé par le serveur*/
            reponseHTTP(buffer_web, WEB, response);

            /*On envoie cet entête au client; Elle va permettre de prévenir le client que l'on va envoyer de l'html*/
            if (send(sock_web_service, response, strlen(response), 0) < 0)
            {
                perror("send()");
                exit(errno);
            }

            /*On récupère le nom du fichier qui est demandé par le client*/
            request_file(buffer_web, name);

            /*Si on a aucun nom de fichier OU que le fichier demandé est inde.html (page par défaut), on ouvre en lecture index.html*/
            /*Sinon, on ouvre le fichier error.html, qui indique au client que la page demandé n'existe pas*/
            if (name[0] == ' ' || strcmp(name, "index.html") == 1)
            {
                fichier = fopen("index.html", "r");
            }
            else
            {
                fichier = fopen("error.html", "r");
            }

            /*On envoie le contenu du fichier ouvert*/
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

            /*On ferme la connexion*/
            close(sock_web_service);
        }

        /*Si la connexion a lieu sur le port LOG*/
        if (FD_ISSET(sock_log, &groupe1) == 1)
        {
            /*On accepte une connexion au serveur, et on récupère les infos du client (IP...), tout en gérant le code de retour*/
            sock_log_service = accept(sock_log, (struct sockaddr *)&add_log, &taille_log);
            if (sock_log_service == ERROR)
            {
                printf("Erreur accept log\n");
                perror("accept()");
                exit(errno);
            }

            /*On lit le contenu de la requête du client, tout en gérant le code de retour*/
            if (read(sock_log_service, &buffer_log, sizeof(buffer_log)) == ERROR)
            {
                printf("Erreur read log\n");
                perror("read()");
                exit(errno);
            }

            /*Comme une connexion a eu lieu, on va l'écrire dans le fichier de log*/
            write_log(buffer_log, add_log, LOG);

            /*On construit l'entête de la réponse qui sera envoyé par le serveur*/
            reponseHTTP(buffer_log, LOG, response);

            /*On envoie cet entête au client; Elle va permettre de prévenir le client que l'on va envoyer de l'html*/
            if (send(sock_log_service, response, strlen(response), 0) < 0)
            {
                perror("send()");
                exit(errno);
            }

            /*On envoie le contenu du fichier log_file.html*/
            fichier = fopen("log_file.html", "r");
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

            /*On ferme la connexion*/
            close(sock_log_service);
        }
    }

    /*On ferme les sockets avant de terminer le programme*/
    close(sock_web);
    close(sock_log);
    return 0;
}