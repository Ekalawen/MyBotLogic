# MyBotMogic
Les fichiers client du projet d'IA de l'UdeS.


Alors je vais faire un petit effort de documentation pour que on puisse tous s'y retrouver rapidement =)

GameManager : C'est dans cette classe que se trouve l'ensemble de la mod�lisation du probl�me d'IA.
Elle contient pour le moment une map (l'ensemble des hexagones), les npcs et les diff�rents objets pr�sents sur la map tel que les objectifs, les portes etc ...
Le GameManager poss�de �galement une m�thode statique nomm� GameManager::Log(string message) permettant de d�bugger facilement notre programme depuis n'importe o� =)

Map : C'est la map qui est contenu dans le GameManager, elle poss�de l'ensemble des Tiles (les petits hexagones), la taille de la map (en hauteur et largeur).
Une map contient �galement bon nombre de fonctions qui vous seront tr�s certainement utiles, allant de fonction renvoyant un chemin d'une case de d�part � une case d'arriv�e, des fonctions de distances entre 2 cases, des fonctions permettant d'obtenir la directions de 2 cases adjacentes, etc ...

MapTile : Ce sont les Tiles qui sont contenues dans la map. Pourquoi pas Tile me direz-vous ? Parce que c'�tait d�j� pris voil� voil� :D
Lorsque l'on cr�e une Tile il faut appeller la fonction setVoisins qui nous permettra par la suite d'acc�der directement aux voisins de cette Tile.
Il y a 2 listes de voisins, les voisins, et les voisins accessibles. Les premiers sont adjacents � notre Tile, les seconds aussi mais un bot peut en plus passer de notre tile � une de ses tuiles accessibles.

MyBotLogic : C'est l� que l'on fera appel aux fonctions de niveau de raffinage R1 de notre code, tel que l'initialisation du GameManager, l'association des Npcs avec leurs objectifs et la fonction MoveNpcs.

Npc : C'est un Npc ! Il poss�de une position (d�finit par l'id de la Tile sur laquelle ils sont), un objectif (d�finit par l'id de la Tile sur laquelle ils vont) et leur chemin actif actuellement ! =)

Chemin : Permet de d�finir un vecteur qui est l'ensemble des identifiants des cases que doit suivre un Npc pour aller � la destination de ce chemin.
La classe peut sans aucun doute �tre compl�t�.

Voil� ! Si vous avez des questions n'h�sitez pas =)