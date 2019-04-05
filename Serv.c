#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

//on lance le serveur
//si on ne lance pas de client, ça restera bloqué à l'étape 2
//sinon, on lance un client et ça marche, on reçoit bien les message que ce soit
//dans le terminal client ou dans le terminal serveur


int main () {
    //Definition du protole
    int dS=socket(PF_INET,SOCK_STREAM,0);

    printf("démarrage du serveur \n");

    //Connexion à un serveur
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(44573); //on rentre le port du serveur

    struct sockaddr_in aC1;
    struct sockaddr_in aC2;
    socklen_t lg = sizeof(struct sockaddr_in);

    //déclaration de variables annexes
    int booleen1 = 1;
    int nbConnexion = 2;
    char msg[124];

    int dSC1;
    int dSC2; 
    bind(dS,(struct sockaddr*)&ad, sizeof(ad)); 
    listen(dS,nbConnexion);    

    //printf("etape 1 while \n");
   /* if(getsockname(dS, (struct sockaddr*) &aC1, &lg) < 0) {
        perror("serveur : erreur : getsockname");
        close (dS);
        exit(1);
    }
    if(getsockname(dS, (struct sockaddr*) &aC2, &lg) < 0) {
        perror("serveur : erreur : getsockname");
        close (dS);
        exit(1);
    }
*/
    char *repServ;

    //Connexion du client 1 et envoie du numéro
    printf("En attente du client 1\n");
    dSC1 = accept(dS, (struct sockaddr*)&aC1,&lg);
    printf("Socket client 1 ouvert\n");
    repServ = "1";
    send(dSC1, repServ, 3, 0);
    printf("Numéro envoyé au client 1 : %s \n", repServ);


    //Connexion du client 1 et envoie du numéro
    printf("En attente du client 2\n");
    dSC2 = accept(dS, (struct sockaddr*)&aC2,&lg);
    printf("Socket client 2 ouvert\n");
    repServ = "2";
    send(dSC2, repServ, 3, 0);
    printf("Numéro envoyé au client 2 : %s \n", repServ);

    //Envoi "ok" aux clients
    repServ = "ok";
    printf("Envoi de %s aux clients \n", repServ);
    send(dSC1, repServ, 3, 0);
    send(dSC2, repServ, 3, 0);

    int res;
    //printf("étape 3 \n")
    while (1){
        printf("Attente du message du client 1 (pour le Client 2)... \n");
        res = recv(dSC1, msg , sizeof(msg), 0);
        printf("Message du client 1 : %s \n", msg);
        send(dSC2, msg, sizeof(msg), 0);

        printf("Attente du message du client 2 (pour le Client 1)... \n");
        res = recv(dSC2, msg , sizeof(msg), 0);
        printf("Message du client 2 : %s \n", msg);
        send(dSC1, msg, sizeof(int), 0);
        //printf("res de taille %d \n",res);


    }


    close(dSC1);
    close(dSC2);
    close(dS);

    printf("Fermeture du serveur \n");
 
    return 0;

    
}
