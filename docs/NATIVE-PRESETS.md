# Presets natifs et audit Q-Link — 8 novembre 2026

Implémentation locale, reprise de la méthode validée dans πToniC. Cette note décrit l’étape initiale du 8 novembre. Des candidats ARM locaux sont désormais construits ; aucune installation de ces évolutions ni validation de leur menu sur Force.

## Menu PRESET natif

Le wrapper expose sept vrais programmes VST, sans ajouter de paramètre ni d’onglet. Le menu PRESET de MPC/Force doit les énumérer à l’ouverture du plugin ; un simple dossier de XPL ne réalise pas cette énumération.

| Programme | Contenu |
| --- | --- |
| INIT | Seize lanes vides, notes 36–51, valeurs usuelles, GATE, MOD OFF. Aucun Note On de motif tant qu’aucun pas n’est créé. |
| Straight Four | Trois lignes régulières, longueurs de seize pas. |
| Broken Grid | Quatre motifs euclidiens de densités et rotations différentes. |
| Polymeter | Quatre longueurs indépendantes : 10, 7, 12 et 16 pas. |
| Dorian Flow | Affectation ascendante en Do dorien, huit lignes espacées. |
| Slow Orbit | Quatre cycles lents avec autorotation. |
| Step Expression | Exemple de vélocité, GATE et ratchet personnalisés par pas. |

Tous les LFO sont OFF dans la banque. Les recettes sont originales et MIT ; aucune banque Microtonic/Patternarium n’est utilisée. `presets/factory.json` contient les états complets et leurs noms. Le générateur produit `build/picn_factory_programs.h` ; les sources JSON entrent dans les reçus de compilation.

Le programme zéro est chargé explicitement à la création. Choisir un autre programme charge l’état complet, ferme les panneaux transitoires et réutilise les notifications différées existantes. La banque est compilée en lecture seule : aucun fichier ni réseau n’est consulté pendant la lecture ou le rappel.

## Projets et presets utilisateur

Les réglages restent sauvegardables par le menu natif. Le chunk contient :

```
PCPROG1 <index>
<état complet PICN, avec les modifications utilisateur>
```

Au rappel d’un projet, l’état édité est restauré, pas la recette d’usine. Le réécho du programme courant est ignoré pour ne pas écraser ces modifications. Pour revenir volontairement à sa recette d’origine, choisir un autre programme puis revenir au premier.

Les anciens chunks PICN bruts sont acceptés. Un ancien index de banque devenu indisponible conserve l’état complet et utilise l’index zéro comme repli. Les données invalides sont rejetées sans appliquer un état partiel. Les anciens binaires ne comprennent pas l’enveloppe PCPROG1 : conserver les projets originaux lors d’un futur essai de cette évolution.

## Q-Links

Trois déclarations explicites : `Q-Links.json`, `Q-Links - 8by1.json` et `Q-Links - 4by4.json`.

Ordre physique gauche → droite sur Force : 13, 9, 5, 1, 14, 10, 6, 2. Les huit autres cases sont -1.

- PATTERN : q0–q7 contextuels LINE/EDIT. EDIT protège les trois références LINE et affecte q3 au GATE du pas.
- MOD : Depth, Offset, Phase, Threshold du LFO 1, puis du LFO 2, pour l’emplacement MOD sélectionné.
- 01–08 / 09–16 : sélection de lane, mode, autorotation, shift et intervalle. Trois cases libres.
- CONFIG : pas d’affectation Q-Link dédiée ajoutée ; édition tactile/molette conservée.
- Program Mode : les huit alias PATTERN, explicitement déclarés. L’ancienne déclaration héritée du skin de référence pointait vers des indices sans rapport avec ce contexte ; elle est remplacée.

Les affectations Screen Mode existantes sont conservées à l’identique. La variante 4by4 et la définition explicite Program Mode sont les corrections apportées par cet audit.

## Performances et preuves

Aucune modification des sources du moteur MIDI. La banque n’est traitée qu’à la création, à un rappel ou à une sauvegarde. Le budget de notifications (huit par bloc pour le projet) et la suppression des reconstructions inutiles ne sont pas changés. Les images statiques et les atlas partagés du nouveau skin sont conservés.

Tests PC sous ASan/UBSan : `scripts/test_native_programs.sh`, suites UI/MOD, décompte et continuité polymétrique. Contrôles de skin et des trois déclarations : `tests/skin_pitonic_test.py`. Tests sur le vrai wrapper et le vrai parseur d’état, avec création hors appareil.

À confirmer sur Force : liste PRESET visible, lecture des exemples, sauvegarde/réouverture d’un preset modifié, et affectations réellement modifiées par les huit encodeurs dans chaque contexte. Un test PC ne qualifie pas le tactile ni les écrans matériels des Q-Links.
