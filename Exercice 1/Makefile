test: client.o serveur.o
	$(CC) -o test client.o serveur.o

client: client.o
	$(CC) -o client client.o

serveur: serveur.o
	$(CC) -o serveur serveur.o

#################################################################################

client.o: client.c
	$(CC) -O -Wall -W -ansi -c client.c

serveur.o: serveur.c
	$(CC) -O -Wall -W -ansi -c serveur.c

#################################################################################

clean:
	rm -f *~ *.o
	rm -f test
	rm -f client
	rm -f serveur