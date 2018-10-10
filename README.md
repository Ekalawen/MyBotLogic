# MyBotLogic
Les fichiers client du projet d'IA de l'UdeS.


Alors je vais faire un petit effort de documentation pour que on puisse tous s'y retrouver rapidement =)

GameManager : C'est dans cette classe que se trouve l'ensemble de la modélisation du problème d'IA.
Elle contient pour le moment une map (l'ensemble des hexagones), les npcs et les différents objets présents sur la map tel que les objectifs, les portes etc ...
Le GameManager possède également une méthode statique nommé GameManager::Log(string message) permettant de débugger facilement notre programme depuis n'importe où =)

Map : C'est la map qui est contenu dans le GameManager, elle possède l'ensemble des Tiles (les petits hexagones), la taille de la map (en hauteur et largeur).
Une map contient également bon nombre de fonctions qui vous seront très certainement utiles, allant de fonction renvoyant un chemin d'une case de départ à une case d'arrivée, des fonctions de distances entre 2 cases, des fonctions permettant d'obtenir la directions de 2 cases adjacentes, etc ...

MapTile : Ce sont les Tiles qui sont contenues dans la map. Pourquoi pas Tile me direz-vous ? Parce que c'était déjà pris voilà voilà :D
Lorsque l'on crée une Tile il faut appeller la fonction setVoisins qui nous permettra par la suite d'accèder directement aux voisins de cette Tile.
Il y a 2 listes de voisins, les voisins, et les voisins accessibles. Les premiers sont adjacents à notre Tile, les seconds aussi mais un bot peut en plus passer de notre tile à une de ses tuiles accessibles.

MyBotLogic : C'est là que l'on fera appel aux fonctions de niveau de raffinage R1 de notre code, tel que l'initialisation du GameManager, l'association des Npcs avec leurs objectifs et la fonction MoveNpcs.

Npc : C'est un Npc ! Il possède une position (définit par l'id de la Tile sur laquelle ils sont), un objectif (définit par l'id de la Tile sur laquelle ils vont) et leur chemin actif actuellement ! =)

Chemin : Permet de définir un vecteur qui est l'ensemble des identifiants des cases que doit suivre un Npc pour aller à la destination de ce chemin.
La classe peut sans aucun doute être complété.

Voilà ! Si vous avez des questions n'hésitez pas =)
