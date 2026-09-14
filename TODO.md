# Avancement Nano808

Ne cocher une tâche que lorsqu'elle est réalisée. Une compilation ne constitue
pas une validation audio ou matérielle.

## Réalisé dans le dépôt

- [x] Documenter le firmware V13 et son organisation.
- [x] Documenter le brochage V13 connu et réserver D9 à l'audio.
- [x] Implémenter séquenceur 16 pas, longueur 1–16, FIRE, MUTE, REC/variation et RESET.
- [x] Améliorer la lecture des potentiomètres : plage A4/A5 calibrée, filtrage,
  hystérésis, confirmation des changements et atténuation de la sensibilité A1–A3.
- [x] Ajouter START/STOP sur D12 avec conservation de la position du séquenceur.
- [x] Enrichir légèrement les timbres kick, tom et snare sans traitement bloquant.
- [x] Implémenter la réception d'une horloge logique externe sur D2 et la mesure de sa période.
- [x] Protéger le mixage contre le clipping dur par une saturation progressive.
- [x] Donner aux hi-hats un caractère métallique plus riche par combinaison inharmonique.
- [x] Ajouter REC/clear des trois paramètres sonores par pas via un appui long sur D11.
- [x] Compiler avec `arduino:avr:nano` : Flash 11 154 / 30 720 octets (36 %), SRAM globale 983 / 2 048 octets (47 %).

## À confirmer sur matériel

- [ ] Confirmer le modèle exact du Nano et la version Mozzi de production.
- [ ] Tester le MAX7219, son découplage et les retours de masse.
- [ ] Mesurer le filtre audio D9, le niveau Eurorack et le bruit avec matrice active.
- [ ] Confirmer alimentation 9 V, régulation 5 V, protections et connecteur Eurorack.
- [ ] Vérifier orientation de matrice et mesurer les extrémités ADC réelles des potentiomètres.
- [ ] Tester anti-rebond, START/STOP, FIRE court/long, MUTE, REC, RESET et longueur.
- [ ] Vérifier stabilité audio pendant séquenceur, affichage et commandes.
- [ ] Réaliser un essai prolongé et consigner durée et résultats.

## Évolutions

- [x] Ajouter SHIFT sur D13 et tap tempo via SHIFT + START/STOP.
- [ ] Ajouter la sélection de forme d'onde et son affichage sur un écran secondaire.
- [ ] ameliorer la selection de step: pot trop sensible, division de la course incertaine.
- [ ] definir des modes de synthese plus adaptés aux instruments (ajout FM, redefinition des enveloppes)
- [x] protéger contre le clipping (saturation)
- [x] recherche d'une synthèse plus fidèle pour le hi-hat : composante inharmonique ajoutée
- [ ] ajout d'un bouton shift et acces à de nouveles fonctions: tap tempo/ forme d'onde (à materialiser graphiquement sur un ecran secondaire)
- [x] ajout d'une fonction REC/clear pour enregistrer les paramètres sonores propres à un step
- [ ] evaluer memoire disponible pour enregistrer les beat, ajout d'une fonction pour parcourir les beats enregistrés
- [ ] Définir et valider le circuit de protection de l'entrée CLOCK externe D2.
- [ ] Ajouter une commande de swing après validation de l'interface V13.
- [ ] Finaliser schémas, valeurs de composants et instructions de montage.
- [x] Documenter le téléversement manuel après choix du modèle de carte.

## Vérifications effectuées

Compilation Arduino CLI du 14/09/2026 réussie avec `arduino:avr:nano` et
arduino:avr 1.8.8 : 11 154 octets de Flash et 983 octets de SRAM globale.
Aucun téléversement et aucun essai audio sur matériel n'ont été effectués.
