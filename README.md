# Pathindor

**C++17 / raylib 5.5 · S1P2 Pathfinding · GTECH 2**

Ce projet consiste à appliquer des méthodes de pathinding dans le cadre d'un jeu vidéo de type *hack and slash*.

## Résumé des fonctionnalités

- Dijkstra et A*
- Composant NavMesh : polygones navigables, localisation, graphe, adjacences et portails
- Requête de chemin et conversion en trajet géométrique
- Suivi de chemin, recalcul et gestion des échecs
- Perception, décisions et comportements ennemis
- Combat, vie, dégâts, mort, vagues et fin de partie

Le joueur peut lancer des drones explosifs sur des ennemies.

Les ennemies sont de trois types :
- archer (lance des projectiles sur le joueur)
- light (patrouille la carte, et attaque le joueur au contact)
- heavy (reste immobile, chasse le joueur et l'attaque au contact)

## Nécessaire

Rien, récupérer la release, et lancer le .exe

## Commandes

| Commande | Action fournie |
|---|---|
| Flèches, ZQSD ou WASD | Déplacer le joueur avec collisions physiques |
| F1 | Afficher les surfaces sources ou le NavMesh construit par votre code |
| F2 | Afficher les chemins produits par votre code |
| F3 | Afficher ou masquer le HUD |
| Tab | Choisir Dijkstra ou A* pour les requêtes |
| Clic gauche sur le sol | Appeler NavMesh::FindPath depuis le joueur |
| Clic droit sur une entité ou N | Sélectionner un marqueur ennemi |
| F5 | Changer de scène |
| R | Réinitialiser les positions de la scène |
| P | Mettre la simulation en pause |
| Espace | Lancer un drone explosif |
| Échap | Fermer |

Le clic de sonde ne déplace pas le joueur. La sonde est recalculée au clic ou au changement d'algorithme.

## Conditions de victoire / défaite

Le joueur gagne s'il parvient à terasser les 3 vagues sans mourir.
Le joueur perd si ses points de vie atteigne 0.

## Licence et vérification

Le code propre au socle est sous licence MIT. raylib conserve sa licence zlib/libpng et les licences de ses dépendances dans external/raylib.
