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
#define nb_client_serveur 10
#define nb_salon 5
#define nb_client_salon 2


struct Client {
	int client_numero;
	char client_pseudo [TAILLE_BUFFER];
	//0 si le client est connecté, 1 sinon
	int client_connecte;
	int client_numeroSalon;
	int dSC;
	//struct sockaddr_in aC;
};

struct Salon {
	int salon_numero;
	int salon_nb_max_client;//nb_client_salon
	int salon_nb_actuel_client;
	char salon_nom [TAILLE_BUFFER];
	char description [TAILLE_BUFFER];
	struct Client salon_tabClient[nb_client_salon];
};

int dS;
//int dSC[nb_client_serveur];
struct sockaddr_in aC[10];
socklen_t lg = sizeof(struct sockaddr_in);

pthread_t tabThread[nb_client_serveur];
struct Salon tabSalon[nb_salon];

int threadActu=-1;
void closeAllPort();
void init_socket();
struct sockaddr_in init_server(int port);
void bind_server(struct sockaddr_in ad);
void listen_server(int nbCo);
struct Client attendreConnexion(struct Client client) ;
void gestionDecoClient(struct Client client);
void recevoirMessage(struct Client client,char *bufferReception);
void envoyerMessage(struct Client client, char *buffer);
void initialisationSalon (struct Salon salon);
void envoyerMessageSalon(struct Client client,char *buffer);

static void* transmettre(void * args);
void traiterDemande(struct Client client,char *buffer);
void transfererFichier(struct Client client);

void delay(int number_of_seconds);

int main () {
    printf("Démarrage du serveur \n");
    int i;

    //initialisation des salon
    for(i = 0; i < nb_salon; i++){
        tabSalon[i].salon_numero=i;
        tabSalon[i].salon_nb_max_client = nb_client_salon;
        //for (int y =0; y < nb_client_serveur; y++){
    	//}
    }

    init_socket();
    // Si l'utilisateur appuie sur CTRL+C, fermeture du port
    signal(SIGINT, closeAllPort);

    // définition des paramètres du serveur
    struct sockaddr_in ad = init_server(44575);

    bind_server(ad);
    listen_server(nb_client_serveur);
    printf("En attente de client \n");
    //Connexion des clients et envoie du numéro
    char numero_choisi[TAILLE_BUFFER];
    int conversion_num_choisi;
    int salon_choisi;
    for (i = 0; i < nb_client_serveur; i++) {
        // Initialisation du client
        salon_choisi = 0;
        struct Client client;
        client.client_numero = i;
        //printf(" num %d \n",client.client_numero);
        client = attendreConnexion(client);
        client.client_connecte = 1;
        threadActu = client.client_numero;
        envoyerMessage(client,"choisissez votre salon, voici la liste");
        char numeroSalonChar[3];
        for (int y = 0; y < nb_salon; y++){
        	sprintf(numeroSalonChar, "%d", tabSalon[y].salon_numero);
            envoyerMessage(client,&numeroSalonChar);
            delay(30);

        }
        envoyerMessage(client, "endSalon \n");
        while (salon_choisi == 0){
            recevoirMessage(client, numero_choisi);
            printf("numero choisi : %s \n", numero_choisi);
            conversion_num_choisi = atoi (numero_choisi);
printf("Serveur nb actuel : %d  /// num_choisi %d \n",tabSalon[conversion_num_choisi].salon_nb_actuel_client, conversion_num_choisi);
            if (tabSalon[conversion_num_choisi].salon_nb_actuel_client>tabSalon[conversion_num_choisi].salon_nb_max_client) {
		printf("salon plein \n");
            }
            else {
                envoyerMessage(client, "OkSalon");
                salon_choisi = 1;
            }

        }
        client.client_numeroSalon = conversion_num_choisi;
        client.client_numero = i;
        printf("%d i",i);
        tabSalon[conversion_num_choisi].salon_tabClient[tabSalon[conversion_num_choisi].salon_nb_actuel_client] = client;
        tabSalon[conversion_num_choisi].salon_nb_actuel_client ++;
        printf("pseudo_client : %s / numero client : %d , numéro_salon_cli : %d \n", client.client_pseudo, client.client_numero, client.client_numeroSalon);
        printf("salon_nb_actuel n°%d : %d \n",tabSalon[conversion_num_choisi].salon_numero, tabSalon[conversion_num_choisi].salon_nb_actuel_client);
        pthread_create((pthread_t*)&tabThread[i], NULL, transmettre,(void *) &client);

    }

    //for (i = 0; i < nb_client_serveur; i++) {
       pthread_join((pthread_t)tabThread[threadActu], NULL);
    //}



    while (1){

    }
    closeAllPort();

    printf("Fermeture du serveur \n");

    return 0;
}

void initialisationSalon (struct Salon salon) {
	salon.salon_nb_max_client = nb_client_salon;
	salon.salon_nb_actuel_client = 0;
	//salon.salon_nom = "None";
	//salon.description = "None";
}

/*
*func transmettre : int ->
*Transmet le message du client donné en parametre à tous les autres clients
*/
static void* transmettre(void *args){
    char buffer[TAILLE_BUFFER];
    //Client qui envoie message - il faut receptionner son message
		struct Client client;
		client.client_numero = ((struct Client *) args) -> client_numero-1 ;
		strcpy(client.client_pseudo, ((struct Client *) args) -> client_pseudo);
		//client.client_pseudo  = ((struct Client *) args) -> client_pseudo;
		client.client_connecte = ((struct Client *) args) -> client_connecte;
	  client.client_numeroSalon = ((struct Client *) args) -> client_numeroSalon;
		client.dSC = ((struct Client *) args) -> dSC;

    printf("le client numero %d / pseudo : %s / salon %d \n", client.client_numero, client.client_pseudo, client.client_numeroSalon);
    while (1) {
        //printf("Attente d'un message ... \n ");
        recevoirMessage(client, buffer);
        traiterDemande(client, buffer);
    }
}


/*
* func recevoirMessage : int, char[], char[]->
* Recoit le message d'un client à partir du numéro du client
* bufferReception est un tableau de charactère pour recevoir le message
* Attend la connexion d'un nouveau client si le contenue de la chaine est "fin "
*/

void recevoirMessage(struct Client client,char *bufferReception) {
    // Reception du message venant du client
    //printf("dSC %d",client.dSC);
    int res = recv(client.dSC, bufferReception, TAILLE_BUFFER - 1, 0);
    //printf("res : %d \n",res);
    if (client.client_numero < 0 || client.client_numero > nb_client_serveur)
    {
        perror("Mauvais numéro client");
        exit(1);
    }

    // Vérification du retour de la fonction
    if (res == -1) {
        //perror("recevoirMessage");
        //gestionDecoClient(client);
    } else if (res == 0) {
        //perror("Client deconnecté");
        //gestionDecoClient(client);
    } else {
        // Remplace le dernier charactère par '\0' pour éviter toute erreur
        // lié a une chaine trop longue ou mal formattée
        bufferReception[res] = '\0';
    }
}


/*
 * func envoyerMessage : Client, char[] ->
 * Envoie a partir d'un socket, une chaine de charactère un message
 */
void envoyerMessage(struct Client client,char *bufferEnvoie) {
    if (strlen(bufferEnvoie) > TAILLE_BUFFER) {
        perror("Message trop long");
        closeAllPort();
        exit(1);
    }
    int res;
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    // Envoie du message au client passé en paramètre s'il est connecté
    if (client.client_numero < 0 || client.client_numero > nb_client_serveur)
    {
        perror("Mauvais numéro client");
        exit(1);
    } else {
            res = send(client.dSC, bufferEnvoie, strlen(bufferEnvoie)+1, 0);
            // Vérification du retour de la fonction
            if (res == -1) {
                perror("envoyerMessage");
                gestionDecoClient(client);
            } else if (res == 0) {
                perror("Client deconnecté");
                gestionDecoClient(client);
            } else {
                printf("Message envoyé au client %d : %s\n", client.client_numero, bufferEnvoie);
            }

    }
}

/*
 * func envoyerMessage : int, char[] ->
 * Envoie a partir d'un numéro de client un message à tout les client connectés incluant le pseudo du client
 */
void envoyerMessageSalon(struct Client client,char *buffer) {
    int numero_Salon = client.client_numeroSalon;
    printf("num salon emsgsalon %d \n",numero_Salon);
    //struct Salon salon_client = tabSalon[numero_Salon];
/*
    for (int compteur =0; compteur < nb_salon; compteur ++) {
    	if (tabSalon[compteur].salon_numero == client.client_numeroSalon){
            printf("1 : %d 2 : %d \n",tabSalon[compteur].salon_numero, client.client_numeroSalon);
    		salon_client = tabSalon[compteur];
            printf("envoyerms num salon %d \n",salon_client.salon_numero);
		//printf("salon_numero : %d / client sal_nume %d", tabSalon[compteur].salon_numero, client.client_numeroSalon);
    	}
    }*/
    for (int z = 0; z < tabSalon[numero_Salon].salon_nb_max_client; z++) {
        if (z != client.client_numero) {

		    if (strlen(buffer) > TAILLE_BUFFER) {
		        perror("Message trop long");
		        closeAllPort();
		        exit(1);
		    }
		    int res;
		    // + d'infos sur send()  : https://man.developpez.com/man2/send/
		    // Envoie du message au client du salon s'il est connecté
		    if (tabSalon[numero_Salon].salon_tabClient[z].client_connecte == 1) {
		        char msg[TAILLE_BUFFER] = "message de ";
		        strcat(msg, client.client_pseudo);
		        strcat(msg, " : ");
		        strcat(msg, buffer);
		        res = send(tabSalon[numero_Salon].salon_tabClient[z].dSC, msg, strlen(msg), 0);

		        // Vérification du retour de la fonction
		        if (res < 0) {
		            perror("envoyerMessage");
		            exit(1);
		        }
		        else {
		            printf("Message envoyé au client %d : %s\n", tabSalon[numero_Salon].salon_tabClient[z].client_numero, buffer);
		        }
		    }
		    //if (salon_client.salon_tabClient[z].client_numero < 0 || salon_client.salon_tabClient[z].client_numero > NBCLIENT)
		    //{
		    //    perror("Mauvais numéro client");
		    //    exit(1);
		    // }
        }
    }
}

/*
* func traiterDemande : struct Client, char[] ->
* Traite la demande du client a partir de son numero de client et du message reçu de ce dernier
*/
void traiterDemande(struct Client client, char *buffer) {
    if (strcmp(buffer, "/fin") == 0) {
        gestionDecoClient(client);
    } else if (strcmp(buffer, "/file") == 0) {
        transfererFichier(client);
    } else if (
    (strcmp( buffer, "/exit") == 0)
    || (strcmp( buffer, "/BOF") == 0)
    ) {
        printf("%s joue au hacker\n", &client.client_pseudo[0]);
        // Envoie de exit avec un espace, sinon le client va se deconnecter a la reception du message
        sprintf(buffer, "%s ", buffer);
        envoyerMessage(client, "NON!");
    } else if (strcmp( buffer, "") == 0) {
        //printf("Message Vide recu de %s\n", &client.client_pseudo[0]);
    } else {
        printf("Message recu de %s : %s\n", &client.client_pseudo[0], buffer);
        envoyerMessageSalon(client, buffer);
    }
}

/*
* func transfererFichier : struct Client ->
* transfère au client un fichier
*/
void transfererFichier(struct Client client) {
    printf("Début du transfère du fichier de %s vers les autres clients\n", &client.client_pseudo[0]);
    // BOF = Beginning of file
    char buffer[TAILLE_BUFFER];
    envoyerMessageSalon(client, "/BOF");
    // Transfère du nom du fichier
    recevoirMessage(client, buffer);
    envoyerMessageSalon(client, buffer);
    // Tant que le fichier n'est pas reçu
    while (strcmp(buffer, "/EOF") != 0) {
        recevoirMessage(client, buffer);
        envoyerMessageSalon(client, buffer);
    }
    // La derniere chaine envoyée au client est /EOF (End of File)
}


/*
* func gestionDecoClient : Client ->
* Gère la deconnexion d'un client
* Attend la connexion d'un nouveau client
* Envoie au nouveau client le dernier message envoyé
*/
void gestionDecoClient(struct Client client) {
    printf("Client %d deconnecté, en attente d'un nouveau client\n", client.client_numero);
    client.client_connecte = 0;
    attendreConnexion(client);
}

/*
* func attendreConnexion : Client -> Client
* Attend une connexion provenant d'un client
*/
struct Client attendreConnexion(struct Client client) {
    int cli = accept(dS, (struct sockaddr*)&aC[client.client_numero],&lg);
    printf (" cli : %d ",cli);
    printf("numclient %d",client.client_numero);
    if(cli == -1) {
        perror("attendreConnexion");
        exit(1);
    }
    else {
        printf("Socket client %d ouvert\n", client.client_numero);
        client.client_connecte = 1;
        client.dSC = cli;
        char num[3];
        sprintf(num, "%d", client.client_numero);
        printf("Connexion du client %d \n", client.client_numero);
        // Envoie au client de son numéro
        envoyerMessage(client, num);
        // Attente du pseudo choisi par le client
        recevoirMessage(client, client.client_pseudo);
        // Tant que le pseudo est trop long en demander un nouveau
        while (strlen(client.client_pseudo) > 32) {
            envoyerMessage(client, "Too Long");
            recevoirMessage(client, client.client_pseudo);
        }
        printf("Pseudo du client : %s \n", client.client_pseudo);
        // Envoie du signal de départ au client
        envoyerMessage(client, "start");

    }
    return client;

}

/*
* func closeAllPort : ->
* Fonction appelée si l'utilisateur appuie sur CTRL+C
* Envoie aux deux client "exit" et ferme tout les ports
*/
void closeAllPort() {
    int i;
    for (i = 0; i < nb_salon; i++) {
        pthread_cancel(tabThread[i]);
        for (int y = 0; y< tabSalon[i].salon_nb_actuel_client;y++){
        	tabSalon[i].salon_tabClient[y].client_connecte = 0;
    		envoyerMessage(tabSalon[i].salon_tabClient[y], "exit");
    		close(tabSalon[i].salon_tabClient[y].dSC);
        }
    }
    close(dS);
    printf("\n bye !\n");
    exit(0);
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

// source : https://www.geeksforgeeks.org/time-delay-c/
void delay(int number_of_seconds) {
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}
