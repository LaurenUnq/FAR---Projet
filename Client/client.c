#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// Taille du buffer
#define TAILLE_BUFFER 1024

// Port(s) utilisé par le socket
int dS;

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
        exit(1);
    } else {
        return 0;
    }
}

/*
 * func envoyerString : Socket, char[] -> int
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Renvoie 0 si aucune Erreur
 * Renvoie -1 si erreur
 * Renvoie -2 si chaine trop longue
 */
int envoyerString(int dSClient,char char_array[]) {
    if (strlen(char_array) > TAILLE_BUFFER) {
        return -2;
    }
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    if (send(dSClient, char_array, strlen(char_array)+1, 0) < 0) {
        perror("envoyerString");
        exit(1);
    } else {
        printf("Message envoyé au client : %s\n", char_array);
        return 0;
    }
}

/*
* func closeAllPort : int ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
*/
void closeAllPort(int sig) {
    envoyerString(dS, "fin");
    close(dS);
    printf("\nbye !\n");
    exit(0);
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
        exit(1);
    } else {
        buffer[n] = '\0';
        if (strcmp( buffer, "exit") == 0) {
            closeAllPort(0);
        }
        return 0;
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


int main () {

    //Definition du protole
    dS = socket(PF_INET,SOCK_STREAM,0);

    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, closeAllPort);

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


    char buffer[1024];  // ce que l'utilisateur va entrer 32 caracteres sans le caract. de fin de chaine
    char NumClient[3];  // Client 1 ou client 2
    char repServ[3];    // ok

    printf("Etape 0 \n");
    //Envoi d'un message
    /*if (res != -1){
        printf("La connexion a été établie \n");
    }
    else {
        printf("La connexion n'a pas fonctionnée \n");
    }*/

    printf("etape1 \n");
    //Obtenir le numero du client
    recv(dS, NumClient, 3,0);
    printf("Votre numero de Client est : %s \n ",NumClient);

    //attente de l'accord du serveur pour commencer
    printf("Veuillez attendre la confirmation du seveur pour commencer ... \n");
    // Attente du message "ok" du serveur
    recv(dS, buffer, sizeof(buffer),0);
    if (strcmp(buffer, "ok" ) != 0) {
        printf("Message reçu : %s\n", buffer);
        printf("Erreur, le message du serveur devrait etre \"ok\"\n");
        close(dS);
        exit(0);
    } else {
        printf("Vous pouvez commencer \n");
    }

    //Debut de la boucle d'action read/write ...
    printf("Pour mettre fin à la connexion, tapez fin \n");
    while (1){

        //Cas du client 1
        if( strcmp(NumClient, "1" )==0 )  {
            printf("=>");
            ecrireMessage(buffer);
            envoyerMessage(dS, buffer);

            printf("En attente du message du client 2 \n");
            recevoirMessage(dS, buffer);
            printf("Message du client 2 : %s \n", buffer);

        }

        //Cas du client 2
        if( strcmp(NumClient, "2" )==0 )  {
            printf("En attente du message du client 1 \n");
            recevoirMessage(dS, buffer);
            printf("Message du client 1 : %s \n", buffer);

            ecrireMessage(buffer);
            envoyerMessage(dS, buffer);
        }

        printf("\n");
    }

    return 0;

}
