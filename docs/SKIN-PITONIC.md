# Skin πClideaN dans la famille πToniC

Évolution graphique demandée et maquettes affinées avec Greg le 8 novembre 2026. Générateur natif modifié localement ; aucune installation ni publication.

## Règles retenues

- Titillium Web Regular, même fichier de police que πToniC.
- Fonds : `#171c1f` ; champs : `#242c31` ; encarts/graphes : `#141d24` ; bordures : `#42515c` ; texte : `#f1f2ed`.
- Boutons : hauteur de police native 23. Listes et panneaux de choix : 20, légèrement inférieure aux boutons.
- Valeurs numériques : 16, comme la légende native « 4 BARS / ... BPM ». Titres des contrôles : 18. LINE / STEPS / STEP : 14.
- Les couleurs fonctionnelles restent distinctes : sélection rouge, verrous de pas violets, état actif ambre, sélection RAND / gamme en attente verte, activité azur. Les références LINE non éditables restent grisées en EDIT, sauf GATE par pas.

## PATTERN

Encarts de 304 × 122, espacés de 8 pixels, à x=20/332/644/956. Deux rangées à y=132/276 dans la zone plugin native (hors en-tête MPC). Suppression du cadre englobant et de sa ligne de séparation. Titres de section plus discrets.

Géométrie πToniC : filmstrip de potentiomètre 58 × 58 à x=12, centré verticalement ; titre à x=86/y=8 ; valeur à x=86/y=43 dans un champ de 210 × 42. La valeur utilise la petite police validée. La ligne de commandes est déplacée ensemble à y=410 ; les pas restent à leur emplacement y=470/534. Aucun chevauchement entre les rangées, commandes et pas.

La sélection RAND garde la même géométrie et le même angle de potentiomètre ; une seule cible tactile est active. Les titres sont des images statiques mises en cache, supprimant les interrogations natives Name correspondantes.

## Autres onglets

MOD conserve ses sliders, ses panneaux de choix, ses courbes statiques, son MIX et ses destinations. La valeur Hz est centrée sous son slider. Même palette et petites valeurs ; ligne centrale maintenue.

01–08 et 09–16 conservent leurs miniatures partagées, sélections de ligne, M/S et contrôles de parcours/autorotation. CONFIG conserve ses seize notes, GATE/HOLD, SCALES/START/APPLY/RESET et COUNT-IN.

## Garanties contrôlées hors appareil

- Sources du moteur, paramètres et indices identiques à l’état avant skin.
- Screen Mode Q-Link conservé ; variante 4by4 et Program Mode explicite ajoutés après audit. Actions natives et propriétaires des gestes conservés.
- Filmstrips des miniatures et courbes MOD partagés, dimensions et états inchangés.
- Aucun nouveau polling, traitement audio/MIDI ou fil de travail ; déduplication d’images conservée.
- Génération séparée dans `build/skin-pitonic`, sans écraser le skin de build précédent.

Test : `tests/skin_pitonic_test.py`. Preuves et aperçus du vrai arbre de composants dans `Validation/skin-pitonic/`.

Un essai sur Force reste nécessaire pour confirmer le rendu réel, le tactile, les Q-Links et l’absence de craquements. Aucun paquet ARM ni redémarrage n’est effectué par ce travail graphique.

Complément : [programmes PRESET natifs et audit Q-Link](NATIVE-PRESETS.md).

## Nom affiché

Le prochain candidat affiche **πClideaN**, sans le suffixe MPC. Le nom du contenu et son identifiant de skin sont générés à partir de cette même identité. Le UID VST `5069434e`, les indices de paramètres, le binaire et les noms des ports MIDI existants sont conservés pour éviter de modifier le routage des projets. Le paquet et les déclarations devront être régénérés ensemble avant installation.
