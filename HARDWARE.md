# Nano808 V13 — matériel et brochage

Ce document décrit le câblage attendu par le firmware présent dans
`firmware/`. Les éléments « à confirmer » ne sont pas validés sur matériel.
D9 est réservé à l'audio ; D10 reste libre dans cette V13.

## Matériel connu

| Élément | État |
| --- | --- |
| Microcontrôleur | Arduino Nano / ATmega328P ; modèle exact à confirmer |
| Alimentation prototype | 9 V vers VIN, GND commun ; consommation à vérifier |
| Affichage | MAX7219 + matrice 8×8, logique 5 V |
| Audio | PWM Mozzi sur D9 ; filtre et niveau Eurorack à valider |
| Commandes | six potentiomètres 10 kΩ linéaires et quatre boutons |

## Brochage V13

| Fonction | Broche | Direction | Câblage | Remarques |
| --- | --- | --- | --- | --- |
| Instrument | A0 | entrée analogique | curseur pot 10 kΩ ; extrémités 5 V/GND | six zones |
| Paramètre 1 | A1 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Paramètre 2 | A2 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Paramètre 3 | A3 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Pas à éditer | A4 | entrée analogique | curseur pot 10 kΩ | 16 zones filtrées |
| Longueur | A5 | entrée analogique | curseur pot 10 kΩ | 1–16, non destructive |
| RESET | D3 | entrée numérique | bouton vers GND | `INPUT_PULLUP`, FALLING |
| MAX7219 DIN | D4 | sortie | DIN | — |
| MAX7219 CLK | D5 | sortie | CLK | — |
| MAX7219 CS/LOAD | D6 | sortie | CS/LOAD | — |
| Step ON/OFF | D7 | entrée numérique | bouton vers GND | anti-rebond 20 ms |
| FIRE / MUTE | D8 | entrée numérique | bouton vers GND | court / long |
| Audio Mozzi | D9 | sortie PWM | filtre audio | broche réservée |
| CLOCK futur | D2 | entrée configurée | ne rien câbler | non traité en V13 |
| Libre | D10, D12, D13 | — | ne rien câbler | D13 : LED future possible |

Tous les boutons utilisent la résistance de rappel interne. Le montage réel
doit être vérifié pour les parasites, le rebond et les niveaux.

## MAX7219

VCC va au 5 V et GND à la masse commune. Prévoir au plus près du module au
minimum 100 nF entre 5 V et GND ; le guide V13 recommande aussi 100 µF de
réserve. Éviter de faire passer le retour MAX7219 par le chemin audio.

L'affichage logique place l'instrument en colonne 0, les bargraphes des trois
paramètres en colonnes 2, 4 et 6, puis les pas 1–8 et 9–16 sur les deux lignes
du bas. L'orientation réelle dépend du module et doit être confirmée.

## Audio et alimentation

Le code produit le PWM Mozzi sur D9 à 16 384 Hz et applique un filtrage
numérique léger. Le filtre analogique, le condensateur de liaison, le niveau
de sortie, la protection et le connecteur Eurorack restent à mesurer et à
valider. Ne pas connecter D10 à l'audio de cette version.

Le schéma détaillé proposé pour le prototype est dans
[Nano808_V13_Hardware.md](Nano808_V13_Hardware.md). Il ne remplace pas une
validation électrique du montage final.
