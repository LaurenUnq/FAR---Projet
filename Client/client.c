# include <stdio.h>
# include <sys/types.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

// Alias pour plus de clarté
#define INVALID_SOCKET -1
#define closesocket(s) close(s)
typedef int SOCKET;

// Taille du buffer
#define TAILLE_BUFFER 1024


/*
 * func envoyerMessage : Socket, char[] -> int
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Renvoie 0 si aucune Erreur
 * Renvoie -1 si erreur
 * Renvoie -2 si chaine trop longue
 */
int envoyerMessage(SOCKET *dSClient,char *buffer) {
    if (strlen(buffer) > TAILLE_BUFFER) {
        return -2;
    }
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    if (send(*dSClient, buffer, strlen(buffer), 0) < 0) {
        return -1;
    } else {
        return 0;
    }
}
/*
 * func recevoirMessage : Socket, char[], int -> int
 * Envoie a partir d'un socket, une chaine de charactère et sa taille  un message
 * Renvoie 0 si aucune Erreur
 * Renvoie -1 si erreur
 * Renvoie -2 si chaine trop longue
 */
int recevoirMessage(SOCKET *dSClient,char *buffer) {
    int n = recv(*dSClient, buffer, sizeof(buffer), 0);
    if (n < 0) {
        return -1;
    } else {
        buffer[n] = '\0';
        return 0;
    }
}

void ecrireMessage(char *msg) {
    fgets(msg, TAILLE_BUFFER, stdin);
    char *toReplace = strchr(msg, '\n');
    *toReplace = '\0';
}

// ici on fait entrer des message à l utilisateur jusqu'à ce qu'il
// decide d'exit


// on met l adresse du serveur
// - pour le tp, comme on crée un serveur, on met l'adresse de notre machine
// on lance ensuite le serveur, puis on lance le client et ça fonctionne

int main () {
    // Definition de l'IP du serveur
    char ipServ[15] = "127.0.0.1";
    // Variable de gestion d'erreur
    int res;
    // Création du buffer pour l'envoie/reception de message
    char buffer[TAILLE_BUFFER] = "empty";
    // Variable numClient pour stocker le numéro du client
    char numClient[2];

    // Definition du protole (TCP)
    SOCKET dSClient = socket(PF_INET,SOCK_STREAM,0);
    // Vérification du bon déroulement de la création du Socket
    if (dSClient == INVALID_SOCKET) {
        printf("erreur lors de la création du socket Client\n");
    }
    // Definition des paramètres du serveur
    struct sockaddr_in adrServ;
    adrServ.sin_family = AF_INET;
    adrServ.sin_port = htons(44573); // Définition du port du serveur
    // inet_pton() convertis une chaine de char en IP comprehensible pour le programme
    res = inet_pton(AF_INET, ipServ, &(adrServ.sin_addr));
    socklen_t lgA = sizeof(struct sockaddr_in);
    // Connexion au serveur
    res =0;
    res = connect(dSClient, (struct sockaddr *) &adrServ, lgA);
    // Vérification du bon déroulement de la connexion
    if (res != -1){
        printf("La connexion a été établie \n");
    }
    else {
        printf("La connexion n'a pas fonctionnée \n");
    }
    // Obtenir le numero du client
    res = 0 ;
    res = recevoirMessage(&dSClient, numClient);
    if (res < 0) {
        printf("Echec de la reception du numéro client\n");
    } else {
        printf("Votre numero de Client est : %s \n ",numClient);
    }

    // Attente de l'accord du serveur pour commencer
    printf("Veuillez attendre la confirmation du seveur pour commencer ... \n");
    res = recevoirMessage(&dSClient, buffer);
    while( strcmp(buffer, "ok" ) != 0) {
        //On attend ...
        res = recevoirMessage(&dSClient, buffer);
    }
    printf("Vous pouvez commencer \n");

    // Debut de la boucle d'action read/write ...
    printf("Pour mettre fin à la connexion, tapez fin \n");
    while ( strcmp( buffer, "fin" ) != 0){
        // Cas du client 1
        if( strcmp(numClient, "1" ) == 0 )  {
            printf("Quel message voulez-vous envoyer au client 2 ? \n");
            ecrireMessage(buffer);
            printf("Votre message est : %s  \n",buffer);
            res = 0;
            res = envoyerMessage(&dSClient, buffer);
            if (res < 0) {
                printf("Echec de l'envoir du message au client 2");
            } else {
                printf("Le message a bien été envoyé \n");
            }

            printf("En attente du message du client 2 ... \n");
            res =0;
            res = recevoirMessage(&dSClient, buffer);
            if (res < 0) {
                printf("Echec lord de la reception du message du client 2\n");
            } else {
                printf("Message du client 2 %s \n", buffer);
            }
        }

        // Cas du client 2
        if( strcmp(numClient, "2" ) == 0 )  {
            res =0;
            printf("En attente du message du client 1 ... \n");
            res = recevoirMessage(&dSClient, buffer);
            if (res < 0) {
                printf("Echec lord de la reception du message du client 2\n");
            } else {
                printf("Message du client 1 %s \n", buffer);
            }


            printf("Quel message voulez-vous envoyer au client 1 ? \n");
            res = 0;
            ecrireMessage(buffer);
            printf("Votre message est : %s  \n",buffer);
            res = 0;
            res = envoyerMessage(&dSClient, buffer);
            if (res < 0) {
                printf("Echec de l'envoir du message au client 1");
            } else {
                printf("Le message a bien été envoyé \n");
            }
        }

        printf("\n\n");
    }

    closesocket(dSClient);
    return 0;

}
