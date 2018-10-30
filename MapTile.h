#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "MyBotLogic/Carte.h"
#include <vector>

class Carte;

class MapTile {
public:
   enum Statut { INCONNU, CONNU, VISITE };

private:
   int id;
   int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
   Tile::ETileType type;
   int voisinsDirection[6] = { -1,-1,-1,-1,-1,-1 };
   std::vector<int> voisins; // les identifiants des voisins de la tuile
   std::vector<int> voisinsAccessibles; // les voisins connus et accessible (pas de murs ni de fenêtres) y compris les voisinsMysterieux
   std::vector<int> voisinsVisibles; // les voisins visibles (contient les voisins accessibles et les voisins fenetres)
   std::vector<int> voisinsMysterieux; // les voisins sur lequel on a pas encore d'information
   Statut statut;

public:
   MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
   MapTile(unsigned int _id, Carte &_map); // Appelé dès le début et uniquement là !

   void setTileDecouverte(const TileInfo _tile);
   void removeMysterieux(int _id);
   void removeAccessible(int _id);
   void removeVisible(int _id);

   int getVoisinByDirection(Tile::ETilePosition _direction) const noexcept; // Permet de récupérer le voisin dans une certaine direction d'une tile
   bool isVoisinAccessible(int _id) const noexcept;
   bool isVoisinVisible(int _id) const noexcept;
   bool isVoisinMysterious(int _id) const noexcept;

   bool existe();

   int getId() const noexcept;
   int getX() const noexcept;
   int getY() const noexcept;
   Tile::ETileType getType() const noexcept;
   std::vector<int> getVoisins() const noexcept;
   std::vector<int> getVoisinsAccessibles() const noexcept;
   std::vector<int> getVoisinsVisibles() const noexcept;
   std::vector<int> getVoisinsMysterieux() const noexcept;
   bool isInVoisins(int _id) const noexcept;
   bool isInVoisinsAccessibles(int _id) const noexcept;
   bool isInVoisinsVisibles(int _id) const noexcept;
   bool isInVoisinsMysterieux(int _id) const noexcept;
   Statut getStatut() const noexcept;
   void setStatut(Statut _newStatut);
};

#endif
