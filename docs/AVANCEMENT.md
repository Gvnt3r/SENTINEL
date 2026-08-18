# Avancement S¢ntïnel v0.1-beta

Dernière mise à jour : 18 août 2026
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

## Matériel disponible pour la validation CAN

> Ceci est le banc précis de l’auteur — un journal, pas une liste d’achat. La fonction réellement requise (transceiver 3,3 V, second nœud actif, terminaison 120 Ω) est décrite indépendamment de la marque dans [Matériel CAN et OBD-II](CAN_HARDWARE.md). Toute pièce équivalente respectant la même tension logique convient.

| Matériel | Décision | Utilisation prévue |
|---|---|---|
| TI `VP230` / SN65HVD230 | **retenu** | transceiver 3,3 V, nœud 1, entre GPIO32/33 du Stick et CANH/CANL |
| Wemos D1 R32 + SparkFun CAN-Bus Shield (`MCP2515` + `MCP2551`) | **retenu** | nœud 2 actif, génère/acquitte le trafic de test |
| module MCP2515 + TJA1050 (`HW-184`) | **écarté** | terminaison 120 Ω non fiable au multimètre (circuit ouvert malgré cavalier) ; remplacé par le shield SparkFun |
| NXP TJA1050 / TJA1050C | écarté en direct | composant 5 V nécessitant alimentation et adaptation de niveaux |
| module d'alimentation `HW-131` | disponible, hors CAN | fournit un rail 3,3 V / 5 V commun au banc ; ce n'est **pas** un level shifter |

### Montage retenu — nœud 1 (Stick + VP230)

| M5StickC Plus2 | VP230 |
|---|---|
| GPIO32 | `D`, `TXD` ou `CTX` |
| GPIO33 | `R`, `RXD` ou `CRX` |
| 3V3 | `VCC` |
| GND | `GND` |
| bus CAN | `CANH` et `CANL` |

- ajouter un condensateur de découplage de 100 nF entre VCC et GND si le module n’en possède pas ;
- placer `Rs` à GND pour le premier essai haute vitesse, ou utiliser une résistance de contrôle de pente adaptée ;
- terminaison 120 Ω confirmée au multimètre sur ce nœud seul (cavalier fermé) ;
- ne pas relier directement un TJA1050 alimenté en 5 V au GPIO33 ;
- effectuer le premier essai uniquement sur un banc isolé, jamais sur un véhicule en circulation.

### Montage retenu — nœud 2 (Wemos D1 R32 + SparkFun CAN-Bus Shield)

Le D1 R32 est au format Uno : le shield s'empile mécaniquement, SPI et masse compris. GPIO réels sous le header (confirmés, pas une supposition) :

| Position header (Uno) | GPIO ESP32 réel | Rôle |
|---|---|---|
| D10 | GPIO5 | CS CAN |
| D11 | GPIO23 | MOSI |
| D12 | GPIO19 | MISO |
| D13 | GPIO18 | SCK |
| D9 | GPIO13 | libre / INT si besoin |

Point critique : le pin `5V` de ce header est du vrai 5V (passthrough USB), pas du 3,3V déguisé. Empilé tel quel, le `MCP2551` tourne à 5V nominal mais sa broche `SO` (MISO) driverait ~5V dans le GPIO19 du D1 R32, non tolérant au-delà de ~3,6V. Toujours pas de level shifter disponible → **soulever la broche `5V` du connecteur du shield** pour qu'elle ne touche plus le pin `5V` du D1 R32, et la relier par un fil volant au `3V3` du D1 R32 à la place. Le reste du header (D9–D13, GND) reste empilé normalement.

Bus CAN côté DB9 (mapping documenté par SparkFun, mais ce shield a des cavaliers qui peuvent le changer — à vérifier sur la carte avant câblage définitif) :

| DB9 | Signal |
|---|---|
| Pin 3 | `CAN_H` |
| Pin 5 | `CAN_L` |
| Pin 2 | `GND` |

- le `MCP2551` est une puce de classe 5 V comme le TJA1050 ; alimenté ici en 3,3 V (broche 5V déroutée) faute de level shifter disponible, hors plage nominale, acceptable uniquement sur banc isolé de test, pas pour un usage prolongé ;
- vérifier la terminaison 120 Ω du shield (cavalier près du DB9) au multimètre, ne pas se fier à la seule sérigraphie ;
- une fois les deux nœuds reliés (VP230 + shield), mesurer CANH/CANL hors tension sur le bus complet : ~60 Ω attendu avec les deux terminaisons actives.

## Risques connus acceptés pour la bêta

- WebUI en HTTP : utilisation limitée à un réseau local ou point d’accès de confiance ;
- certaines routes héritées doivent encore migrer complètement de GET vers POST ;
- dépendances et Actions GitHub pas encore toutes figées par SHA ;
- marge IRAM/DRAM interne à surveiller lors des prochaines fonctions ;
- CAN classique sans authentification native : ne jamais tester sur un véhicule en circulation.

## État du banc (18 août 2026)

- [x] nœud 1 (Stick + VP230) câblé, terminaison 120 Ω confirmée seule au multimètre ;
- [x] nœud 2 initialement prévu avec le module `HW-184`, terminaison non fiable (circuit ouvert) → remplacé par le SparkFun CAN-Bus Shield ;
- [ ] nœud 2 empilé sur le Wemos D1 R32 (broche 5V du shield déroutée vers 3V3, DB9 câblé) ;
- [ ] terminaison 120 Ω du shield SparkFun vérifiée au multimètre ;
- [ ] bus complet mesuré hors tension, CANH/CANL, ~60 Ω attendu ;
- [ ] sketch de test MCP2515 flashé sur le D1 R32, bitrate aligné avec le backend TWAI du Stick ;
- [ ] mise sous tension et premier test Dashboard/TWAI côté Stick.

## Reprise recommandée

1. Souder/router le contournement d'alimentation (5V shield → 3V3 D1 R32), puis empiler le nœud 2 selon le montage ci-dessus et câbler le DB9.
2. Vérifier la terminaison 120 Ω du shield, puis mesurer le bus complet hors tension (~60 Ω attendu).
3. Flasher le D1 R32 avec un sketch MCP2515 de test (trafic périodique ou réponse), bitrate identique à celui prévu côté Stick.
4. Brancher le M5StickC Plus2 déjà flashé, sélectionner le backend TWAI, régler le même bitrate, démarrer le Dashboard en écoute seule.
5. Réaliser le test WebUI et noter le résultat.
6. Valider une capture ASC, la relire dans un outil CAN compatible.
7. Tester un replay contrôlé après déverrouillage physique du TX, puis reverrouiller.
8. Fermer les cases restantes du rapport de sécurité.
9. Publier seulement ensuite `v0.1-beta`.
