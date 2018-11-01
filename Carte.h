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

class tile_inexistante {};

class MapTile;
class Npc;
class Carte {
    int rowCount;
    int colCount;
    int nbTilesDecouvertes;
    std::vector<MapTile> tiles;
    std::vector<unsigned int> objectifs;
    std::map<unsigned int, ObjectInfo> murs;
    std::map<unsigned int, ObjectInfo> portes;
    std::map<unsigned int, ObjectInfo> fenetres;
    std::map<unsigned int, ObjectInfo> activateurs;

public:
    Carte() = default;
    Carte(const LevelInfo&);
    bool isInMap(const int _idTile) const noexcept;
    std::vector<unsigned int> getObjectifs() const noexcept;

    Chemin aStar(const int _depart, const int _arrivee, const float _coefEvaluation = 1) const noexcept; // Renvoie le chemin à parcourir pour aller du départ à l'arrivée

    float distanceL2(const int _depart, const int _arrivee) const noexcept; // Renvoie la distance L2 à vol d'oiseau !
    int distanceHex(const int _depart, const int _arrivee) const noexcept;
    Tile::ETilePosition getDirection(const int _tileFrom, const int _tileTo) const noexcept; // Permet de savoir dans quel sens se déplacer pour aller d'une tile à l'autre si celles-ci sont adjacentes ! =)
    int Carte::getAdjacentTileAt(const int _tileSource, const Tile::ETilePosition _direction) const noexcept; // Permet de récupérer l'indice d'une tuile adjacente à une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(const TileInfo&) noexcept; // Permet de rajouter une tile à la map
    void addObject(const ObjectInfo&) noexcept; // Permet de rajouter un object à la map

    int getX(const int _id) const noexcept; // Permet de récupérer x et y à partir d'un indice
    int getY(const int _id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(const int _id);
    const MapTile& getTile(const int id) const;

    std::vector<unsigned int> getObjectifs();
    std::map<unsigned int, ObjectInfo> getMurs();
    std::map<unsigned int, ObjectInfo> getPortes();
    std::map<unsigned int, ObjectInfo> getFenetres();
    std::map<unsigned int, ObjectInfo> getActivateurs();

    bool objectExist(const int _id)const noexcept; // Permet de savoir si un objet existe déjà ou pas
};


#endif
