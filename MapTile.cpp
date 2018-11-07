
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

#include <algorithm>
#include <vector>

MapTile::MapTile(unsigned int id, Map &m) :
    id{ static_cast<int>(id) },
    x{ static_cast<int>(id) % m.getColCount() },
    y{ static_cast<int>(id) / m.getColCount() },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
	voisins.reserve(6);
    // On regarde sur quelle ligne on est, car �a change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = id - m.getColCount();
        if (m.isInMap(indice) && y > 0) {
            voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW 
        indice = id + m.getColCount() - 1;
        if (m.isInMap(indice) && y < m.getRowCount() - 1 && x > 0) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = id - m.getColCount() - 1;
        if (m.isInMap(indice) && y > 0 && x > 0) {
           voisins.emplace_back(indice, Tile::NW);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = id - m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = id + m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1 && y < m.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = id - m.getColCount();
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisins.emplace_back(indice, Tile::NW);
        }
    }
}

void MapTile::setTileDecouverte(const TileInfo& tile) noexcept {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAvecEtat(const Etats etat, const int id) const noexcept {
   return std::find_if(voisins.begin(), voisins.end(), [&](const Voisin& v) {
      return v.getTuileIndex() == id && v.estEtat(etat);
   }) != voisins.end();
}

void MapTile::removeEtat(const Etats etat, const int id) {
   auto it = std::find_if(voisins.begin(), voisins.end(), [&id](const Voisin& v) {
      return v.getTuileIndex() == id;
   });

   if (it != voisins.end()) {
      it->setEtat(etat, false);
   }
}

bool MapTile::existe() const noexcept {
   return statut != MapTile::Statut::INCONNU;
}

int MapTile::getId() const noexcept {
   return id;
}

int MapTile::getX() const noexcept {
    return x;
}

int MapTile::getY() const noexcept {
    return y;
}

Tile::ETileType MapTile::getType() const noexcept {
   return type;
}

std::vector<Voisin> MapTile::getVoisins() const noexcept {
   return voisins;
}

vector<int> MapTile::getVoisinsIDParEtat(const Etats etat) const noexcept {

   std::vector<int> resultat;

   std::for_each(begin(voisins), end(voisins), [&](const Voisin& v) {
      if (v.estEtat(_etat))
         resultat.emplace_back(v.getTuileIndex());
   });

   return resultat;
}

MapTile::Statut MapTile::getStatut() const noexcept {
   return statut;
}

void MapTile::setStatut(MapTile::Statut new_statut) {
   statut = new_statut;
}
