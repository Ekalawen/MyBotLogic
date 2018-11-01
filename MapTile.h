#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Carte.h"
#include "Voisin.h"
#include <vector>

class Carte;

class MapTile {
public:
   enum Statut { INCONNU, CONNU, VISITE };

private:
    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
    std::vector<Voisin> voisins;
    Statut statut;

public:
   MapTile() = default; // Constructeur par d�faut obligatoire pour pouvoir utiliser tuple ...
   MapTile(unsigned int _id, Carte& _map); // Appel� d�s le d�but et uniquement l� !

    void setTileDecouverte(const TileInfo& _tile) noexcept;

    void removeEtat(const Etats _etat, const int _id);
	bool isVoisinAvecEtat(const Etats _etat, const int _id) const noexcept;

    bool existe() const noexcept;

    int getId() const noexcept;
    int getX() const noexcept;
    int getY() const noexcept;
    Tile::ETileType getType() const noexcept;
    std::vector<Voisin> getVoisins() const noexcept;
    std::vector<int> getVoisinsIDParEtat(const Etats _etat) const noexcept;
    Statut getStatut() const noexcept;
    void setStatut(Statut _newStatut);
};

#endif
