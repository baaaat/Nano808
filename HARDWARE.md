# Matériel et brochage Nano808

Ce document centralise le brochage et le câblage du module. Les affectations indiquées « à définir » ne constituent pas des connexions validées.

## Matériel

| Élément | État ou choix |
| --- | --- |
| Format | Eurorack ; dimensions à définir |
| Microcontrôleur | Arduino Nano ; modèle exact à confirmer |
| Audio | Mozzi Hi-Fi sur D9/D10 ; version et configuration à confirmer |
| Séquenceur | 16 pas ; commandes et affichage à définir |
| Boutons | START/STOP et FUNCTION/SHIFT |
| Alimentation | Connecteur, rails utilisés et régulation à définir |
| Interfaces Eurorack | Connecteurs, niveaux électriques et protections à définir |

## Brochage

Documenter ici toute connexion matérielle ajoutée au projet. Compléter l'inventaire des broches utilisées, libres et réservées lorsque le modèle du Nano sera confirmé.

| Fonction | Broche Nano | Direction côté Nano | Câblage | Remarques |
| --- | --- | --- | --- | --- |
| Audio Mozzi Hi-Fi | D9 | Sortie | Réseau de sortie à définir | Réservée à l'audio |
| Audio Mozzi Hi-Fi | D10 | Sortie | Réseau de sortie à définir | Réservée à l'audio |
| START/STOP | À définir | Entrée | Polarité et résistance de rappel à définir | Commande de transport |
| FUNCTION/SHIFT | À définir | Entrée | Polarité et résistance de rappel à définir | Comportement à préciser |

## Sortie audio

- Documenter le schéma de combinaison des sorties D9/D10 et les valeurs des composants.
- Définir le filtrage, le couplage et l'adaptation du niveau de sortie au système Eurorack.
- Documenter le connecteur de sortie, la masse et les protections retenues.
- Confirmer la compatibilité du montage avec le modèle du Nano et la configuration Mozzi choisis.

Le schéma de sortie reste à concevoir et à valider sur matériel.

## Commandes et indicateurs

- Définir les commandes de programmation des 16 pas et leur brochage.
- Définir les indicateurs éventuels et leur câblage.
- Préciser le comportement des appuis sur START/STOP et FUNCTION/SHIFT.
- Documenter la stratégie d'anti-rebond après validation.

## Entrées/sorties complémentaires

Les éventuelles entrées ou sorties d'horloge, de synchronisation ou de contrôle restent à décider. Aucune affectation de broche n'est définie pour ces fonctions.

## Alimentation et intégration

Documenter le connecteur d'alimentation, son orientation, les tensions utilisées, la régulation, les masses, le découplage et les protections. Ajouter les schémas dans `docs/` et les référencer ici lorsqu'ils seront disponibles.
