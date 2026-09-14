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
| Commandes | six potentiomètres 10 kΩ linéaires et cinq boutons |

## Brochage V13

| Fonction | Broche | Direction | Câblage | Remarques |
| --- | --- | --- | --- | --- |
| Instrument | A0 | entrée analogique | curseur pot 10 kΩ ; extrémités 5 V/GND | six zones avec hystérésis |
| Paramètre 1 | A1 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Paramètre 2 | A2 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Paramètre 3 | A3 | entrée analogique | curseur pot 10 kΩ | soft takeover |
| Pas à éditer | A4 | entrée analogique | curseur pot 10 kΩ | 16 zones égales filtrées |
| Longueur | A5 | entrée analogique | curseur pot 10 kΩ | 16 zones, 1–16, non destructive |
| RESET | D3 | entrée numérique | bouton vers GND | `INPUT_PULLUP`, FALLING |
| MAX7219 DIN | D4 | sortie | DIN | — |
| MAX7219 CLK | D5 | sortie | CLK | — |
| MAX7219 CS/LOAD | D6 | sortie | CS/LOAD | — |
| Step ON/OFF | D7 | entrée numérique | bouton vers GND | anti-rebond 20 ms |
| FIRE / MUTE | D8 | entrée numérique | bouton vers GND | court / long |
| Audio Mozzi | D9 | sortie PWM | filtre audio | broche réservée |
| START/STOP | D12 | entrée numérique | bouton vers GND | `INPUT_PULLUP`, anti-rebond 20 ms |
| CLOCK externe | D2 | entrée numérique | signal d'horloge protégé, masse commune | front montant = 1 pas |
| Libre | D10 | — | ne rien câbler | D10 reste réservé/libre |
| SHIFT | D13 | entrée numérique | bouton vers GND | utilisé avec D12 |

Tous les boutons utilisent la résistance de rappel interne. Le montage réel
doit être vérifié pour les parasites, le rebond et les niveaux.

## Câblage et réglage des potentiomètres

Chaque potentiomètre est câblé en parallèle, jamais en série : une extrémité
au 5 V, l'autre au GND et le curseur sur l'entrée indiquée. Utiliser des
potentiomètres linéaires de 10 kΩ et des fils courts pour A4 et A5.

A4 et A5 utilisent une plage logicielle commune de 24 à 999 mesures ADC. Les
valeurs en dessous/au-dessus sont rabattues sur les extrémités ; la plage
intermédiaire est divisée en 16 zones égales. Le firmware applique un filtre
progressif, une hystérésis de 8 mesures et une confirmation de trois lectures
pour A4. Ces choix évitent les changements intempestifs sans attente bloquante.

Si la première ou la dernière position n'est pas atteignable sur le montage,
mesurer les valeurs ADC réelles et ajuster `STEP_ADC_MIN` / `STEP_ADC_MAX`
dans le sketch ; reporter alors les valeurs mesurées ici. Ne pas modifier ces
constantes uniquement pour compenser un câblage inversé : permuter les deux
extrémités 5 V/GND du potentiomètre.

Les paramètres A1–A3 ont aussi un filtrage et une zone morte de deux niveaux
sur 255. Après changement d'instrument, le soft takeover empêche un saut : le
potentiomètre doit rejoindre ou croiser la valeur mémorisée.

Le bouton START/STOP sur D12 bascule le transport. À l'arrêt, la position du
séquenceur est conservée et les voix déjà déclenchées finissent leur enveloppe.
Au redémarrage, la lecture reprend à la position conservée. Le bouton est
normalement ouvert : D12 — bouton — GND, sans résistance externe.

Le bouton SHIFT utilise D13, également reliée à la LED intégrée de l'UNO. Son
association avec D12 transforme temporairement D12 en tap tempo : maintenir
SHIFT et appuyer plusieurs fois sur START/STOP règle le tempo par l'intervalle
entre les appuis. Un appui sur D12 seul conserve la fonction START/STOP.

Le bouton REC sur D11 conserve deux actions : un appui court bascule la
variation du pas ; un appui long (environ 650 ms) enregistre les trois
paramètres actuels sur ce pas. Si un enregistrement existe déjà, un nouvel
appui long l'efface. Le rappel des paramètres est automatique à la lecture du
pas. La mémoire est volatile et est perdue à l'extinction.

Pour les essais, vérifier au multimètre que chaque bouton est bien câblé entre
la broche et GND, et non vers 5 V : D11 pour REC, D12 pour START/STOP et D13
pour SHIFT. Une entrée non câblée ou maintenue à LOW sur D13 peut faire
interpréter les appuis D12 comme des taps tempo au lieu de START/STOP.

## ⚠️ Avertissement — horloge externe sur D2

D2 accepte une horloge logique protégée référencée à la masse Arduino. Chaque
front montant valide fait avancer d'un pas ; après deux fronts valides, la
période mesurée remplace l'horloge interne et synchronise aussi le clignotement.
Les périodes acceptées sont de 30 à 500 ms (environ 30–500 BPM en pas).

Ne jamais injecter directement une horloge Eurorack ±5 V ou ±10 V sur D2 : cela
peut endommager l'entrée de l'Arduino. Le circuit d'adaptation (limitation de
tension, protection contre l'inversion et mise en forme logique) reste à
concevoir et à valider. En attendant, utiliser uniquement un signal 0–5 V
protégé, avec GND commun, ou laisser D2 non câblée.

## MAX7219

VCC va au 5 V et GND à la masse commune. Prévoir au plus près du module au
minimum 100 nF entre 5 V et GND ; le guide V13 recommande aussi 100 µF de
réserve. Éviter de faire passer le retour MAX7219 par le chemin audio.

L'interface est montée avec une rotation de 180 degrés. Le firmware compense
cette rotation : l'affichage logique place l'instrument en colonne 0, les
bargraphes des trois paramètres en colonnes 2, 4 et 6, puis les pas 1–8 et
9–16 sur les deux lignes du bas, toujours lus de gauche à droite. La
correspondance finale doit être vérifiée sur le module réel.

La colonne 1 indique MUTE par un point fixe à côté de l'instrument sélectionné.
Le point de sélection en colonne 0 continue de clignoter lorsqu'un instrument
est muté. Le point en haut à droite indique que le transport fonctionne.
Après une action sur D11, le step édité est forcé brièvement allumé comme
confirmation visuelle.

## Audio et alimentation

Le code produit le PWM Mozzi sur D9 à 16 384 Hz et applique un filtrage
numérique léger. Si un buzz reste présent lorsque le MAX7219 est débranché ou
éteint, il ne provient pas de l'affichage : rechercher alors l'alimentation 9 V,
la masse audio, le filtre de sortie et les boucles de masse. Le filtre analogique, le condensateur de liaison, le niveau
de sortie, la protection et le connecteur Eurorack restent à mesurer et à
valider. Ne pas connecter D10 à l'audio de cette version.

### Filtre audio proposé pour le prototype

```text
D9 --- 270 ohms ---+--- 10 uF --- 1 kohm --- AUDIO OUT TIP
                   |
                 100 nF
                   |
                  GND
```

Après le condensateur de liaison, prévoir 100 kohms vers GND. Pour le
condensateur électrolytique, le positif va côté Arduino et le négatif côté
sortie. Ces valeurs sont une proposition de prototype et restent à valider au
mesure, notamment pour le niveau Eurorack.

## Alimentation prototype

Pour l'UNO, l'alimentation 9 V peut entrer par le jack ; le 5 V de l'UNO
alimente les potentiomètres et le MAX7219. Pour le Nano, utiliser VIN et GND.
Le montage final devra ajouter protection contre l'inversion, découplage et
retours de masse séparés entre LED et audio. La consommation du MAX7219 et la
régulation doivent être vérifiées avant intégration Eurorack.
