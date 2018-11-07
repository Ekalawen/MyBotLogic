
#include "MapTile.h"
#include "Carte.h"
#include "TileInfo.h"
#include "GameManager.h"
#include "Porte.h"
#include <algorithm>
#include <vector>

using std::vector;

MapTile::MapTile(unsigned int id, Carte &c) :
    id{ static_cast<int>(id) },
    x{ static_cast<int>(id) % c.getColCount() },
    y{ static_cast<int>(id) / c.getColCount() },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
	voisins.reserve(6);
    // On regarde sur quelle ligne on est, car �a change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = id - c.getColCount();
        if (c.isInMap(indice) && y > 0) {
            voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = id + 1;
        if (c.isInMap(indice) && x < c.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = id + c.getColCount();
        if (c.isInMap(indice) && y < c.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW 
        indice = id + c.getColCount() - 1;
        if (c.isInMap(indice) && y < c.getRowCount() - 1 && x > 0) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = id - 1;
        if (c.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = id - c.getColCount() - 1;
        if (c.isInMap(indice) && y > 0 && x > 0) {
           voisins.emplace_back(indice, Tile::NW);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = id - c.getColCount() + 1;
        if (c.isInMap(indice) && x < c.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = id + 1;
        if (c.isInMap(indice) && x < c.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = id + c.getColCount() + 1;
        if (c.isInMap(indice) && x < c.getColCount() - 1 && y < c.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW
        indice = id + c.getColCount();
        if (c.isInMap(indice) && y < c.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = id - 1;
        if (c.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = id - c.getColCount();
        if (c.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisins.emplace_back(indice, Tile::NW);
        }
    }
}

void MapTile::setTileDecouverte(const TileInfo& tile) noexcept {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAvecEtat(const Etats etat, const int id) const noexcept {
   return find_if(voisins.begin(), voisins.end(), [&](const Voisin& v) {
      return v.getTuileIndex() == id && v.estEtat(etat);
   }) != voisins.end();
}

void MapTile::removeEtat(const Etats etat, const int id) {
   auto it = find_if(voisins.begin(), voisins.end(), [&id](const Voisin& v) {
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

vector<Voisin> MapTile::getVoisins() const noexcept {
   return voisins;
}

vector<int> MapTile::getVoisinsId() const noexcept {
    vector<int> ids;
    for (auto voisin : getVoisins()) {
        ids.push_back(voisin.getTuileIndex());
    }
    return ids;
}

vector<int> MapTile::getVoisinsIDParEtat(const Etats _etat) const noexcept {

   vector<int> resultat;

   for_each(begin(voisins), end(voisins), [&](const Voisin& v) {
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

void MapTile::addPorte(int porteId) {
    portesAdjacentes.push_back(porteId);
}

bool MapTile::hasDoorPoigneeVoisin(const int voisinId, const Carte& c) const noexcept {
    for (auto porteId : portesAdjacentes) {
        Porte porte = c.getPorte(porteId);
        if (porte.getType() == Porte::A_POIGNEE && porte.getEtat() == Object::ObjectState_Closed) {
            return porte.isVoisine(voisinId);
        }
    }
    return false;
}
