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
 //   vector<int> voisins; // les identifiants des voisins de la tuile
	//vector<int> voisinsAccessibles; // les voisins connus et accessible (pas de murs ni de fenêtres) y compris les voisinsMysterieux
	//vector<int> voisinsVisibles; // les voisins visibles (contient les voisins accessibles et les voisins fenetres)
	//vector<int> voisinsMysterieux; // les voisins sur lequel on a pas encore d'information
    Statut statut;

public:
    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(unsigned int id, Map &m); // Appelé dès le début et uniquement là !

    void setTileDecouverte(const TileInfo ti);
    void removeMysterieux(int id);
    void removeAccessible(int id);
    void removeVisible(int id);

    int getVoisinByDirection(Tile::ETilePosition direction) const noexcept; // Permet de récupérer le voisin dans une certaine direction d'une tile
	bool isVoisinAccessible(int id) const noexcept;
	bool isVoisinVisible(int id) const noexcept;
	bool isVoisinMysterious(int id) const noexcept;

    bool existe();

    int getId() const noexcept;
    int getX() const noexcept;
    int getY() const noexcept;
    Tile::ETileType getType() const noexcept;
    vector<Voisin> getVoisins() const noexcept;
    //vector<int> getVoisinsAccessibles() const noexcept;
    //vector<int> getVoisinsVisibles() const noexcept;
    //vector<int> getVoisinsMysterieux() const noexcept;
    //bool isInVoisins(int id) const noexcept;
    //bool isInVoisinsAccessibles(int id) const noexcept;
    //bool isInVoisinsVisibles(int id) const noexcept;
    //bool isInVoisinsMysterieux(int id) const noexcept;
    Statut getStatut() const noexcept;
    void setStatut(Statut new_statut);
};

#endif
