# Avancement S¢ntïnel v0.1-beta

Dernière mise à jour : 16 août 2026  
Branche de travail : `sentinel`  
État : **développement et audit logiciel terminés — validation matérielle requise avant release**

## Terminé

- identité `S¢ntïnel-v0.1-beta`, boot et animation SENTINEL ;
- interface automobile quatre couleurs et navigation plein écran ;
- backends CAN ESP32 TWAI et UART Lawicel/SLCAN ;
- dashboard, IDs actifs, capture ASC, replay et scénarios JSON ;
- ELM327 Bluetooth Classic et pont UART Proxmark3 ;
- arbitrage du port Grove entre CAN et Proxmark ;
- WebUI CAN et routes API authentifiées ;
- verrouillage CAN TX par défaut et confirmation physique ;
- double-clic sur le bouton haut pour arrêter une opération ;
- appui long sur le bouton haut pour annuler une saisie ;
- audit logiciel et corrections de sécurité prioritaires ;
- documentation matériel, sécurité et présentation GitHub.

## Build final disponible

| Élément | Valeur |
|---|---|
| Cible | `m5stack-cplus2` |
| Binaire | `S¢ntïnel-v0.1-beta-m5stack-cplus2.bin` |
| Taille | 4 414 960 octets |
| SHA-256 | `02a15cea1ce713238edd2c3b1ca2553b61ef9dfbc2482874f3d5f0315f653a84` |
| RAM globale | 34,7 % |
| Flash | 51,8 % |
| Flash matériel | réussi sur `/dev/ttyACM0` |

## Verrous avant release

- [ ] connecter la WebUI avec les identifiants valides ;
- [ ] confirmer qu’un ancien cookie `BRUCESESSION` ou `SENTINELSESSION` est refusé ;
- [ ] vérifier connexion, déconnexion et reconnexion WebUI ;
- [ ] connecter un adaptateur CAN sur un banc isolé ;
- [ ] confirmer le démarrage en écoute seule ;
- [ ] lancer et arrêter Dashboard/Capture avec le double-clic haut ;
- [ ] contrôler une capture ASC dans un outil CAN compatible ;
- [ ] déverrouiller volontairement le TX par appui long M5 ;
- [ ] envoyer une trame de test non critique et reverrouiller le TX ;
- [ ] mettre à jour le rapport d’audit avec les résultats ;
- [ ] créer le tag `v0.1-beta` et la release GitHub avec le binaire et son SHA-256.

## Risques connus acceptés pour la bêta

- WebUI en HTTP : utilisation limitée à un réseau local ou point d’accès de confiance ;
- certaines routes héritées doivent encore migrer complètement de GET vers POST ;
- dépendances et Actions GitHub pas encore toutes figées par SHA ;
- marge IRAM/DRAM interne à surveiller lors des prochaines fonctions ;
- CAN classique sans authentification native : ne jamais tester sur un véhicule en circulation.

## Reprise recommandée

1. Brancher le M5StickC Plus2 déjà flashé.
2. Réaliser le test WebUI et noter le résultat.
3. Réaliser le test CAN sur banc isolé, d’abord sans TX.
4. Valider une capture et un replay contrôlé.
5. Fermer les deux dernières cases du rapport de sécurité.
6. Publier seulement ensuite `v0.1-beta`.
