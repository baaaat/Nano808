# Nano808

Nano808 est un projet de module Eurorack basé sur Arduino Nano, de la même famille que Mutant et Freak. Les caractéristiques communes à ces projets restent à documenter.

## Fonctionnalités prévues

- Séquenceur 16 pas.
- Sortie audio Mozzi Hi-Fi sur D9/D10.
- Bouton START/STOP.
- Bouton FUNCTION/SHIFT, dont les fonctions détaillées restent à définir.

La stabilité audio est la priorité du projet, y compris pendant le fonctionnement du séquenceur et les interactions avec les commandes.

## État actuel

Le dépôt contient la documentation initiale. Les dossiers `firmware/` et `docs/` sont encore vides ; aucun firmware ni montage n'est validé à ce stade.

## Organisation

- `firmware/` : code embarqué à développer.
- `docs/` : schémas et documentation complémentaire à ajouter.
- `README.md` : présentation et prise en main.
- `HARDWARE.md` : référence matérielle et brochage complet.
- `TODO.md` : suivi des tâches et validations.
- `AGENTS.md` : consignes de travail sur le dépôt.

## Compilation et installation

À définir avant la première compilation :

- modèle exact de l'Arduino Nano et configuration de carte ;
- environnement de développement et version du cœur Arduino ;
- version de Mozzi et configuration du mode Hi-Fi ;
- dépendances éventuelles et emplacement du sketch ;
- procédure de compilation et de téléversement.

Consulter [HARDWARE.md](HARDWARE.md) pour les choix matériels et [TODO.md](TODO.md) pour l'avancement.
