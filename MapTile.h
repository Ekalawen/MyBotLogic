#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Map.h"
#include <vector>
using namespace std;

class Map;

class MapTile {
public:
    enum Statut{INCONNU,CONNU,VISITE};

    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
	int NE, E, SE, SW, W, NW;
    vector<int> voisins; // les identifiants des voisins de la tuile
	vector<int> voisinsAccessibles; // les voisins connus et accessible (pas de murs ni de fenêtres) y compris les voisinsMysterious
	vector<int> voisinsVisibles; // les voisins visibles (contient les voisins accessibles et les voisins fenetres)
	vector<int> voisinsMysterious; // les voisins sur lequel on a pas encore d'information
    Statut statut;


    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(unsigned int id, Map &m); // Appelé dès le début et uniquement là !

    void setTileDecouverte(const TileInfo ti);
    void setVoisins(Map &m) noexcept;
    void removeMysterieux(int id);
    void removeAccessible(int id);
    void removeVisible(int id);

    int getVoisinByDirection(Tile::ETilePosition direction) const noexcept; // Permet de récupérer le voisin dans une certaine direction d'une tile
	bool isVoisinAccessible(Tile::ETilePosition direction) const noexcept;
	bool isVoisinAccessible(int id) const noexcept;
	bool isVoisinVisible(int id) const noexcept;
	bool isVoisinMysterious(int id) const noexcept;
	vector<int> getVoisinFenetres() const noexcept;

private:
	void putTileInVectors(Map m, int indice) noexcept;
};

#endif
