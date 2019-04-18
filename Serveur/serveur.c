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

// Port(s) utilisé par le socket
int dS;
int dSC1;
int dSC2;

//Déclaration des threads
pthread_t c1toc2;
pthread_t c2toc1;

struct sockaddr_in aC1;
struct sockaddr_in aC2;
socklen_t lg = sizeof(struct sockaddr_in);

struct arg_struct {
        int arg1;
        int arg2;
    };

int attendreConnexion(int numClient);
void envoyerMessage(int numClient,char *buffer);
void closeAllPort();
void recevoirMessage(int numClient,char *bufferReception);
void init_socket();
struct sockaddr_in init_server(int port);
void bind_server(struct sockaddr_in ad);
void listen_server(int nbConnexion);
static void* transmettre(void * args);
void gestionDecoClient(int numClient);


int main () {
    printf("Démarrage du serveur \n");
    init_socket();
    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, closeAllPort);

    // définition des paramètres du serveur
    struct sockaddr_in ad = init_server(44573);

    //déclaration de variables annexes
    int booleen1 = 1;
    int nbConnexion = 2;

    struct arg_struct *paraThreadc1toc2 = (struct arg_struct *)malloc(sizeof(struct arg_struct));

    paraThreadc1toc2->arg1 = 1;
    paraThreadc1toc2->arg2 = 2;
    
    struct arg_struct *paraThreadc2toc1 = (struct arg_struct *)malloc(sizeof(struct arg_struct));;
    paraThreadc2toc1->arg1 = 2;
    paraThreadc2toc1->arg2 = 1;

    bind_server(ad);
    listen_server(2);

    //Connexion du client 1 et envoie du numéro
    printf("En attente du client 1\n");
    dSC1 = attendreConnexion(1);

    //Connexion du client 1 et envoie du num
    //char buffer1[TAILLE_BUFFER];
    //char buffer2[TAILLE_BUFFER];éro
    printf("En attente du client 2\n");
    dSC2 = attendreConnexion(2);


    //Les threads
    pthread_create (&c1toc2, NULL, transmettre, (void *)paraThreadc1toc2);
    pthread_create (&c2toc1, NULL, transmettre, (void *)paraThreadc2toc1);
    pthread_join (c1toc2, NULL);
    pthread_join (c2toc1, NULL);

    while (1){

    }
    closeAllPort();

    printf("Fermeture du serveur \n");

    return 0;
}

/*
*func transmettre : int ->
*Transmet le message du client donné en parametre à l'autre client
*/
static void* transmettre(void * args){
    char buffer[TAILLE_BUFFER];
	//Client qui envoie message - il faut receptionner son message
    int numClient1 = (int)((struct arg_struct*) args)->arg1;
	//Client qui reçoit message - il faut lui envoyer le message
    int numClient2 = (int)((struct arg_struct*) args)->arg2;  
    while (1) {
        //printf("Attente d'un message ... \n ");
        recevoirMessage(numClient1, buffer);
		if (strcmp(buffer, "fin") != 0) {
			printf("Message du client %d : %s \n", numClient1, buffer);
			envoyerMessage(numClient2, buffer);
		}
        
    } 
}

/*
* func attendreConnexion : int ->
* Attend une connexion provenant d'un client
*/
int attendreConnexion(int numClient) {
    int cli = accept(dS, (struct sockaddr*)&aC1,&lg);
    if(cli == -1) {
        perror("attendreConnexion");
        exit(1);
    } else {
        printf("Socket client %d ouvert\n", numClient);
        if (numClient == 1) {
            dSC1 = cli;
            envoyerMessage(1, "1");
			envoyerMessage(1, "start");
        } else if (numClient == 2) {
            dSC2 = cli;
            envoyerMessage(2, "2");
			envoyerMessage(2, "start");
        } else {
            perror("attendreConnexion");
            exit(1);
        }
        return cli;
    }
}

/*
* func gestionDecoClient : int, char[] ->
* Gère la deconnexion d'un client
* Attend la connexion d'un nouveau client
* Envoie au nouveau client le dernier message envoyé
*/
void gestionDecoClient(int numClient) {
    printf("Client %d deconnecté, en attente d'un nouveau client\n", numClient);
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
    // Envoie du message au client voulu.
    if (numClient == 1) {
        res = send(dSC1, buffer, strlen(buffer)+1, 0);
    } else if (numClient == 2) {
        res = send(dSC2, buffer, strlen(buffer)+1, 0);
    } else {
        perror("Mauvais numéro client");
        exit(1);
    }
    // Vérification du retour de la fonction
    if (res < 0) {
        perror("envoyerMessage");
        exit(1);
    } else {
        printf("Message envoyé : %s\n", buffer);
    }
}



/*
* func closeAllPort : ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
* Envoie aux deux client "exit" et ferme tout les ports
*/
void closeAllPort() {
    envoyerMessage(1, "exit");
    sleep(1);
    pthread_cancel(c1toc2);
    envoyerMessage(2, "exit");
    pthread_cancel(c2toc1);
    close(dSC1);
    close(dSC2);
    close(dS);
    printf("\nbye !\n");
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
    if (numClient == 1) {
        res = recv(dSC1, bufferReception, TAILLE_BUFFER, 0);
    } else if (numClient == 2) {
        res = recv(dSC2, bufferReception, TAILLE_BUFFER, 0);
    } else {
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
void listen_server(int nbConnexion) {
    if ((listen(dS,nbConnexion)) == -1) {
        perror("listen");
        exit(1);
    }
}
