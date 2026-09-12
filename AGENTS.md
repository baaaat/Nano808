# Consignes de travail — Nano808

## Contexte

Projet Eurorack sur Arduino Nano, de la même famille que Mutant et Freak : séquenceur 16 pas, sortie Mozzi Hi-Fi D9/D10, boutons START/STOP et FUNCTION/SHIFT. Ne pas déduire de caractéristiques supplémentaires de cette parenté.

## Priorités de développement

- Préserver la stabilité audio lors de toute modification.
- Éviter les attentes et traitements bloquants dans les chemins sensibles à l'audio ; garder les traitements audio courts et prévisibles.
- Tenir compte de l'impact des commandes, de l'anti-rebond, du séquenceur et des indicateurs sur le fonctionnement audio.
- Ne pas réaffecter D9/D10 sans décision explicite de l'utilisateur.
- Adapter les choix techniques au modèle de Nano et aux versions des dépendances effectivement retenus.

## Documentation et suivi

- Maintenir HARDWARE.md à jour pour chaque changement de brochage ou de câblage, avec fonction, broche, direction, câblage et remarques utiles.
- Mettre à jour TODO.md lors des avancées et des validations ; ne cocher que les tâches réalisées.
- Mettre à jour README.md lorsque la prise en main ou l'organisation du projet change.
- Distinguer les informations confirmées des hypothèses ; indiquer « à définir » ou « à confirmer » pour les éléments inconnus.
- Placer le code embarqué dans `firmware/` et les schémas ou documents complémentaires dans `docs/`.

## Vérification

- Effectuer les vérifications adaptées à la modification et à l'environnement disponible.
- Rapporter les vérifications réellement effectuées et leurs résultats.
- Distinguer une compilation réussie d'une validation audio sur matériel.
- Indiquer les essais matériels restant à réaliser, particulièrement après une modification de l'audio, du séquenceur ou des commandes.
