#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "Npc.h"
#include "MyBotLogic/MapTile.h"
#include <map>
using namespace std;

class MapTile;
class Npc;
class Map {
    public:
    int rowCount;
    int colCount;
    int nbTiles;
    int nbtilesDecouvertes;
    vector<MapTile> tiles;

    vector<unsigned int> objectifs;
    map<unsigned int, ObjectInfo> murs;
    map<unsigned int, ObjectInfo> portes;
    map<unsigned int, ObjectInfo> fenetres;
    map<unsigned int, ObjectInfo> activateurs;

    Map() = default;
    Map(const LevelInfo);
    bool isInMap(int idTile) const noexcept;
    vector<unsigned int> getObjectifs() const noexcept;

    Chemin aStar(int depart, int arrivee, float coefEvaluation = 1) noexcept; // Renvoie le chemin à parcourir pour aller du départ à l'arrivée

    float distanceL2(int depart, int arrivee) const noexcept; // Renvoie la distance L2 à vol d'oiseau !
    int distanceHex(int depart, int arrivee) const noexcept;
    Tile::ETilePosition getDirection(int tile1, int tile2) const noexcept; // Permet de savoir dans quel sens se déplacer pour aller d'une tile à l'autre si celles-ci sont adjacentes ! =)
    int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) const noexcept; // Permet de récupérer l'indice d'une tuile adjacente à une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo) noexcept; // Permet de rajouter une tile à la map
    void addObject(ObjectInfo) noexcept; // Permet de rajouter un object à la map

    int getX(int id) const noexcept; // Permet de récupérer x et y à partir d'un indice
    int getY(int id) const noexcept;
    vector<int> getVoisins(int id) const noexcept;
};


#endif
