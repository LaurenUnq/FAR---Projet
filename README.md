# FAR---Projet

## Itération 1

##### Groupe :
##### - Unquera Lauren
##### - Saimond Etienne

But : Créer une messagerie instantanée codée en C et basée sur le protocole Socket
Itération 3 : transfert de fichiers (livrable 3) :
```
A tout moment, un client peut décider d’envoyer un fichier, il saisit le mot « file ».
Le client liste les fichiers d’un répertoire (dédié à l’application et dans lequel on doit ranger au
préalable les fichiers pouvant être transférés) et demande à l’utilisateur de choisir, dans cette liste,
un fichier à envoyer. Ceci est géré par un thread dédié.
Comme ça, le client peut continuer à échanger des messages textuels.
Un fichier reçu par le client est stocké dans un deuxième répertoire (dédié aux fichiers téléchargés). Un thread est dédié à la réception de fichiers.
v1 : Le serveur reçoit le contenu du fichier et le transmet à l’autre client (sur Moodle, un exemple
de gestion de fichiers dans le langage C).
Alternative (plus optimale, mais plus complexe que v1) : le serveur met juste en relation les clients
pour le transfert des fichiers (il ne reçoit pas le contenu des fichiers et le relaie).
Il transmet à chaque client la structure (sockaddr_in) qui contient l’adresse IP et le numéro de port de l’autre client (le serveur devient un annuaire dans ce cas). 
Le client qui transmet le fichier devient un serveur pour l’autre client (l’application est un système pair-à-pair).

Voilà un extrait de code qui
montre comment récupérer cette structure côté serveur (si ce n’est pas déjà fait dans votre code) :

struct sockaddr_in adCv ;
socklen_t lgCv = sizeof (struct sockaddr_in);
...
int connfd = accept(listenfd, (struct sockaddr*)&adCv, &lgCv);
puts("Demande de connexion d’un client acceptée \n");
printf("L’adresse IP du client est : %s\n", inet_ntoa(adCv.sin_addr));
printf("Son numéro de port est : %d\n", (int) ntohs(adCv.sin_port));

v2 : mettre en place un thread par fichier à transférer (et un thread par fichier à recevoir). De cette
manière, si des fichiers volumineux sont transférés, on peut avoir plusieurs transferts de fichiers en
parallèle.

Semaines du 22 et du 29 avril (la semaine du 29 étant une semaine de vacances). 2 séances
encadrées : 3h pour chaque groupe

Livrable à rendre le dimanche 5 mai au soir.
```

## Comment se servir de l'application
Pour compiler les fichier :
```bash
make
```

Pour lancer le serveur :
```bash
bash launch_server.sh
```

Pour compiler les fichier :
```bash
bash launch_client.sh
```


## Déroulement :
##### Echange entre le serveur et le client
1. Le Serveur attend une connexion.
2. Le Client 1 se connecte.
3. Le serveur envoie au Client 1 son numéro.
4. Le Client 1 attend de recevoir son role du serveur.
5. Le Serveur attend une autre connexion.
6. Le Client 2 se connecte.
7. Le serveur envoie au Client 2 son numéro.
8. Le Client 2 attend de recevoir son role du serveur.
9. Le Serveur envoie au Client 1 "emi" (Mode émission).
10. Le Serveur envoie au Client 2 "rec" (Mode réception).
* Debut de boucle
11. Le client 1 envoie son message au serveur et se met en mode reception.
12. Le Serveur le recoit et le retransmet au Client 2.
13. Le client 2 recoit le message du client 1.
14. Le client 2 envoie son message au serveur et se met en mode reception.
15. Le Serveur le recoit et le retransmet au Client 1.
16. Le client 1 recoit le message du client 2.
* Fin de Boucle

##### Arrêt du programme client :
1. Si un client envoie "fin" au serveur, il ferme ses port et le programme se termine.
2. Le serveur va lui fermer le socket du client concerné, et attendre la connexion d'un autre client.
3. Un nouveau client se connecte.
4. Le serveur envoie au nouveau client le numéro du client précedemment deconnecté.
5. Le serveur envoie au nouveau client "rec" (Mode reception).
6. Le serveur envoie aussi au nouveau client le dernier message envoyé au client precedemment deconnecté.
7. Le client va alors entrer dans son fonctionnement normal.

##### Fermeture du serveur :
1. Si le serveur est fermé via CTRL+C, il envoie aux deux client "exit" et ferme ses socket.
2. Les deux client à la reception du message "exit" vont fermer leurs socket


## Sources :
- [Guide pour la programmation réseaux de Beej's](http://vidalc.chez.com/lf/socket.html)
- [Les sockets en C de developpez](https://broux.developpez.com/articles/c/sockets/)
