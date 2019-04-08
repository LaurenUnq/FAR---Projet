#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

// Taille du buffer
#define TAILLE_BUFFER 1024

// Port(s) utilisé par le socket
int dS;
int dSC1;
int dSC2;


/*
 * func envoyerMessage : Socket, char[] -> int
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Renvoie 0 si aucune Erreur
 * Renvoie -1 si erreur
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
* func attendreConnexion : int, struct sockaddr_in, socklen_t -> int
* Attend une connexion provenant d'un client
*/
int attendreConnexion(int dS, struct sockaddr_in *aC1, socklen_t *lg) {
    int cli = accept(dS, (struct sockaddr*)aC1,lg);
    if(cli == -1) {
        perror("attendreConnexion");
        exit(1);
    } else {
        return cli;
    }
}

/*
* func closeAllPort : int ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
*/
void closeAllPort(int sig) {
    envoyerString(dSC1, "exit");
    sleep(1);
    envoyerString(dSC2, "exit");
    close(dSC1);
    close(dSC2);
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
* Arrete le programme via la fonction closeAllPort si le contenue de la chaine est "fin "
*/

int recevoirMessage(int dSClient,char *buffer) {
    int n = recv(dSClient, buffer, TAILLE_BUFFER, 0);
    if (n < 0) {
        perror("recevoirMessage");
        exit(1);
    } else {
        buffer[n] = '\0';
        if (strcmp( buffer, "fin") == 0) {
            closeAllPort(0);
        }
        return 0;
    }
}

void init_socket() {
    //Definition du protole
    if ((dS = socket(PF_INET,SOCK_STREAM,0)) == -1) {
        perror("socket");
        exit(1);
    }
}

struct sockaddr_in init_server(int port) {
    // définition des paramètres du serveur
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port); //on rentre le port du serveur
    return ad;
}

void bind_server(struct sockaddr_in ad) {
    if ((bind(dS, (struct sockaddr*)&ad, sizeof(ad))) == -1) {
        perror("bind");
        exit(1);
    }
}

void listen_server(int nbConnexion) {
    if ((listen(dS,nbConnexion)) == -1) {
        perror("listen");
        exit(1);
    }
}


int main () {
    printf("démarrage du serveur \n");
    init_socket();
    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, closeAllPort);

    // définition des paramètres du serveur
    struct sockaddr_in ad = init_server(44573);

    struct sockaddr_in aC1;
    struct sockaddr_in aC2;
    socklen_t lg = sizeof(struct sockaddr_in);

    //déclaration de variables annexes
    int booleen1 = 1;
    int nbConnexion = 2;
    char buffer[1024];

    bind_server(ad);

    listen_server(2);

    //Connexion du client 1 et envoie du numéro
    printf("En attente du client 1\n");
    dSC1 = attendreConnexion(dS, &aC1,&lg);
    printf("Socket client 1 ouvert\n");
    envoyerString(dSC1, "1");

    //Connexion du client 1 et envoie du numéro
    printf("En attente du client 2\n");
    dSC2 = attendreConnexion(dS, &aC2,&lg);
    printf("Socket client 2 ouvert\n");
    envoyerString(dSC2, "2");

    //Envoi "ok" aux clients 1
    envoyerString(dSC1, "ok");
    // Attendre un peu avant l'envoie du message suivant sinon bug des fois
    sleep(1);
    //Envoi "ok" aux clients 2
    envoyerString(dSC2, "ok");

    int res;
    //printf("étape 3 \n")
    while (1){
        printf("Attente du message du client 1 (pour le Client 2)... \n");
        recevoirMessage(dSC1, buffer);
        printf("Message du client 1 : %s \n", buffer);
        envoyerMessage(dSC2, buffer);

        printf("Attente du message du client 2 (pour le Client 1)... \n");
        recevoirMessage(dSC2, buffer);
        printf("Message du client 2 : %s \n", buffer);
        envoyerMessage(dSC1, buffer);
    }

    close(dSC1);
    close(dSC2);
    close(dS);

    printf("Fermeture du serveur \n");

    return 0;


}
