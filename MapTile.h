#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Map.h"
#include "Voisin.h"
#include <vector>

using namespace std;

class Map;

class MapTile {
public:
    enum Statut{INCONNU,CONNU,VISITE};

private:
    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
    int voisinsDirection[6] = { -1,-1,-1,-1,-1,-1 };
    std::vector<Voisin> voisins;
    Statut statut;

public:
    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(unsigned int id, Map &m); // Appelé dès le début et uniquement là !

    void setTileDecouverte(const TileInfo ti);
    void removeEtat(const Etats etat, const int id);

    int getVoisinByDirection(Tile::ETilePosition direction) const noexcept; // Permet de récupérer le voisin dans une certaine direction d'une tile
	bool isVoisinAvecEtat(const Etats etat, const int id) const noexcept;

    bool existe();

    int getId() const noexcept;
    int getX() const noexcept;
    int getY() const noexcept;
    Tile::ETileType getType() const noexcept;
    vector<Voisin> getVoisins() const noexcept;
    vector<Voisin> getVoisinsParEtat(const Etats etat) const noexcept;
    Statut getStatut() const noexcept;
    void setStatut(Statut new_statut);
};

#endif
