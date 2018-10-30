#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "LevelInfo.h"
#include "TileInfo.h"
#include "ObjectInfo.h"
#include "Chemin.h"
#include "Npc.h"
#include "MyBotLogic/MapTile.h"
#include <map>

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
    Carte(const LevelInfo);
    bool isInMap(int _idTile) const noexcept;
    std::vector<unsigned int> getObjectifs() const noexcept;

    Chemin aStar(int _depart, int _arrivee, float _coefEvaluation = 1) noexcept; // Renvoie le chemin � parcourir pour aller du d�part � l'arriv�e

    float distanceL2(int _depart, int _arrivee) const noexcept; // Renvoie la distance L2 � vol d'oiseau !
    int distanceHex(int _depart, int _arrivee) const noexcept;
    Tile::ETilePosition getDirection(int _tileFrom, int _tileTo) const noexcept; // Permet de savoir dans quel sens se d�placer pour aller d'une tile � l'autre si celles-ci sont adjacentes ! =)
    int Carte::getAdjacentTileAt(int _tileSource, Tile::ETilePosition _direction) const noexcept; // Permet de r�cup�rer l'indice d'une tuile adjacente � une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(TileInfo) noexcept; // Permet de rajouter une tile � la map
    void addObject(ObjectInfo) noexcept; // Permet de rajouter un object � la map

    int getX(int _id) const noexcept; // Permet de r�cup�rer x et y � partir d'un indice
    int getY(int _id) const noexcept;
    std::vector<int> getVoisins(int _id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(int _id);

    std::vector<unsigned int> getObjectifs();
    std::map<unsigned int, ObjectInfo> getMurs();
    std::map<unsigned int, ObjectInfo> getPortes();
    std::map<unsigned int, ObjectInfo> getFenetres();
    std::map<unsigned int, ObjectInfo> getActivateurs();

    bool objectExist(int id); // Permet de savoir si un objet existe d�j� ou pas
};


#endif
