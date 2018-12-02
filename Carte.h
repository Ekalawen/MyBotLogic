#ifndef CARTE_H
#define CARTE_H

#include "Porte.h"
#include "Activateur.h"
#include "Npc.h"
#include "MapTile.h"
#include "Chemin.h"
#include "LevelInfo.h"
#include "ObjectInfo.h"
#include "Porte.h"
using std::vector;
using std::map;

class tile_inexistante {};
class porte_inexistante {};

struct TileInfo;
class GameManager;
class Carte {
    int rowCount;
    int colCount;
    int nbTilesDecouvertes;
    vector<MapTile> tiles;

    vector<unsigned int> objectifs;

    map<unsigned int, ObjectInfo> murs;
    map<int, Porte> portes;
    map<unsigned int, ObjectInfo> fenetres;
    map<unsigned int, Activateur> activateurs;

public:
    Carte() = default;
    Carte(const LevelInfo&);
    bool isInMap(const int idTile) const noexcept;
    vector<unsigned int> getObjectifs() const noexcept;

    Chemin aStar(const int depart, const int arrivee, int npcActif, GameManager& gm, const vector<Contrainte>& contraintesDejaNecessaires = vector<Contrainte>{}) const noexcept;
    //Chemin aStar(const int depart, const int arrivee, GameManager& gm, const vector<int>& npcsOccupesIds = vector<int>{}, vector<Contrainte>& contraintesNecessaires = vector<Contrainte>{}) const noexcept; // Renvoie le chemin à parcourir pour aller du départ à l'arrivée

    float distanceL2(const int depart, const int arrivee) const noexcept; // Renvoie la distance L2 à vol d'oiseau !
    int distanceHex(const int depart, const int arrivee) const noexcept;
    Tile::ETilePosition getDirection(const int tile1, const int tile2) const noexcept; // Permet de savoir dans quel sens se déplacer pour aller d'une tile à l'autre si celles-ci sont adjacentes ! =)
    int getAdjacentTileAt(const int tileSource, const Tile::ETilePosition direction) const noexcept; // Permet de récupérer l'indice d'une tuile adjacente à une autre
    int tailleCheminMax() const noexcept; // Permet de savoir la taille maximum d'un chemin

    void addTile(const TileInfo&) noexcept; // Permet de rajouter une tile à la map
    void addObject(const ObjectInfo&) noexcept; // Permet de rajouter un object à la map
    void presumerConnu(const int idTile) noexcept; // Permet de présumer qu'une tuile existe, mais de la découvrir quand même le jour où on la verra réellement !

    int getX(const int id) const noexcept; // Permet de récupérer x et y à partir d'un indice
    int getY(const int id) const noexcept;

    int getRowCount() const noexcept;
    int getColCount() const noexcept;
    int getNbTiles() const noexcept;
    int getNbTilesDecouvertes() const noexcept;
    MapTile& getTile(const int id);
    const MapTile& getTile(const int id) const;

    vector<unsigned int> getObjectifs();
    map<unsigned int, ObjectInfo> getMurs();
    map<int, Porte>& getPortes();
    Porte getPorte(const int id) const noexcept;
    Porte& getPorte(const int tileIdVoisine1, const int tileIdVoisine2);
    map<unsigned int, ObjectInfo> getFenetres();
    map<unsigned int, Activateur> getActivateurs() const noexcept;
    bool isKnownActivateur(const int activateurId) const noexcept;
    bool isActivateurUnderTileId(const int tileId) const noexcept;

    bool objectExist(const int id) const noexcept; // Permet de savoir si un objet existe déjà ou pas
};


#endif
