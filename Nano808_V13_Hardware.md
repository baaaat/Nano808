# Nano808 V13 — guide hardware prototype

## Objectif

Cette V13 conserve l'architecture validée du prototype :
- Arduino Uno pour les essais ou Nano pour la version compacte ;
- alimentation générale 9 V ;
- MAX7219 + matrice 8×8 ;
- sortie audio Mozzi sur D9 ;
- 6 instruments ;
- 16 steps ;
- 3 paramètres par instrument avec soft takeover.

Elle ajoute :
- FIRE ;
- longueur de séquence 1 à 16 ;
- REC / variation par step ;
- RESET ;
- MUTE par instrument.

La philosophie reprend Mutant/Freaq : contrôle direct, peu de menus, séquenceur 16 pas, bouton de retrigger, REC et contrôle de longueur.

---

## 1. Brochage complet V13

| Broche Arduino | Fonction V13 | Câblage |
|---|---|---|
| A0 | Sélection instrument | curseur pot 10 kΩ |
| A1 | Paramètre 1 | curseur pot 10 kΩ |
| A2 | Paramètre 2 | curseur pot 10 kΩ |
| A3 | Paramètre 3 | curseur pot 10 kΩ |
| A4 | Sélection step 1–16 | curseur pot 10 kΩ |
| A5 | LENGTH, longueur 1–16 | curseur pot 10 kΩ |
| D2 | réservé CLOCK IN futur | ne rien câbler pour l'instant |
| D3 | RESET | bouton poussoir vers GND |
| D4 | MAX7219 DIN | DIN matrice |
| D5 | MAX7219 CLK | CLK matrice |
| D6 | MAX7219 CS/LOAD | CS matrice |
| D7 | STEP ON/OFF | bouton poussoir vers GND |
| D8 | FIRE / MUTE | bouton poussoir vers GND |
| D9 | AUDIO PWM Mozzi | filtre audio |
| D10 | réservé / laisser libre | ne rien câbler |
| D11 | REC / VARIATION | bouton poussoir vers GND |
| D12 | START / STOP | bouton poussoir vers GND |
| D13 | SHIFT | bouton poussoir vers GND ; LED intégrée indisponible |
| 5V | rail logique | pots + MAX7219 |
| GND | masse | masse commune |

D10 est volontairement laissé libre sur cette version. La sortie Mozzi standard de l'ATmega328 utilise D9 et la temporisation audio dépend du Timer1 ; garder D10 libre évite de compliquer le prototype.

---

## 2. Les six potentiomètres

Tous les pots sont des 10 kΩ linéaires branchés EN PARALLÈLE, jamais en série.

Pour chaque pot :

    5V ----- patte extérieure
                  |
               [ POT 10k ]
                  |
    entrée A? --- curseur central
                  |
    GND ---- patte extérieure

Affectation :
- A0 : INSTRUMENT
- A1 : PARAM 1
- A2 : PARAM 2
- A3 : PARAM 3
- A4 : STEP SELECT
- A5 : LENGTH

Si le sens d'un pot est inversé, permuter uniquement ses deux pattes extérieures 5V/GND.

---

## 3. Nouveaux boutons

Utiliser des boutons poussoirs normalement ouverts.

### STEP ON/OFF — D7

    D7 ----- bouton ----- GND

Appui court :
- active le step sélectionné avec A4 s'il est OFF ;
- désactive le step s'il est ON.

### FIRE / MUTE — D8

    D8 ----- bouton ----- GND

Appui court :
- déclenche immédiatement l'instrument sélectionné.

Appui long, environ 0,65 s :
- MUTE / UNMUTE l'instrument sélectionné dans le séquenceur.

Le FIRE reste utilisable pour écouter un instrument même s'il est muté dans la séquence.

Quand l'instrument sélectionné est muté, son point dans la colonne instrument clignote.

### REC / VARIATION — D11

    D11 ----- bouton ----- GND

Appui :
- bascule la variation du step sélectionné pour l'instrument sélectionné.

Appui long, environ 0,65 s :
- enregistre les trois paramètres actuels sur le step sélectionné ;
- si ce step possède déjà un enregistrement, l'efface.

Le rappel des paramètres enregistrés est automatique pendant la lecture. Ces
valeurs sont conservées en RAM uniquement et sont perdues à l'extinction.

Cette V13 utilise une variation légère et spécifique à chaque percussion :
- Kick : davantage de punch + accord légèrement plus haut ;
- Snare : accord/bruit légèrement différents ;
- Hats : métal légèrement plus aigu ;
- Clap : accent de niveau ;
- Tom : accord légèrement relevé.

La grille ON/OFF reste totalement indépendante : D7 décide si le hit existe, D11 décide si ce hit est normal ou en variation.

La variation est stockée sous forme d'un seul bit par step, donc seulement 12 octets pour les 6 instruments.

### RESET — D3

    D3 ----- bouton ----- GND

Appui :
- retour immédiat au step 1 ;
- le step 1 est rejoué immédiatement ;
- la phase de clignotement est recalée.

Le code utilise INPUT_PULLUP : aucune résistance externe n'est nécessaire pour ces boutons.

---

## 4. Pot LENGTH — A5

A5 règle la longueur globale de boucle entre 1 et 16 steps.

Exemples :
- 16 = boucle normale de 16 pas ;
- 12 = boucle de 12 pas ;
- 7 = boucle impaire très utile pour créer un déphasage avec les autres modules.

Les patterns 16 steps ne sont pas effacés quand on raccourcit la longueur. Si on revient ensuite à 16, les anciens steps sont toujours présents.

---

## 5. Matrice MAX7219

Connexions :

| MAX7219 | Arduino |
|---|---|
| VCC | 5V |
| GND | GND |
| DIN | D4 |
| CLK | D5 |
| CS/LOAD | D6 |

Affichage conservé :
- deux lignes du bas = 16 steps, gauche vers droite ;
- colonne 0 = instrument ;
- colonne 2 = paramètre 1 ;
- colonne 4 = paramètre 2 ;
- colonne 6 = paramètre 3.

Le step sélectionné par A4 clignote au BPM même s'il est actif.

---

## 6. Réduction du buzz du MAX7219

Le test sur breadboard a montré que le MAX7219 injecte du bruit principalement par son alimentation/retour de masse.

Au plus près du module MAX7219 :

    5V -----+---------------- VCC MAX7219
            |
            +--- 100 nF (104) --- GND
            |
            +--- 100 µF --------- GND

Pour le 100 µF électrolytique :
- + vers 5V ;
- - vers GND.

Le 104 céramique n'est pas polarisé.

Un 220 µF peut remplacer le 100 µF. Un 1000 µF peut être testé comme capacité de réserve supplémentaire mais ne remplace pas le 104.

### Masse

Garder le GND du MAX7219 sur un retour séparé jusqu'au GND principal/Arduino, comme le test qui a fortement réduit le buzz.

Éviter :

    MAX7219 GND -> masse audio -> Arduino GND

Préférer :

                   +--- GND audio
    GND principal -+
                   +--- GND MAX7219

Sur le PCB définitif, on fera une vraie stratégie de plan de masse / retour de courant au lieu de dépendre des contacts de breadboard.

---

## 7. Alimentation 9 V du prototype

### Avec Arduino Uno

Le plus simple :

    alimentation 9 V -> prise jack d'alimentation UNO

Puis :
- 5V UNO -> potentiomètres ;
- 5V UNO -> MAX7219 ;
- GND UNO -> masses.

### Avec Nano

    9 V -> VIN Nano
    GND -> GND Nano

Puis le 5V régulé du Nano alimente les commandes et le MAX7219, tant que la consommation de la matrice reste raisonnable.

Pour le PCB final :
- protection inversion de polarité ;
- condensateur de réserve sur 9V ;
- 100 nF de découplage ;
- routage séparé des retours LED/audio.

---

## 8. Sortie audio

Mozzi PWM :

    D9 --- 270 ohms ---+--- 10 µF --- 1 kΩ --- AUDIO OUT TIP
                       |
                     100 nF
                       |
                      GND

Après le condensateur de liaison 10 µF :
- ajouter 100 kΩ vers GND ;
- sleeve du jack AUDIO OUT vers GND.

Polarité du 10 µF :
- + côté Arduino / filtre ;
- - côté sortie jack.

---

## 9. ⚠️ Avertissement — CLOCK externe sur D2

D2 est maintenant l'entrée CLOCK externe. Un front montant fait avancer un pas
après validation de la période mesurée.

La V13 possède déjà une fonction interne permettant de mettre à jour :
- la durée d'un step ;
- la durée d'un beat ;
- le clignotement du step sélectionné.

La période des fronts valides est mesurée et le tempo d'affichage/séquenceur
est dérivé du signal reçu au lieu de 90 BPM.

L'entrée doit recevoir un signal logique 0–5 V protégé avec masse commune. Une
horloge Eurorack ±5 V ou ±10 V ne doit jamais être raccordée directement à D2,
au risque d'endommager l'Arduino. L'adaptation et la protection restent à
réaliser et valider.

---

## 10. Swing

L'infrastructure du swing est déjà présente dans le code mais `swingPercent = 0`.

Il n'y a volontairement pas encore de commande physique de swing pour ne pas surcharger l'interface avant validation de FIRE / LENGTH / REC / MUTE.

---

## 11. Résumé des commandes

| Commande | Fonction |
|---|---|
| A0 | instrument |
| A1 | paramètre 1 |
| A2 | paramètre 2 |
| A3 | paramètre 3 |
| A4 | step à éditer |
| A5 | longueur 1–16 |
| D7 | step ON/OFF |
| D8 court | FIRE |
| D8 long | MUTE/UNMUTE instrument |
| D11 | variation REC du step |
| D3 | RESET step 1 |
