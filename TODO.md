# Avancement Nano808

Ne cocher une tâche que lorsqu'elle est réalisée. Une compilation ne constitue
pas une validation audio ou matérielle.

## Réalisé dans le dépôt

- [x] Documenter le firmware V13 et son organisation.
- [x] Documenter le brochage V13 connu et réserver D9 à l'audio.
- [x] Implémenter séquenceur 16 pas, longueur 1–16, FIRE, MUTE, REC/variation et RESET.
- [x] Compiler avec `arduino:avr:nano` : Flash 11 154 / 30 720 octets (36 %), SRAM globale 983 / 2 048 octets (47 %).

## À confirmer sur matériel

- [ ] Confirmer le modèle exact du Nano et la version Mozzi de production.
- [ ] Tester le MAX7219, son découplage et les retours de masse.
- [ ] Mesurer le filtre audio D9, le niveau Eurorack et le bruit avec matrice active.
- [ ] Confirmer alimentation 9 V, régulation 5 V, protections et connecteur Eurorack.
- [ ] Vérifier orientation de matrice et zones des potentiomètres.
- [ ] Tester anti-rebond, FIRE court/long, MUTE, REC, RESET et longueur.
- [ ] Vérifier stabilité audio pendant séquenceur, affichage et commandes.
- [ ] Réaliser un essai prolongé et consigner durée et résultats.

## Évolutions

- [ ] Définir et protéger l'entrée CLOCK externe sur D2.
- [ ] Ajouter une commande de swing après validation de l'interface V13.
- [ ] Finaliser schémas, valeurs de composants et instructions de montage.
- [ ] Documenter le téléversement manuel après choix du modèle de carte.

## Vérifications effectuées

Compilation Arduino CLI du 14/09/2026 réussie avec `arduino:avr:nano` et
arduino:avr 1.8.8 : 11 154 octets de Flash et 983 octets de SRAM globale.
Aucun téléversement et aucun essai audio sur matériel n'ont été effectués.
