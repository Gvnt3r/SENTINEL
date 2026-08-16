# M5StickC Plus2 — Proxmark et carte SD externe

Ce firmware ajoute une application **Proxmark** à Bruce. Elle transforme le M5StickC Plus2 en pont
transparent entre le client Proxmark3 sur le PC (USB-C du M5) et l'UART 3,3 V du Proxmark.

## Câblage Proxmark (port Grove)

| M5StickC Plus2 | Proxmark UART |
|---|---|
| GPIO 32 (TX) | RX |
| GPIO 33 (RX) | TX |
| GND | GND |

Le Proxmark doit être alimenté séparément. Ne jamais injecter du 5 V dans les GPIO du M5Stick.
Le pont utilise 115200 bauds, 8N1. Dans Bruce : **Proxmark > UART Bridge**. Ensuite lancer le client
Proxmark3 sur le port série USB du M5Stick. La variante **Bridge + SD log** enregistre le trafic brut
dans `/Proxmark/uart_capture.bin`.

## Câblage microSD SPI externe

| Signal SD | GPIO M5StickC Plus2 |
|---|---:|
| CS | 14 |
| SCK | 0 |
| MISO | 25 |
| MOSI | 26 |
| VCC | 3,3 V |
| GND | GND |

La SD et le Proxmark utilisent des broches distinctes et peuvent donc rester connectés ensemble.
Formater la carte en FAT32, puis ouvrir **Files > SD Card**. Éviter les modules microSD conçus
uniquement pour une alimentation/logique 5 V.

## Compilation

```sh
pio run -e m5stack-cplus2
```

Le binaire se trouve sous `.pio/build/m5stack-cplus2/`.

## Suite automobile CAN

Le menu **Automotive** pilote un adaptateur CAN UART compatible avec le protocole texte Lawicel/SLCAN.
Le port Grove est utilisé à 115200 bauds : GPIO32 vers RX, GPIO33 depuis TX et masse commune. Il est
partagé avec le Proxmark, les deux applications ne peuvent donc pas utiliser l'UART simultanément.

Les captures 125/250/500 kbit/s sont écrites dans `/Automotive/*.slcan`. Ces fichiers peuvent être
relus depuis le menu Replay. Chaque ligne est validée comme trame SLCAN standard ou étendue avant
transmission ; les autres lignes sont ignorées.
