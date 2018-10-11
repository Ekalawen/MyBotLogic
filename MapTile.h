#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Map.h"
#include <vector>
using namespace std;

class Map;
class MapTile {
public:
    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
    vector<int> voisins;
    vector<int> voisinsAccessibles;
    int NE, E, SE, SW, W, NW;

    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(const TileInfo, int rowCount, int colCount);
    void setVoisins(Map m);
    int getVoisinByDirection(Tile::ETilePosition direction); // Permet de récupérer le voisin dans une certaine direction d'une tile

};


#endif
