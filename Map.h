#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "Npc.h"
#include "MyBotLogic/MapTile.h"
#include <map>
#include <vector>
#include "Porte.h"

class tile_inexistante {};
class porte_inexistante {};

class MapTile;
class Npc;
class Porte;
class Map {
    int rowCount;
    int colCount;
    int nbTilesDecouvertes;
    std::vector<MapTile> tiles;

    std::vector<unsigned int> objectifs;

    std::map<unsigned int, ObjectInfo> murs;
    std::map<int, Porte> portes;
    std::map<unsigned int, ObjectInfo> fenetres;
    std::map<unsigned int, ObjectInfo> activateurs;

public:
    Map() = default;
    Map(const LevelInfo&);
    bool isInMap(const int idTile) const noexcept;
    std::vector<unsigned int> getObjectifs() const noexcept;

    Chemin aStar(const int depart, const int arrivee, const float coefEvaluation = 1) const noexcept; // Renvoie le chemin à parcourir pour aller du départ à l'arrivée

    float distanceL2(const int depart, const int arrivee) const noexcept; // Renvoie la distance L2 à vol d'oiseau !
    int distanceHex(const int depart, const int arrivee) const noexcept;
    Tile::ETilePosition getDirection(const int tile1, const int tile2) const noexcept; // Permet de savoir dans quel sens se déplacer pour aller d'une tile à l'autre si celles-ci sont adjacentes ! =)
    int Map::getAdjacentTileAt(const int tileSource, const Tile::ETilePosition direction) const noexcept; // Permet de récupérer l'indice d'une tuile adjacente à une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(const TileInfo&) noexcept; // Permet de rajouter une tile à la map
    void addObject(const ObjectInfo&) noexcept; // Permet de rajouter un object à la map

    int getX(const int id) const noexcept; // Permet de récupérer x et y à partir d'un indice
    int getY(const int id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(const int id);
    const MapTile& getTile(const int id) const;

    std::vector<unsigned int> getObjectifs();
    std::map<unsigned int, ObjectInfo> getMurs();
    std::map<int, Porte> getPortes();
    Porte getPorte(const int id) const noexcept;
    Porte& getPorte(const int tileIdVoisine1, const int tileIdVoisine2) noexcept;
    std::map<unsigned int, ObjectInfo> getFenetres();
    std::map<unsigned int, ObjectInfo> getActivateurs();

    bool objectExist(const int id) const noexcept; // Permet de savoir si un objet existe déjà ou pas
};


#endif
