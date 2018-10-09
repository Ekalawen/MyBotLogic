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
    int x, y; // La position de la tile
    Tile::ETileType type;
    vector<int> voisins;
    vector<int> voisinsAccessibles;

    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(const TileInfo, int rowCount, int colCount);
    void setVoisins(Map m);
};


#endif
