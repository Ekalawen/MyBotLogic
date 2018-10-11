#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "MyBotLogic/MapTile.h"
#include <map>
using namespace std;

class MapTile;
class Map {
    void sortByDistance(vector<float>& base, vector<int>& autre1, vector<int>& autre2); // Permet de trier base dans l'ordre décroissant, et autre1 et autre2 dans le même ordre
public:
    int rowCount;
    int colCount;
    int nbTiles;
    int nbtilesDecouvertes;
    map<unsigned int, MapTile> tiles;
    map<unsigned int, MapTile> objectifs;
    map<unsigned int, ObjectInfo> murs;
    map<unsigned int, ObjectInfo> portes;
    map<unsigned int, ObjectInfo> fenetres;
    map<unsigned int, ObjectInfo> activateurs;

    Map() = default;
    Map(const LevelInfo);
    bool isInMap(int idTile) const;
    map<unsigned int, MapTile> getObjectifs();
    Chemin aStar(int depart, int arrivee); // Renvoie le chemin à parcourir pour aller du départ à l'arrivée
    float distanceL2(int depart, int arrivee); // Renvoie la distance L2 à vol d'oiseau !
    int distanceHex(int depart, int arrivee);
    bool areAccessible(int tile1, int tile2); // Permet de savoir si deux tiles sont accessibles l'une par rapport à l'autre ! pour 2 tiles adjacentes
    Tile::ETilePosition getDirection(int tile1, int tile2); // Permet de savoir dans quel sens se déplacer pour aller d'une tile à l'autre si celles-ci sont adjacentes ! =)
    int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction); // Permet de récupérer l'indice d'une tuile adjacente à une autre
    int tailleCheminMax(); // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo); // Permet de rajouter une tile à la map
    void addObject(ObjectInfo); // Permet de rajouter un object à la map
};


#endif
