# Nano808

Prototype de boîte à rythmes Eurorack autour d'un Arduino Nano / ATmega328P.
Le firmware présent est la V13 : séquenceur 16 pas, six voix de percussion,
matrice MAX7219 et synthèse audio Mozzi.

## État du dépôt

Le firmware compile avec `arduino:avr:nano` et l'environnement installé
(arduino:avr 1.8.8). Cette compilation ne valide pas le son ni le câblage sur
matériel. Aucun téléversement automatique n'est effectué.

Le prototype utilise une horloge interne à 90 BPM. L'horloge externe sur D2 et
le swing sont préparés dans le code mais ne sont pas fonctionnels dans cette
V13.

## Fonctionnalités et commandes

- Six instruments : kick, snare, charleston fermé/ouvert, clap et tom/conga.
- Séquenceur 16 pas, longueur de boucle réglable de 1 à 16.
- Trois paramètres indépendants par instrument avec soft takeover.
- Affichage sur matrice 8×8 MAX7219 et sortie PWM Mozzi sur D9.
- D7 bascule le pas sélectionné ; D8 court déclenche FIRE ; D8 maintenu au
  moins 650 ms mute/unmute l'instrument pour la lecture séquencée.
- D11 bascule la variation du pas sélectionné ; D3 réinitialise au pas 1.

Les six potentiomètres sont raccordés entre 5 V et GND, curseur vers A0–A5.
Les boutons sont normalement ouverts entre leur broche et GND ; le firmware
active `INPUT_PULLUP`. FIRE continue de déclencher un instrument muté.

## Organisation

- `firmware/` : sketch embarqué V13 ;
- `docs/` : architecture et documents complémentaires ;
- `HARDWARE.md` : brochage, câblage et réserves matérielles ;
- `TODO.md` : tâches et validations ;
- `Nano808_V13_Hardware.md` : guide détaillé du prototype ;
- `Nano808_V13_CHANGELOG.md` : historique V13.

## Compilation

Le dossier de sketch doit porter le même nom que le fichier `.ino` selon la
version d'Arduino CLI. Le fichier V13 est actuellement directement dans
`firmware/`, donc il faut l'ouvrir dans l'IDE Arduino ou le placer dans un
dossier de sketch portant son nom avant d'exécuter :

```text
arduino-cli compile --fqbn arduino:avr:nano <dossier-du-sketch>
```

La compilation V13 a été effectuée de cette manière dans un dossier temporaire.
Le téléversement reste manuel et nécessite une autorisation explicite.

Voir [HARDWARE.md](HARDWARE.md), [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
et [TODO.md](TODO.md).
