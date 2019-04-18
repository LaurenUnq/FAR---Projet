#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

// Taille du buffer
#define TAILLE_BUFFER 1024


// Port(s) utilisé par le socket
int dS;

// threads
pthread_t thread_ecrire;
pthread_t thread_lire;

int envoyerMessage(int dSClient,char *buffer);
void closeAllPort();
int recevoirMessage(int dSClient,char *buffer);
void ecrireMessage(char *msg);
void deconnexion();
static void * ecrire();
static void * lire();

int main () {

    //Definition du protole
    dS = socket(PF_INET,SOCK_STREAM,0);

    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, deconnexion);

    //Connexion à un serveur
    struct sockaddr_in adServ;
    int res;
    adServ.sin_family = AF_INET;
    adServ.sin_port = htons(44573); //on rentre le port du serv
    //Attention, le dernier chiffre de l'adresse correspond à celui
    //de la machine sur laquelle on est !!
    //à changer si on change de machine
    res = inet_pton(AF_INET,"127.0.0.1",&(adServ.sin_addr));

    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(dS, (struct sockaddr *) &adServ, lgA);


    char buffer[4096];  // ce que l'utilisateur va entrer 32 caracteres sans le caract. de fin de chaine
    char NumClient[3];  // Client 1 ou client 2

    // Obtenir le numero du client
    recv(dS, NumClient, 3,0);
    printf("Votre numero de Client est : %s \n ",NumClient);

    // Attente de l'accord du serveur pour commencer
    printf("Veuillez attendre la confirmation du seveur pour commencer ... \n");
    // Attente du role
    recv(dS, buffer, sizeof(buffer),0);
    if (strcmp(buffer, "start" ) != 0) {
        perror("Bad start message");
        closeAllPort();
    } else {
        printf("Vous pouvez commencer \n");
    }

    //Debut de la boucle d'action read/write ...
    printf("Pour mettre fin à la connexion, tapez fin \n");
    // Creer thread lecture
    pthread_create (&thread_ecrire, NULL, ecrire, NULL);
    // Creer thread ecriture
    pthread_create (&thread_lire, NULL, lire, NULL);
    // Joindre thread lecture
    pthread_join (thread_ecrire, NULL);
    // Joindre thread ecriture
    pthread_join (thread_lire, NULL);
    // Attendre que l'utilisateur appuie sur CTRL+C
    while (1);

    return 0;
}

/*
 * func envoyerMessage : Socket, char[] -> int
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Renvoie 0 si aucune Erreur
 * Arrete le programme et donne l'erreur si erreur
 * Renvoie -2 si chaine trop longue
 */
int envoyerMessage(int dSClient,char *buffer) {
    if (strlen(buffer) > TAILLE_BUFFER) {
        return -2;
    }
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    if (send(dSClient, buffer, strlen(buffer)+1, 0) < 0) {
        perror("envoyerMessage");
        closeAllPort();
    } else {
        printf("Message envoyé : %s\n", buffer);
        if (strcmp(buffer, "fin")) {
            closeAllPort();
        }
        return 0;
    }
}

/*
* func closeAllPort : ->
* Ferme tout les ports
*/
void closeAllPort() {
    close(dS);
    printf("\nbye !\n");
    exit(0);
}

/*
* func deconnexion : ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
*/
void deconnexion() {
    envoyerMessage(dS, "fin");
    sleep(1);
    closeAllPort();
}


/*
* func recevoirMessage : Socket, char[], int -> int
* Envoie a partir d'un socket, un tableau de charactère un message
* Renvoie 0 si aucune Erreur
* Renvoie -1 si erreur
* Renvoie -2 si chaine trop longue
* Arrete le programme via la fonction closeAllPort si le contenue de la chaine est "exit"
*/
int recevoirMessage(int dSClient,char *buffer) {
    int n = recv(dSClient, buffer, TAILLE_BUFFER, 0);
    if (n < 0) {
        perror("recevoirMessage");
        closeAllPort();
    } else {
        buffer[n] = '\0';
        if (strcmp( buffer, "exit") == 0) {
            closeAllPort();
        }
        return 0;
    }
}

static void * lire() {
    char buffer[4096];
    while(1) {
        recevoirMessage(dS, buffer);
        printf("Message reçu : %s \n", buffer);
        printf("\n");
    }
}

static void * ecrire()  {
    char buffer[4096];
    while (1) {
        printf("Ecrivez votre message => ");
        ecrireMessage(buffer);
        envoyerMessage(dS, buffer);
        printf("\n");
    }
}

/*
 * func ecrireMessage : char[] ->
 * Recupère une chaine de charactère entrée par l'utilisateur et la stoque à l'adresse fourni
 */
void ecrireMessage(char *msg) {
    fgets(msg, TAILLE_BUFFER, stdin);
    char *toReplace = strchr(msg, '\n');
    *toReplace = '\0';
}
