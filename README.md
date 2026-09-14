# Nano808

Prototype de boîte à rythmes Eurorack autour d'un Arduino Nano / ATmega328P.
Le firmware présent est la V13 : séquenceur 16 pas, six voix de percussion,
matrice MAX7219 et synthèse audio Mozzi.

## État du dépôt

Le firmware compile avec `arduino:avr:nano` et l'environnement installé
(arduino:avr 1.8.8). Cette compilation ne valide pas le son ni le câblage sur
matériel. Aucun téléversement automatique n'est effectué.

Le prototype utilise une horloge interne à 90 BPM. Une horloge logique externe
peut maintenant être appliquée sur D2 : chaque front montant avance d'un pas
et la période mesurée remplace l'horloge interne. La protection électrique de
cette entrée reste à valider. Le swing est préparé dans le code mais aucune
commande physique ne lui est encore affectée.

**Avertissement : ne jamais brancher directement une horloge Eurorack ±5 V ou
±10 V sur D2.** Utiliser uniquement un signal logique 0–5 V protégé avec masse
commune ; l'adaptation électrique reste à concevoir et valider.

## Fonctionnalités et commandes

- Six instruments : kick, snare, charleston fermé/ouvert, clap et tom/conga.
- Séquenceur 16 pas, longueur de boucle réglable de 1 à 16.
- Trois paramètres indépendants par instrument avec soft takeover.
- Affichage sur matrice 8×8 MAX7219 et sortie PWM Mozzi sur D9.
- D7 bascule le pas sélectionné ; D8 court déclenche FIRE ; D8 maintenu au
  moins 650 ms mute/unmute l'instrument pour la lecture séquencée.
- D11 ouvre/ferme le mode d'édition du pas ; D3 réinitialise au pas 1 ; D12
  bascule START/STOP.
- D11 maintenu environ 650 ms enregistre les trois paramètres sur le pas ; si
  un enregistrement existe, il est effacé.
- Chaque appui sur D13 règle le tap tempo ; D12 reste exclusivement START/STOP.

La matrice est montée avec un demi-tour (180°), compensé par le firmware ; les
steps restent lisibles de gauche à droite. La colonne voisine de l'instrument
indique MUTE, et le coin supérieur droit indique que le transport tourne.

Les six potentiomètres sont raccordés entre 5 V et GND, curseur vers A0–A5.
Les cinq boutons sont normalement ouverts entre leur broche et GND ; le firmware
active `INPUT_PULLUP`. FIRE continue de déclencher un instrument muté.

## Organisation

- `firmware/` : sketch embarqué V13 ;
- `docs/` : architecture et documents complémentaires ;
- `HARDWARE.md` : brochage, câblage et réserves matérielles ;
- `TODO.md` : tâches et validations ;
- `Nano808_V13_CHANGELOG.md` : historique V13.

## Compilation

Le dossier de sketch doit porter le même nom que le fichier `.ino` selon la
version d'Arduino CLI. Le fichier V13 est actuellement directement dans
`firmware/`, donc il faut l'ouvrir dans l'IDE Arduino ou le placer dans un
dossier de sketch portant son nom avant d'exécuter :

```text
arduino-cli compile --fqbn arduino:avr:nano <dossier-du-sketch>
``

La compilation V13 a été effectuée de cette manière dans un dossier temporaire.
Le téléversement reste manuel et nécessite une autorisation explicite.

Voir [HARDWARE.md](HARDWARE.md), [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
et [TODO.md](TODO.md). `HARDWARE.md` est la référence unique du brochage et du
câblage.
