#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
//ici on fait entrer des message à l utilisateur jusqu'à ce qu'il
//decide d'exit

//on met l adresse du serveur 
//- pour le tp, comme on crée un serveur, on met l'adresse de notre machine
//on lance ensuite le serveur, puis on lance le client et ça fonctionne 

int main () {
    //Definition du protole
    int dS=socket(PF_INET,SOCK_STREAM,0);

    //Connexion à un serveur
    struct sockaddr_in adServ;
    int res; 
    adServ.sin_family = AF_INET;
    adServ.sin_port = htons(44573); //on rentre le port du serv
    //Attention, le dernier chiffre de l'adresse correspond à celui 
    //de la machine sur laquelle on est !!    
    //à changer si on change de machine
    res = inet_pton(AF_INET,"162.38.111.95",&(adServ.sin_addr));

    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(dS, (struct sockaddr *) &adServ, lgA);


    char msg[1024]; // ce que l'utilisateur va entrer 32 caracteres sans le caract. de fin de chaine
    char depart;
    char NumClient; // Client 1 ou client 2
    
    //Envoi d'un message
    if (res != -1){
        printf("La connexion a été établie \n");
    }
    else {
        printf("La connexion n'a pas fonctionnée \n");
    }

    //Obtenir le numero du client
    recv(dS, NumClient, 1024,0);
    printf("Votre numero de Client est : %s \n ",NumClient);

    //attente de l'accord du serveur pour commencer
    printf("Veuillez attendre la confirmation du seveur pour commencer ... \n");
    recv(dS, depart, 1024,0);
    while( strcmp(depart, "ok" ) != 0) {
        //On attend ...
        recv(dS, depart, 1024,0);
    }
    printf("Vous pouvez commencer \n");

    //Debut de la boucle d'action read/write ...
    printf("Pour mettre fin à la connexion, tapez fin \n");
    while ( strcmp( msg, "fin" ) != 0){

        //Cas du client 1
        if( strcmp(NumClient, "1" )==0 )  {
            printf("Quel message voulez-vous envoyer au serveur ? \n");
            scanf("%s",msg);
            printf("Votre message est : %s  \n",msg);
            send(dS, msg, 1024, 0); //Le 1024 est la taille possible du message
            printf("Le message %s a bien été envoyé \n",msg);

            printf("Vous allez maintenant lire ... \n");
            recv(dS, msg, 1024,0);
            printf("La réponse du serveur est : %s \n", msg);

        }

        //Cas du client 2
        if( strcmp(NumClient, "2" )==0 )  {
            printf("Vous allez maintenant lire ... \n");
            recv(dS, msg, 1024,0);
            printf("La réponse du serveur est : %s \n", msg);

            printf("Quel message voulez-vous envoyer au serveur ? \n");
            scanf("%s",msg);
            printf("Votre message est : %s  \n",msg);
            send(dS, msg, 1024, 0); //Le 1024 est la taille possible du message
            printf("Le message %s a bien été envoyé \n",msg);
        }

        printf("\n");
    }
    
    return 0;
    
}
