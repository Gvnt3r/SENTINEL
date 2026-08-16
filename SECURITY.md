# Politique de sécurité

SENTINEL est un firmware d’audit matériel. Ses fonctions CAN, radio et réseau doivent uniquement être utilisées sur des équipements autorisés.

## Signaler une vulnérabilité

Ne publiez pas immédiatement une vulnérabilité exploitable dans une issue publique. Utilisez de préférence l’onglet **Security** du dépôt GitHub et un signalement privé. Incluez la version, la cible matérielle, les conditions de reproduction et l’impact observé.

Les secrets, captures automobiles, identifiants Wi-Fi, configurations WireGuard et scénarios privés ne doivent jamais être joints au rapport sans les avoir anonymisés.

## Périmètre

La branche SENTINEL maintient en priorité :

- les backends CAN TWAI/SLCAN et leur verrouillage d’émission ;
- les captures, scénarios et API Web CAN ;
- le diagnostic ELM327 ;
- l’interface spécifique M5StickC Plus2.

Une partie importante du firmware provient de Bruce. Les problèmes purement hérités et reproductibles sur Bruce doivent également être signalés au projet amont, sans divulguer prématurément un exploit.

## Règles de release

- aucun secret ni fichier de configuration personnel suivi par Git ;
- compilation reproductible de la cible publiée ;
- fonctions d’écriture protégées par authentification et garde-fous locaux ;
- rapport d’audit relu et vulnérabilités critiques ou élevées fermées avant publication ;
- somme SHA-256 publiée avec chaque binaire de release.
