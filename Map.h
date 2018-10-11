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
    void sortByDistance(vector<float>& base, vector<int>& autre1, vector<int>& autre2); // Permet de trier base dans l'ordre d�croissant, et autre1 et autre2 dans le m�me ordre
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
    Chemin aStar(int depart, int arrivee); // Renvoie le chemin � parcourir pour aller du d�part � l'arriv�e
    float distanceL2(int depart, int arrivee); // Renvoie la distance L2 � vol d'oiseau !
    int distanceHex(int depart, int arrivee);
    bool areAccessible(int tile1, int tile2); // Permet de savoir si deux tiles sont accessibles l'une par rapport � l'autre ! pour 2 tiles adjacentes
    Tile::ETilePosition getDirection(int tile1, int tile2); // Permet de savoir dans quel sens se d�placer pour aller d'une tile � l'autre si celles-ci sont adjacentes ! =)
    int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction); // Permet de r�cup�rer l'indice d'une tuile adjacente � une autre
    int tailleCheminMax(); // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo); // Permet de rajouter une tile � la map
    void addObject(ObjectInfo); // Permet de rajouter un object � la map
};


#endif
