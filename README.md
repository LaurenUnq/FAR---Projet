# FAR---Projet

## Itération 1

##### Groupe :
##### - Unquera Lauren
##### - Saimond Etienne

But : Créer une messagerie instantanée codée en C et basée sur le protocole Socket
Itération 1 :
```
Un serveur relaie des messages textuels entre deux clients (livrable 1)
Il doit y avoir 1 programme serveur et 1 programme client. Ce dernier devant être lancé deux fois (deux processus distincts).
1 seul processus/thread serveur doit gérer les 2 clients,
qui envoient leurs messages à tour de rôle (client 1 : write puis read, et client 2 : read puis write)
L’échange de messages s’arrête lorsque l’un des clients envoie le message « fin ».
Ceci n’arrête pas le serveur, qui peut attendre la connexion d’autres clients.

Semaines du 1er et du 8 avril

1 séance encadrée : 1h30 le mercredi 03 avril
```

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


Déroulement :
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

Arrêt du programme client :
1. Si un client envoie "fin" au serveur, il ferme ses port et le programme se termine.
2. Le serveur va lui fermer le socket du client concerné, et attendre la connexion d'un autre client.
3. Un nouveau client se connecte.
4. Le serveur envoie au nouveau client le numéro du client précedemment deconnecté.
5. Le serveur envoie au nouveau client "emi" (Mode reception).
6. Le serveur envoie aussi au nouveau client le dernier message envoyé au client precedemment deconnecté.
7. Le client va alors entrer dans son fonctionnement normal.

Fermeture du serveur :
1. Si le serveur est fermé via CTRL+C, il envoie aux deux client "exit" et ferme ses socket.
2. Les deux client à la reception du message "exit" vont fermer leurs socket


Sources :
- [Guide pour la programmation réseaux de Beej's](http://vidalc.chez.com/lf/socket.html)
- [Les sockets en C de developpez](https://broux.developpez.com/articles/c/sockets/#L3-2-1-c)
