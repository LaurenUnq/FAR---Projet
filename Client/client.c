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
#include <limits.h>
#include <dirent.h>

// Taille du buffer
#define TAILLE_BUFFER 4096


// Port(s) utilisé par le socket
int dS;

// threads
pthread_t thread_ecrire;
pthread_t thread_lire;

// Pseudo du client
char pseudo[TAILLE_BUFFER];

void envoyerMessage(int dSClient,char *bufferEnvoie);
void closeAllPort();
void recevoirMessage(int dSClient,char *bufferReception);
void ecrireMessage(char *msg);
void deconnexion();
void traiterCommande(int dSClient,char *buffer);
void envoyerFichier();
void recevoirFichier(int dSClient,char *buffer);
static void * ecrire();
static void * lire();
// External functions
int get_last_tty();
FILE* new_tty();

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


    char buffer[4096];  // ce que l'utilisateur va entrer
    char NumClient[3];  // Client 1 ou client 2

    // Obtenir le numero du client
    recv(dS, NumClient, 3,0);
    printf("Votre numero de Client est : %s \n",NumClient);

    do {
        printf("Donnez votre pseudo (32 char max)\n");
        ecrireMessage(pseudo);
        envoyerMessage(dS, pseudo);
        recevoirMessage(dS, buffer);
    } while ((strcmp(buffer, "Too Long") == 0));
    // Attente de l'accord du serveur pour commencer
    printf("Veuillez attendre la confirmation du seveur pour commencer ... \n");
    if (strcmp(buffer, "start" ) != 0) {
        perror("Bad start message");
        closeAllPort();
    } else {
        printf("Vous pouvez commencer \n");
    }

    printf("\n\n");
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
 * func envoyerMessage : Socket, char[] ->
 * Envoie a partir d'un socket, une chaine de charactère un message
 * Renvoie 0 si aucune Erreur
 * Arrete le programme et donne l'erreur si erreur
 * Renvoie -2 si chaine trop longue
 */
void envoyerMessage(int dSClient,char *bufferEnvoie) {
    int res = send(dSClient, bufferEnvoie, strlen(bufferEnvoie), 0);
    // + d'infos sur send()  : https://man.developpez.com/man2/send/
    if (res == -1) {
        perror("envoyerMessage");
        closeAllPort();
    } else if (res == 0){
        perror("Connexion perdu");
        closeAllPort();
    } else {
        printf("Message envoyé : %s\n", bufferEnvoie);
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
    printf("Deconnexion au serveur\n");
    envoyerMessage(dS, "/fin");
    closeAllPort();
}


/*
* func recevoirMessage : Socket, char[], int ->
* Envoie a partir d'un socket, un tableau de charactère un message
* Renvoie 0 si aucune Erreur
* Renvoie -1 si erreur
* Renvoie -2 si chaine trop longue
* Arrete le programme via la fonction closeAllPort si le contenue de la chaine est "exit"
*/
void recevoirMessage(int dSClient,char *bufferReception) {
    int res = recv(dSClient, bufferReception, TAILLE_BUFFER - 1, 0);
    if (res == -1) {
        perror("recevoirMessage");
        closeAllPort();
    } else if (res == 0){
        perror("Connexion perdu");
        closeAllPort();
    } else {
        bufferReception[res] = '\0';
    }
}

/*
* func traiterCommande : Socket ->
* Lance la procédure d'envoie d'un fichier
*/
void envoyerFichier() {
    FILE* fp1 = new_tty();
    fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

    // Demander à l'utilisateur quel fichier afficher
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("./Upload");
    if (dp != NULL) {
        fprintf(fp1,"Voilà la liste de fichiers :\n");
        while (ep = readdir (dp)) {
        if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0)
  	        fprintf(fp1,"%s\n",ep->d_name);
        }
        (void) closedir (dp);
    }
    else {
        perror ("Ne peux pas ouvrir le répertoire");
    }
    fprintf(fp1, "%s\n", "Indiquer le nom du fichier : ");
    char fileName[1023];
    char fullPath[1023];
    // L'utilisateur écrit le nom du fichier
    ecrireMessage(fileName);
    sprintf(fullPath, "./Upload/%s", fileName);
    // Ouverture du fichier a envoyer
    FILE *fps = fopen(fullPath, "r");
    if (fps == NULL){
        fprintf(fp1, "%s%s\n", "Ne peux pas ouvrir le fichier suivant :",fileName);
    }
    else {
        char str[128];
        printf("Début du transfère du fichier\n");
        // Envoie de la commande de début de transfère
        sprintf(str, "/file");
        envoyerMessage(dS, str);
        // Envoie du nom du fichier
        envoyerMessage(dS, fileName);
        // Lire et afficher le contenu du fichier
        while (fgets(str, 127, fps) != NULL) {
            str[strlen(str)] = '\0';
            envoyerMessage(dS, str);
        }
    }
    fclose(fps);
    fclose(fp1);
    // Envoyer End Of File
    envoyerMessage(dS, "/EOF");
    printf("Fin du transfère du fichier");
}

/*
* func recevoirFichier : ->
* Recevoir un fichier provenant d'un autre client
*/
void recevoirFichier(int dSClient,char *buffer) {
    printf("Reception d'un fichier\n");
    // Reception du nom du fichier
    recevoirMessage(dS, buffer);
    char filePath[75];
    sprintf(filePath, "./Download/%s", buffer);
    printf("Lien du fichier à recevoir : %s \n\n", filePath);
    // Création du fichier portant le nom du fichiers
    FILE* fichier = fopen(filePath, "w");
    if (fichier != NULL) {
        printf("Début de l'écriture dans le fichier\n");
        int endOfFile = 1;
        while (endOfFile != 0) {
            recevoirMessage(dSClient, buffer);
            endOfFile = strcmp(buffer, "/EOF");
            if (endOfFile != 0) {
                fprintf(fichier, "%s", buffer);
            } else {
                fclose(fichier);
            }
        }
        printf("Fin de l'écriture dans le fichier");
    } else {
        printf("Echec lors de l'ouverture du fichier");
    }
    // Fermer le fichier
}

/*
* func traiterCommande : Socket, char[] ->
* Traite la chaine de charactère ecrite par l'utilisateur
* afin de determiner l'action à réaliser
*/
void traiterCommande(int dSClient,char *buffer) {
    if (strcmp(buffer, "/fin") == 0) {
        // Le client veut mettre fin à la connexion
        deconnexion();
    } else if (strcmp(buffer, "/help") == 0) {
        // Le client veut afficher toute les commandes disponibles
        printf("/fin\t\t:\t\tMettre fin à la connexion avec le serveur\n");
        printf("/file\t\t:\t\tEnvoyer un fichier à l'autre client\n");
    } else if (strcmp(buffer, "/file") == 0) {
        // Le client souhaite envoyer un message aux autre client
        // TODO Faire un thread
        //pthread_t thread_fichier;
        //pthread_create (&thread_fichier, NULL, envoyerFichier, NULL);
        envoyerFichier();
    } else {
        envoyerMessage(dSClient, buffer);
    }
}

/*
* func traiterCommande : Socket, char[] ->
* Traite la chaine de charactère recu
* afin de determiner l'action à réaliser
*/
void traiterDemande(int dSClient,char *buffer) {
    if(strcmp(buffer, "/BOF") == 0) {
        recevoirFichier(dSClient, buffer);
    } else if (strcmp( buffer, "exit") == 0) {
        printf("EXIT !!");
        closeAllPort();
    } else {
        printf("%s \n", buffer);
    }
}

/*
* func lire* : ->
* Recoid les messages venant des autres clients
*/
static void * lire() {
    char buffer[4096];
    while(1) {
        recevoirMessage(dS, buffer);
        traiterDemande(dS, buffer);
        printf("\n");
    }
}

/*
* func ecrire* : ->
* Recoit les messages/commandes du client et les traite
*/
static void * ecrire()  {
    char buffer[4096];
    printf("Ecrivez vos message\n");
    // TODO : /help
    printf("/help pour une liste des commandes ... \n");

    while (1) {
        ecrireMessage(buffer);
        traiterCommande(dS, buffer);
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

// External Functions


int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("Impossible d'exécuter la commande\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }

  pclose(fp);
  return i;
}


FILE* new_tty() {
  pthread_mutex_t the_mutex;
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal");
  sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  int i;
  for(i = strlen(tty_name)-1; i >= 0; i--) {
    if(tty_name[i] == '/') break;
  }
  tty_name[i+1] = '\0';
  strcat(tty_name,str);
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}
