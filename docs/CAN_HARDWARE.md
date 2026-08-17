# Matériel CAN et OBD-II

SENTINEL sépare trois usages : capture CAN autonome sur le Stick, diagnostic OBD-II et analyse approfondie sur ordinateur.

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

> Le **M5Stack Unit CAN** classique contient son propre microcontrôleur et dialogue en UART avec un protocole propriétaire : ce n’est pas un simple transceiver TWAI. Il pourra devenir un backend distinct, mais ne doit pas être câblé comme le tableau ci-dessus. Utiliser pour TWAI un module transceiver explicitement prévu pour une entrée TXD/RXD 3,3 V.

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
