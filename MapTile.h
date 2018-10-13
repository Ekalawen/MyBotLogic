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
	int NE, E, SE, SW, W, NW;
    vector<int> voisins; // les identifiants des voisins de la tuile
	vector<int> voisinsAccessibles; // les voisins connus et accessible (pas de murs ni de fenêtres)
	vector<int> voisinsVisibles; // les voisins visibles (contient les voisins accessibles et les voisins fenetres)
	vector<int> voisinsObscurs; // les voisins sur lequel on a pas encore d'information

    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(const TileInfo, int rowCount, int colCount);
    void setVoisins(Map m);
    int getVoisinByDirection(Tile::ETilePosition direction); // Permet de récupérer le voisin dans une certaine direction d'une tile
	bool isVoisinAccessible(Tile::ETilePosition direction);
	bool isVoisinAccessible(int id);
	bool isVoisinVisible(int id);
	bool isVoisinObscur(int id);
	vector<int> getVoisinFenetres();
};

#endif
