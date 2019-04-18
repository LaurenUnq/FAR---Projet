#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <pwd.h> 

// Taille du buffer
#define TAILLE_BUFFER 4096
//nb de connexions
#define n 10

// Port(s) utilisé par le socket
const int NBCLIENT = n;
int dS;
int dSC[n];
struct sockaddr_in aC[n];
//table de connexion : 1 connecté, 0 sinon
int tabCo[n];

//Déclaration des threads
pthread_t tabThread[n];

socklen_t lg = sizeof(struct sockaddr_in);

struct arg_struct {
        int thnumCli;
        int arg2;
    };

int attendreConnexion(int numClient);
void envoyerMessage(int numClient,char *buffer);
void closeAllPort();
void recevoirMessage(int numClient,char *bufferReception);
void init_socket();
struct sockaddr_in init_server(int port);
void bind_server(struct sockaddr_in ad);
void listen_server(int NBCLIENT);
static void* transmettre(void * args);
void gestionDecoClient(int numClient);
void envoyerMessageCom(int numClientDest, char *buffer, char* numClientSourc);


int main () {
    printf("Démarrage du serveur \n");
	int i;

	for (i = 0; i < NBCLIENT; i++) {
		tabCo[i] = 0;
	}
    init_socket();
    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, closeAllPort);

    // définition des paramètres du serveur
    struct sockaddr_in ad = init_server(44573);

/*
	//struct arg_struct * tabParThread[NBCLIENT];
	int * tabParThread[NBCLIENT];
	for (i = 0; i < NBCLIENT; i++) {
	//	(tabParThread[i])->thnumCli = i;
		tabParThread[i] = i;
	}
*/

    bind_server(ad);
    listen_server(NBCLIENT);
	printf("En attente de client \n");
    //Connexion des clients et envoie du numéro

	for (i = 0; i < NBCLIENT; i++) {
		dSC[i] = attendreConnexion(i);
		//Les threads
		pthread_create((pthread_t*)&tabThread[i], NULL, transmettre, (void *)i);
		
	}

	for (i = 0; i < NBCLIENT; i++) {
		pthread_join((pthread_t)tabThread[i], NULL);
	}

    

    while (1){

    }
    closeAllPort();

    printf("Fermeture du serveur \n");
	
    return 0;
}

/*
*func transmettre : int ->
*Transmet le message du client donné en parametre à tous les autres clients
*/
static void* transmettre(void * args){
    char buffer[TAILLE_BUFFER];
	//Client qui envoie message - il faut receptionner son message
    int numClient = (int) args;
	char num[3];
	sprintf(num, "%d", numClient);
    while (1) {
        //printf("Attente d'un message ... \n ");
        recevoirMessage(numClient, buffer);
		if (strcmp(buffer, "fin") != 0) {
			printf("Message du client %d : %s \n", numClient, buffer);
			for (int z = 0; z < NBCLIENT; z++) {
				if (z != numClient) {
					envoyerMessageCom(z, buffer, num);
				}
				
			}
		}
        
    } 
}

/*
* func attendreConnexion : int ->
* Attend une connexion provenant d'un client
*/
int attendreConnexion(int numClient) {
    int cli = accept(dS, (struct sockaddr*)&aC[numClient],&lg);
    if(cli == -1) {
        perror("attendreConnexion");
        exit(1);
    } 
	else {
		printf("Socket client %d ouvert\n", numClient);
		dSC[numClient] = cli;
		tabCo[numClient] = 1;
		char num[3];
		sprintf(num, "%d", numClient);
		printf("Connexion du client %s \n", num);
		envoyerMessage(numClient, num);
		envoyerMessage(numClient, "start");
    } 
    return cli;
    
}

/*
* func gestionDecoClient : int, char[] ->
* Gère la deconnexion d'un client
* Attend la connexion d'un nouveau client
* Envoie au nouveau client le dernier message envoyé
*/
void gestionDecoClient(int numClient) {
    printf("Client %d deconnecté, en attente d'un nouveau client\n", numClient);
	tabCo[numClient] = 0;
    attendreConnexion(numClient);
}

/*
 * func envoyerMessage : int, char[] ->
 * Envoie a partir d'un socket, une chaine de charactère un message
 */
void envoyerMessage(int numClient,char *buffer) {
    if (strlen(buffer) > TAILLE_BUFFER) {
        perror("Message trop long");
        closeAllPort();
        exit(1);
    }
    int res;
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    // Envoie du message au client passé en paramètre s'il est connecté
	if (tabCo[numClient] == 1) {
		res = send(dSC[numClient], buffer, strlen(buffer) + 1, 0);

		// Vérification du retour de la fonction
		if (res < 0) {
			perror("envoyerMessage");
			exit(1);
		}
		else {
			printf("Message envoyé au client %d : %s\n", numClient, buffer );
		}
	}
	if (numClient <0 || numClient >NBCLIENT)
	{
		perror("Mauvais numéro client");
		exit(1);
	}

}

/*
 * func envoyerMessageCom : int, char[] ->
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Dans ce message est figuré le numéro du client
 */
void envoyerMessageCom(int numClientDest, char *buffer, char* numClientSourc) {
	if (strlen(buffer) > TAILLE_BUFFER) {
		perror("Message trop long");
		closeAllPort();
		exit(1);
	}
	int res;
	// + d'infos sur send()  : https://man.developpez.com/man2/send/
	// Envoie du message au client passé en paramètre s'il est connecté
	if (tabCo[numClientDest] == 1) {
		char msg[TAILLE_BUFFER] = "message du client ";
		strcat(msg, numClientSourc);
		strcat(msg, " : ");
		strcat(msg, buffer);
		res = send(dSC[numClientDest], msg, strlen(msg) + 1, 0);

		// Vérification du retour de la fonction
		if (res < 0) {
			perror("envoyerMessage");
			exit(1);
		}
		else {
			printf("Message envoyé au client %d : %s\n", numClientDest, buffer);
		}
	}
	if (numClientDest <0 || numClientDest >NBCLIENT)
	{
		perror("Mauvais numéro client");
		exit(1);
	}

}



/*
* func closeAllPort : ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
* Envoie aux deux client "exit" et ferme tout les ports
*/
void closeAllPort() {
	int i;
	for (i = 0; i < NBCLIENT; i++) {
		envoyerMessage(i, "exit");
		pthread_cancel(tabThread[i]);
		close(dSC[i]);
		tabCo[i] = 0;
	}
    close(dS);
    printf("\n bye !\n");
    exit(0);
}

/*
* func recevoirMessage : int, char[], char[]->
* Recoit le message d'un client à partir du numéro du client
* bufferReception est un tableau de charactère pour recevoir le message
* Attend la connexion d'un nouveau client si le contenue de la chaine est "fin "
*/

void recevoirMessage(int numClient,char *bufferReception) {
    int res;

    // Reception du message venant du client
	res = recv(dSC[numClient], bufferReception, TAILLE_BUFFER, 0);
	if (numClient <0 || numClient >NBCLIENT)
	{
		perror("Mauvais numéro client");
		exit(1);
	}

    // Vérification du retour de la fonction
    if (res < 0) {
        perror("recevoirMessage");
        exit(1);
    } else {
        // Remplace le dernier charactère par '\0' pour éviter toute erreur
        // lié a une chaine trop longue ou mal formattée
        bufferReception[res] = '\0';
        if (strcmp( bufferReception, "fin") == 0) {
            // Si un client se deconnecte, lance la procedure de gestion de deconnexion d'un client
            gestionDecoClient(numClient);
        } else if (strcmp( bufferReception, "exit") == 0) {
            // Envoie de exit avec un espace, sinon le client va se deconnecter a la reception du message
            strcpy(bufferReception, "exit ");
        }
    }
}

/*
* func init_socket : ->
* Initialise le socket
*/
void init_socket() {
    //Definition du protole (TCP ici)
    if ((dS = socket(PF_INET,SOCK_STREAM,0)) == -1) {
        perror("socket");
        exit(1);
    }
}

/*
* func init_server : ->
* Initialise les paramètres du serveur
*/
struct sockaddr_in init_server(int port) {
    // définition des paramètres du serveur
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port); //on rentre le port du serveur
    return ad;
}

/*
* func bind_server : ->
* Assigne l'adresse ip et le port au serveur
*/
void bind_server(struct sockaddr_in ad) {
    if ((bind(dS, (struct sockaddr*)&ad, sizeof(ad))) == -1) {
        perror("bind");
        exit(1);
    }
}

/*
* func listen_server : ->
* Definit le nombre de connexion maximum accepté par le socket
*/
void listen_server(int nbCo) {
    if ((listen(dS,nbCo)) == -1) {
        perror("listen");
        exit(1);
    }
}
