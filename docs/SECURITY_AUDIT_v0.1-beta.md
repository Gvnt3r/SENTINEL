# Audit sécurité pré-release — v0.1-beta

Date : 2026-08-16  
Statut : **audit logiciel validé ; release bloquée jusqu’aux tests WebUI et CAN sur matériel**

## Corrigé avant release

| ID | Sévérité | Sujet | Correction |
|---|---|---|---|
| SEN-001 | Élevée | Jetons WebUI générés avec un PRNG générique | utilisation du générateur matériel ESP32 et nouveau cookie `SENTINELSESSION` |
| SEN-002 | Élevée | Anciennes sessions réutilisables après mise à jour | changement de nom du cookie, imposant une nouvelle connexion |
| SEN-003 | Élevée | Croissance mémoire non bornée sur réponse ELM327 | réponse limitée à 4096 octets |
| SEN-004 | Élevée | Lecture concurrente de la table d’IDs CAN par l’API | instantané protégé par mutex FreeRTOS |
| SEN-005 | Moyenne | IDs et DLC Web acceptant des suffixes non numériques | analyse stricte et bornée |
| SEN-006 | Moyenne | Scénarios JSON pouvant épuiser la mémoire | limite de 48 Kio avant désérialisation |
| SEN-007 | Moyenne | Écriture de scénario non restaurable après échec de renommage | fichier temporaire, sauvegarde et rollback |
| SEN-008 | Élevée | Clé privée WireGuard affichée sur le port série | suppression de la journalisation des lignes et de la clé privée |
| SEN-009 | Moyenne | Mot de passe WebUI placé dans une URL et renvoyé en clair | route convertie en POST et réponse expurgée |
| SEN-010 | Moyenne | Cookie de session sans politique SameSite | ajout de `HttpOnly; SameSite=Strict` |

## Risques résiduels suivis

| ID | Sévérité | Sujet | Décision v0.1-beta |
|---|---|---|---|
| SEN-R01 | Moyenne | WebUI servie en HTTP sur le réseau local | documenter : utiliser uniquement un AP/local de confiance ; HTTPS embarqué à étudier |
| SEN-R02 | Moyenne | Plusieurs dépendances et Actions GitHub référencées par tags/branches plutôt que SHA | figer les révisions dans une version ultérieure après inventaire SBOM |
| SEN-R03 | Moyenne | Fonctions héritées de gestion de fichiers utilisent encore GET pour certaines mutations | protégé par session SameSite ; migration complète vers POST planifiée |
| SEN-R04 | Faible | BluetoothSerial est déprécié dans Arduino-ESP32 3.x | surveiller la migration avant Arduino-ESP32 4.x |
| SEN-R05 | Opérationnelle | CAN classique n’authentifie pas les trames | écoute seule par défaut et déverrouillage TX physique obligatoire |
| SEN-R06 | Moyenne | Marge faible dans les régions internes IRAM (90,75 %) et DRAM (91,21 %) | surveiller chaque évolution ; éviter les allocations statiques et fonctions IRAM non indispensables |

## Contrôles de publication

- [x] compilation `m5stack-cplus2` réussie après corrections (RAM globale 34,7 %, flash 51,8 %) ;
- [x] `git diff --check` sans erreur ;
- [x] recherche de secrets sans résultat réel dans l’arbre de travail, hors `.git`, `.pio` et binaires ;
- [x] binaire final généré après correction de navigation (4 414 960 octets), SHA-256 `02a15cea1ce713238edd2c3b1ca2553b61ef9dfbc2482874f3d5f0315f653a84` ;
- [ ] test de connexion WebUI avec invalidation des anciennes sessions ;
- [ ] test matériel CAN en écoute seule puis déverrouillage physique ;
- [ ] tag et release créés uniquement après validation des cases précédentes.
