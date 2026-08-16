# SENTINEL

Firmware de sécurité embarquée et d’analyse automobile pour le **M5StickC Plus2**.

SENTINEL transforme le Stick en instrument portable consacré à l’observation, la capture et l’émission sur bus CAN, tout en conservant les nombreuses capacités ESP32 héritées de Bruce.

> Ce projet est un fork indépendant en cours de développement. Utilisez les fonctions d’émission radio ou automobile uniquement sur du matériel et des réseaux que vous possédez ou pour lesquels vous disposez d’une autorisation explicite.

## Origine du projet

SENTINEL est basé sur le firmware open source [Bruce](https://github.com/BruceDevices/firmware), développé par Bruce Devices et ses contributeurs.

Bruce fournit la fondation technique du projet : prise en charge de l’ESP32 et du M5Stack, pilotes matériels, stockage SD/LittleFS, Wi-Fi, BLE, RFID, infrarouge, radio, WebUI et infrastructure générale du firmware.

Le code original demeure soumis à sa licence et les crédits reviennent à ses auteurs respectifs. SENTINEL n’est pas une version officielle de Bruce et n’est pas affilié à Bruce Devices.

## Ce que SENTINEL modifie

Le fork ajoute une identité et une expérience spécifiques au M5StickC Plus2 :

- démarrage **SENTINEL** sans l’ancien visuel Bruce ;
- animation automobile avec radar, activité CAN et diagnostic système ;
- interface plein écran adaptée au petit écran du Stick ;
- palette instrument automobile à fort contraste : noir, cyan, ambre, vert et rouge ;
- navigation verticale, une fonction par page ;
- menus Automotive et Proxmark dédiés ;
- port Grove partagé entre l’adaptateur CAN SLCAN et le Proxmark3.

Les fonctions générales provenant de Bruce restent disponibles, mais l’interface et les développements futurs de ce fork sont orientés diagnostic et audit embarqué.

## Suite CAN automobile

SENTINEL introduit une couche CAN autonome multi-backend : contrôleur ESP32 TWAI avec transceiver externe, ou adaptateur UART compatible Lawicel/SLCAN :

- trames CAN classiques standard 11 bits et étendues 29 bits ;
- trames de données et RTR, DLC de 0 à 8 ;
- initialisation contrôlée avec commandes `C`, `Sx` et `O` ;
- débits Lawicel de 10 kbit/s à 1 Mbit/s ;
- vue live avec débit de trames, IDs actifs et dernière trame ;
- table limitée à 128 IDs avec compteurs, fréquence et octets variables ;
- capture horodatée au format ASC sur carte SD ;
- replay conservant l’ordre et la temporisation des trames ;
- filtres persistants dans `/Automotive/filters.json` ;
- scénarios JSON avec écriture atomique dans `/Automotive/scenarios/` ;
- validation des IDs, DLC, données, délais et répétitions côté firmware.
- démarrage en écoute seule et déverrouillage TX physique par appui long ;
- diagnostic ELM327 Bluetooth Classic séparé du CAN brut : PID moteur, vitesse, température et DTC.

CAN-FD, ISO-TP, UDS et DBC ne sont pas encore pris en charge.

## WebUI CAN

La WebUI héritée de Bruce a été étendue avec un panneau responsive pour :

- consulter l’état de l’adaptateur CAN ;
- afficher les IDs actifs ;
- envoyer une trame validée par le firmware ;
- créer, consulter et supprimer des scénarios ;
- télécharger les captures avec le gestionnaire de fichiers existant.

Les routes CAN réutilisent l’authentification de la WebUI :

```text
GET    /api/can/status
GET    /api/can/ids
GET    /api/can/config
POST   /api/can/config
POST   /api/can/lock
POST   /api/can/send
GET    /api/can/scenarios
POST   /api/can/scenario
DELETE /api/can/scenario
```

## Matériel ciblé

- M5Stack M5StickC Plus2 ;
- transceiver CAN classique 3,3 V, de préférence isolé, pour ESP32 TWAI ;
- ou adaptateur UART Lawicel/SLCAN à 115200 bauds ;
- ELM327 Bluetooth Classic facultatif pour le diagnostic OBD-II ;
- CANable facultatif comme outil compagnon sur ordinateur ;
- logique UART 3,3 V ;
- carte microSD pour les captures et scénarios ;
- Proxmark3 RDV4 facultatif.

### Câblage SLCAN

| M5StickC Plus2 | Adaptateur SLCAN |
|---|---|
| GPIO32 TX | RX |
| GPIO33 RX | TX |
| GND | GND |

Le port Grove est une ressource partagée : CAN et Proxmark ne doivent pas l’utiliser simultanément.

Le guide [Matériel CAN et OBD-II](docs/CAN_HARDWARE.md) décrit le câblage TWAI/SLCAN, le câble OBD-II passif, ELM327, CANable et les règles de sécurité. Le +12 V de la broche 16 OBD-II ne doit jamais être relié au Stick.

## Compiler

Le projet utilise PlatformIO :

```sh
pio run -e m5stack-cplus2
```

Le binaire fusionné est produit sous le nom :

```text
S¢ntïnel-v0.1-beta-m5stack-cplus2.bin
```

Le nom de version officiel du firmware est **S¢ntïnel-v0.1-beta**. L’interface compacte conserve le logotype lisible **SENTINEL** sur l’écran du Stick.

## Flasher

Avec un M5StickC Plus2 connecté sur `/dev/ttyACM0` :

```sh
pio run -e m5stack-cplus2 -t upload --upload-port /dev/ttyACM0
```

La taille exacte dépend des fonctions Bluetooth liées à la cible ; la compilation PlatformIO affiche l’usage réel de la flash et de la RAM avant flashage.

## Structure des ajouts

```text
src/
├── core/menu_items/
│   ├── AutomotiveMenu.*
│   └── ProxmarkMenu.*
└── modules/
    ├── automotive/
    │   ├── can_suite.*
    │   ├── elm327_service.*
    │   └── slcan_tools.*
    └── proxmark/
        └── proxmark_bridge.*
```

## Feuille de route

- finaliser l’éditeur local de trames et de scénarios ;
- améliorer la détection et les commandes Proxmark3 RDV4 ;
- ajouter ISO-TP et UDS ;
- intégrer un décodage DBC simplifié ;
- organiser les captures par dossier d’audit et par véhicule ;
- poursuivre la séparation visuelle et fonctionnelle avec Bruce.

## Crédits et licence

- Base du firmware : [Bruce Devices Firmware](https://github.com/BruceDevices/firmware)
- Proxmark3 : [Iceman](https://github.com/iceman1001) et la communauté [RfidResearchGroup/proxmark3](https://github.com/RfidResearchGroup/proxmark3)
- Fork SENTINEL : [Gvnt3r/SENTINEL](https://github.com/Gvnt3r/SENTINEL)
- Licence : voir [LICENSE](./LICENSE)

Un grand merci :

- à **Bruce Devices** et à tous les contributeurs de Bruce pour la base ESP32/M5Stack, les pilotes, les modules et l’infrastructure sur lesquels SENTINEL est construit ;
- à **Iceman** pour son travail majeur autour du Proxmark3 et pour les outils, recherches et connaissances partagés avec la communauté RFID ;
- à la **RfidResearchGroup** et à l’ensemble des contributeurs du client Proxmark3 ;
- aux communautés open source de la sécurité matérielle, automobile et radio.

SENTINEL est un fork indépendant. Les noms Bruce et Proxmark3 appartiennent à leurs projets respectifs ; leur mention constitue un crédit et non une affiliation officielle.
