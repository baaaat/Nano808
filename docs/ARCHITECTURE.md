# Architecture du firmware V13

Le sketch utilise Mozzi : `loop()` appelle `audioHook()`, qui alimente le
chemin audio, tandis que `updateControl()` traite périodiquement le séquenceur,
les potentiomètres, les boutons et l'affichage.

## Chemins principaux

- `updateAudio()` synthétise les six voix, les mélange, limite le niveau,
  applique un filtre numérique léger et renvoie un signal mono 16 bits vers le
  PWM Mozzi.
- Chaque instrument possède une `Voice` et un masque de pattern 16 bits.
  `variationPattern` ajoute un bit par pas et par instrument ;
  `instrumentMuted` ne bloque que la lecture automatique.
- L'horloge interne est fixée à 90 BPM, avec quatre pas par temps.
  `stepDurationUs()` et `setTimingFromExternalStepPeriod()` centralisent la
  durée du séquenceur et du clignotement.
- Les lectures analogiques sont filtrées et protégées par hystérésis. Les
  paramètres utilisent un soft takeover au changement d'instrument.
- Les boutons sont lus avec un anti-rebond de 20 ms. RESET pose un drapeau dans
  l'interruption ; le traitement est effectué ensuite dans `updateControl()`.

## Limites connues

Patterns et paramètres sont en RAM et sont perdus à la mise hors tension. D2
est seulement réservée : aucune horloge externe n'est traitée. La compilation
ne permet pas de conclure à la stabilité audio, au niveau de sortie ou à la
compatibilité électrique Eurorack.
