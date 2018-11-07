#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Carte.h"
#include "Voisin.h"
#include <vector>

using std::vector;

class Carte;

class MapTile {
public:
    enum Statut { INCONNU, CONNU, VISITE };

private:
    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
    vector<Voisin> voisins;
    Statut statut;
    vector<int> portesAdjacentes;

public:
    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(unsigned int id, Carte &m); // Appelé dés le début et uniquement là !

    void setTileDecouverte(const TileInfo& _tile) noexcept;

    void removeEtat(const Etats etat, const int id);
	bool isVoisinAvecEtat(const Etats etat, const int id) const noexcept;

    bool existe() const noexcept;

    int getId() const noexcept;
    int getX() const noexcept;
    int getY() const noexcept;
    Tile::ETileType getType() const noexcept;
    vector<Voisin> getVoisins() const noexcept;
    vector<int> getVoisinsId() const noexcept;
    vector<int> getVoisinsIDParEtat(const Etats etat) const noexcept;
    bool hasDoorPoigneeVoisin(const int voisinId, const Carte& c) const noexcept; // Permet de savoir s'il y a une porte à poignée entre cette tile et sa voisine. Si la voisine n'existe pas, on lève une exception
    Statut getStatut() const noexcept;
    void setStatut(Statut new_statut);
    void addPorte(int porteId);
};

#endif
