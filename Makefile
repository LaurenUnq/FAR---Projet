Server : Serveur/serveur.c
	clear
	gcc -o Serveur/serveur Serveur/serveur.c -pthread
	gcc -o Client/client Client/client.c -pthread
