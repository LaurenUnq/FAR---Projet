# FAR---Projet
Groupe :
    - Unquera Lauren
    - Saimond Etienne

But : Coder une messagerie instantanée codé en C et basé sur le protocole Socket

Déroulement :
    * 1. Le Serveur attend un connexion
    * 2. Le Client 1 se connecte et attend
    * 3. Le Client 2 se connecte et attend
    * 4. Le Serveur envoie au deux Clients "ok"
    * 5. Le client 1 envoie son message au serveur
    * 6. Le Serveur le recoit et le retransmet au Client 2
    * 7. Le client 2 recoit le message du client 1 et peut alors écrire son message et l'envoyer au serveur
    * 8. Le Serveur le recoit et le retransmet au Client 1
    * 9. Et ainsi de suite jusqu'a qu'un des clients envoie "fin" au serveur
    * 10. Le serveur va alors envoyer aux deux client "stop", s'arreter et fermer ses ports.
    * 11. Les deux clients à la reception du message "stop" vont fermer leurs ports s'arreter.

    
Sources :
    - [Guide pour la programmation réseaux de Beej's](http://vidalc.chez.com/lf/socket.html)
