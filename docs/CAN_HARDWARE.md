# Matériel CAN et OBD-II

SENTINEL sépare trois usages : capture CAN autonome sur le Stick, diagnostic OBD-II et analyse approfondie sur ordinateur.

> **Indispensable vs remplaçable —** ce qui est requis, c’est la fonction : un transceiver 3,3 V (idéalement isolé) pour le backend TWAI, ou un adaptateur SLCAN 3,3 V ; un second nœud CAN actif sur le banc de test ; une terminaison 120 Ω à chaque extrémité du bus. Les références précises citées ici (VP230, MCP2515, SparkFun CAN-Bus Shield…) sont des exemples qui fonctionnent, pas des exigences de marque — tout composant équivalent respectant la même tension logique et la même fonction convient. Le détail du banc précis de l’auteur, avec son historique de bricolage, est journalisé dans [Avancement v0.1-beta](AVANCEMENT.md) ; ce document-ci reste la référence générique.

## 1. Capture autonome recommandée : ESP32 TWAI

Le backend `TWAI` utilise le contrôleur CAN classique intégré à l’ESP32. Il nécessite obligatoirement un **transceiver CAN 3,3 V** externe, idéalement isolé galvaniquement. Le firmware démarre en écoute seule et n’autorise l’émission qu’après un appui physique prolongé pour la session en cours.

Pour le banc de validation v0.1-beta, le composant retenu est un **TI VP230 / SN65HVD230** alimenté en 3,3 V. Les NXP TJA1050/TJA1050C disponibles sont des composants 5 V et ne doivent pas être raccordés directement à l’entrée RX du Stick sans adaptation de niveaux. Le module MCP2515 disponible communique en SPI : il peut servir de nœud CAN de test indépendant, mais n’est pas le transceiver du backend TWAI actuel.

| M5StickC Plus2 | Transceiver CAN |
|---|---|
| GPIO32 | TXD |
| GPIO33 | RXD |
| GND | GND logique |
| 3V3 | alimentation logique compatible |

Relier ensuite `CANH` et `CANL` au réseau. Ne pas ajouter de résistance de terminaison sur une prise OBD-II d’un véhicule normalement terminé. Sur un banc isolé, placer 120 Ω à chaque extrémité du bus.

### Modules CAN M5Stack officiels

Le **Unit CAN** M5Stack actuel (chip `CA-IS3050G`) n’est pas un module UART à protocole propriétaire contrairement à une note précédente de ce document : c’est un transceiver CAN isolé galvaniquement, à sortie TXD/RXD brute, du même principe que le VP230 ci-dessus. Sa tension logique côté TXD/RXD n’est en revanche pas documentée par M5Stack sur ce connecteur JST à rail unique 5 V : le `CA-IS3050G` accepte 2,5–5 V côté logique selon son alimentation VIO, mais rien ne confirme que ce breakout régule ce rail à 3,3 V. Ne pas le relier à GPIO32/33 sans avoir vérifié la tension réelle en sortie (multimètre, hors bus) ou sans level shifter par précaution.

D’autres modules CAN existent dans la gamme M5Stack, avec des interfaces différentes du backend TWAI actuel :

| Module M5Stack | Chip | Interface | Compatibilité avec SENTINEL v0.1-beta |
|---|---|---|---|
| Unit CAN | `CA-IS3050G` | TXD/RXD brut, isolé, alim. 5 V | proche TWAI, tension logique TXD/RXD à vérifier avant câblage |
| Unit Mini CAN | `TJA1051T` | TXD/RXD brut (à confirmer) | non testé ; TJA = composants historiquement 5 V, prudence identique au TJA1050 |
| Module COMMU | `MCP2515` (SPI) + `SP3485EN` | SPI vers l’hôte (CS/INT/SCK/SI/SO sur GPIO12/15/18/19/23) | non compatible : ni le backend TWAI ni SLCAN ne pilotent de contrôleur SPI |

Ces trois modules sont des candidats de backends distincts, pas des équivalents du VP230 câblé directement. Aucun n’est implémenté ni validé électriquement dans ce fork à ce stade ; ce tableau documente la compatibilité connue pour éviter un mauvais câblage, pas une prise en charge logicielle.

## 2. Adaptateur Lawicel/SLCAN

Le backend `SLCAN` reste compatible avec un adaptateur UART Lawicel à 115200 bauds :

| M5StickC Plus2 | SLCAN |
|---|---|
| GPIO32 TX | RX 3,3 V |
| GPIO33 RX | TX 3,3 V |
| GND | GND |

SENTINEL contrôle la fermeture, le débit et l’ouverture du canal. Les débits disponibles sont 10, 20, 50, 100, 125, 250, 500, 800 kbit/s et 1 Mbit/s.

## 3. ELM327 Bluetooth Classic

Le mode ELM327 est réservé au diagnostic OBD-II : initialisation AT, sélection automatique du protocole, lecture des PID régime/vitesse/température et lecture des DTC. L’effacement des DTC exige une confirmation explicite. Un ELM327 ne remplace pas une interface CAN brute pour les captures ASC ou le replay.

Le nom Bluetooth et le PIN sont saisis sur le Stick. Les valeurs courantes des clones sont `OBDII`, `1234` ou `0000`. Bluetooth Classic et les fonctions BLE du firmware sont utilisés alternativement.

## 4. CANable

CANable est recommandé comme compagnon PC sous Linux, macOS ou Windows. Avec le firmware Candlelight il s’intègre à SocketCAN sous Linux ; avec slcan il fournit une interface série Lawicel. Il ne se connecte pas directement au port Grove USB du Stick : on l’utilise en parallèle sur le même banc CAN pour valider une capture, examiner le trafic avec SavvyCAN/Wireshark et comparer les horodatages.

## Câble OBD-II passif

Pour les essais autorisés sur véhicule :

| Broche OBD-II | Signal |
|---|---|
| 6 | CAN High |
| 14 | CAN Low |
| 4 ou 5 | masse de référence, uniquement si le module l’exige |
| 16 | +12 V véhicule — **ne pas connecter à SENTINEL** |

Alimenter le M5Stick et l’interface isolée par USB ou batterie. Ne jamais injecter le +12 V OBD dans le Grove ou les GPIO. Vérifier le brochage, la masse, l’isolation et le débit avant connexion.

## Sécurité d’émission

- démarrage systématique en écoute seule ;
- déverrouillage TX physique par appui long, valable uniquement pour la session ;
- l’API Web peut reverrouiller TX mais ne peut pas le déverrouiller ;
- changement de backend et arrêt du transport reverrouillent TX ;
- CAN et Proxmark sont mutuellement exclusifs sur le port Grove.

CAN-FD n’est pas pris en charge par le contrôleur TWAI de l’ESP32. ISO-TP, UDS et DBC restent des évolutions futures.
