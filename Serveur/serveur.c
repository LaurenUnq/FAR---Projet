# include <stdio.h>
# include <sys/types.h>
# include <netdb.h> /* gethostbyname */
# include <arpa/inet.h>
# include <sys/socket.h>
# include <unistd.h> /* close */
# include <stdlib.h>
# include <string.h>
#include <time.h>

// Alias pour plus de clarté
#define INVALID_SOCKET -1
#define closesocket(s) close(s)
typedef int SOCKET;

// Taille du buffer
#define TAILLE_BUFFER 1024

/*
 * func envoyerMessage : Socket, char[] -> int
 * Renvoie 0 si aucune Erreur
 * Renvoie -1 si erreur
 * Renvoie -2 si chaine trop longue
 */
int envoyerMessage(SOCKET *dSClient,char *buffer) {
    if (strlen(buffer) > 1024) {
        return -2;
    }
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    if (send(*dSClient, buffer, sizeof(buffer), 0) < 0) {
        return -1;
    } else {
        return 0;
    }
}

int recevoirMessage(SOCKET *dSClient,char *buffer) {
    // + D'info sur recv() : https://man.developpez.com/man2/recv/
    return recv(*dSClient, buffer, sizeof(buffer)-1, 0);
}

void delay(int i)    /*Pause l'application pour i seconds*/
{
    clock_t start,end;
    start=clock();
    while(((end=clock())-start)<=i*CLOCKS_PER_SEC);
}

void attendre(int i) {
    for (i=5; i > 0; i--) {
        printf("Début dans %d secondes\n", i);
        delay(1);
    }
}

int main (int argc, const char* argv[] ) {
    // Création d'un SOCKET
    /*
     *  Udp :
     *  SOCK DGRAM
     *  Tcp
     *  SOCK_STREAM
     *  + d'info : https://man.developpez.com/man2/socket/
     */
    SOCKET dSServer = socket(PF_INET, SOCK_STREAM, 0) ;
    // Vérification du bon déroulement de la création du socket
    if(dSServer == INVALID_SOCKET)
    {
        printf("erreur lors de la création du socket serveur\n");
    }
    // Création de la structure avec les paramètre du serveur
    /*
     * struct sockaddr in {
     *      sa family t sin family ;     // famille AF INET
     *      in port t sin port ;         // numéro de port au format réseau
     *      struct in addr sin addr ;    // structure d’adresse IP
     * };
     */
    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET ;
    adServ.sin_addr.s_addr = INADDR_ANY ;
    adServ.sin_port = htons(44573);

    // bind lie un socket avec une structure sockaddr.
    // + D'info sur bind() : https://man.developpez.com/man2/bind/
    bind(dSServer, (struct sockaddr*)&adServ, sizeof(adServ));

    // Définition du nombre maximum de connexion entrante
    // + D'info sur listen : https://man.developpez.com/man2/listen/
    int nbrConnexionMax = 2;
    int l = listen(dSServer, nbrConnexionMax);

    // Variable servant a la vérification des retour d'erreur
    int res;

    // Buffer servant à l'envoie et la reception de message
    char buffer[TAILLE_BUFFER] = "empty";
    // Init
    socklen_t lg = sizeof(struct sockaddr_in);
    struct sockaddr_in adrClient1;
    printf("En attente du client 1\n");
    SOCKET dSClient1 = accept(dSServer, (struct sockaddr *)&adrClient1, &lg);
    printf("Socket client 1 ouvert\n");
    printf("Envoie numéro au client 1\n");
    if (envoyerMessage(&dSClient1, "1") < 0) {
        printf("Erreur fatale client1\n");
    }
    // Attente du client 2
    struct sockaddr_in adrClient2;
    printf("En attente du client 2\n");
    SOCKET dSClient2 = accept(dSServer, (struct sockaddr *)&adrClient2, &lg);
    printf("Socket client 2 ouvert\n");
    printf("Envoie numéro au client 2\n");
    if (envoyerMessage(&dSClient2, "2") < 0) {
        printf("Erreur fatale client2\n");
    }
    // Attend 5 seconde avant le début
    attendre(5);

    // Envoie Ok aux deux clients
    if (envoyerMessage(&dSClient1, "ok") < 0) {
        printf("Erreur fatale client1\n");
    }

    if (envoyerMessage(&dSClient2, "ok") < 0) {
        printf("Erreur fatale client2\n");
    }


    while(strcmp(buffer, "fin") != 0) {
        printf("En attente du message du client 1 pour le client 2");
        // Reception du message du client 1 pour le client 2
        res = recevoirMessage(&dSClient1, buffer);
        if (res < 0) {
            printf("Erreur lors de la reception du message du client 1\n");
            return -1;
        } else {
            printf("Message du client 1 : %s", buffer);
            res = envoyerMessage(&dSClient2, buffer);
            if (res < 0) {
                printf("Echec de l'envoie du message au client 2");
            } else {
                printf("Message envoyé au client 2");
            }
        }

        printf("En attente du message du client 2 pour le client 1");
        // Reception du message du client 1 pour le client 2
        res = recevoirMessage(&dSClient2, buffer);
        if (res < 0) {
            printf("Erreur lors de la reception du message du client 2\n");
            return -1;
        } else {
            printf("Message du client 2 : %s", buffer);
            res = envoyerMessage(&dSClient2, buffer);
            if (res < 0) {
                printf("Echec de l'envoie du message au client 1");
            } else {
                printf("Message envoyé au client 1");
            }
        }
    }
    printf("Fin du serveur\n");
    // + D'info close() : https://man.developpez.com/man2/close/
    closesocket(dSClient1);
    closesocket(dSClient2);
    printf("Socket clients fermés\n\n");
    closesocket(dSServer);
    //
    return 0;
}
