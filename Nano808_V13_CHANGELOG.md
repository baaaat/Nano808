# Nano808 V13 — changelog

## Base conservée

La V13 part de `Nano808_Prototype_90BPM_V12_EqualStepZones.ino`.

Conservé sans changement de philosophie :
- synthèse Kick / Snare / CHH / OHH / Clap / Tom ;
- tempo interne de test 90 BPM ;
- MAX7219 ;
- orientation de matrice validée ;
- steps gauche vers droite ;
- A4 avec 16 zones régulières et stabilisation ;
- trois bargraphs en colonnes 2 / 4 / 6 ;
- instrument en colonne 0 ;
- réglages propres à chaque instrument ;
- soft takeover au changement d'instrument ;
- aucun son lors du simple changement d'instrument ;
- filtre numérique audio léger de la version précédente.

## Nouveautés

### FIRE
D8 devient officiellement FIRE.
- appui court : déclenche immédiatement l'instrument sélectionné ;
- pas besoin d'attendre le séquenceur.

### MUTE
Appui long sur FIRE (~650 ms).
- mute uniquement la lecture automatique de l'instrument ;
- FIRE continue à permettre l'écoute ;
- le point de l'instrument clignote quand il est muté.

### LENGTH
Ajout d'un sixième potentiomètre sur A5.
- longueur globale 1 à 16 steps ;
- hystérésis + léger lissage ;
- réduction de longueur non destructive : les steps cachés restent mémorisés.

### REC / VARIATION
Ajout d'un bouton sur D11.
- un bit de variation par instrument et par step ;
- 6 instruments × 16 bits = 12 octets ;
- différence de timbre légère, adaptée à chaque percussion ;
- indépendant du bouton STEP ON/OFF.

### RESET
D3 peut recevoir un bouton poussoir vers GND.
- retour immédiat au step 1 ;
- resynchronisation du clignotement ;
- step 1 rejoué immédiatement.

### Clock variable préparé
Le code n'utilise plus une constante de clignotement 333333 µs.
Le clignotement est calculé à partir de `beatPeriodUs`.

La fonction `setTimingFromExternalStepPeriod()` est prête pour la future carte CLOCK sur D2.

### Swing préparé
`stepDurationUs()` permet déjà un décalage swing.
V13 laisse `swingPercent = 0` et ne lui affecte pas encore de contrôle physique.

## Hardware ajouté

- 1 × pot 10 kΩ linéaire pour LENGTH sur A5 ;
- 1 × bouton FIRE sur D8 ;
- 1 × bouton REC sur D11 ;
- 1 × bouton RESET sur D3 ;
- bouton STEP existant sur D7.

Tous les boutons sont normalement ouverts et vont simplement de leur broche Arduino vers GND grâce à `INPUT_PULLUP`.

## Vérifications faites

- accolades équilibrées ;
- références des anciennes variables de clock supprimées ;
- `Voice` reste déclaré en amont pour éviter l'erreur du préprocesseur Arduino déjà rencontrée ;
- vérification syntaxique C++ effectuée avec des stubs Arduino/Mozzi.

Une compilation finale dans l'IDE Arduino avec la version exacte de Mozzi installée sur le PC reste le test de référence.
