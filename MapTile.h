#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Map.h"
#include "Voisin.h"
#include <vector>

class Map;

class MapTile {
public:
    enum Statut{ INCONNU, CONNU, VISITE };
    const static int NB_VOISINS_TOTAL = 6;

private:
    int id;
    MapPosition position;
    Tile::ETileType type;
    std::vector<Voisin> voisins;
    Statut statut;

public:
    MapTile() = default; // Constructeur par d�faut obligatoire pour pouvoir utiliser tuple ...
    MapTile(const unsigned int _id, MapPosition& _position); // Appel� d�s le d�but et uniquement l� !

    void setTileDecouverte(const TileInfo& _tile) noexcept;

    void removeEtat(const Etats etat, const int id);
	bool isVoisinAvecEtat(const Etats etat, const int id) const noexcept;

    bool existe() const noexcept;

    int getId() const noexcept;
    MapPosition getPosition() const noexcept;
    Tile::ETileType getType() const noexcept;
    std::vector<Voisin> getVoisins() const noexcept;
    std::vector<Voisin>& getVoisins() noexcept;
    std::vector<int> getVoisinsIDParEtat(const Etats _etat) const noexcept;
    Statut getStatut() const noexcept;
    void setStatut(Statut new_statut);
};

#endif
