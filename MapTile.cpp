
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

#include <algorithm>

MapTile::MapTile(unsigned int id, Map &m) :
    id{ static_cast<int>(id) },
    x{ static_cast<int>(id) % m.getColCount() },
    y{ static_cast<int>(id) / m.getColCount() },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
	voisins.reserve(6);
    // On regarde sur quelle ligne on est, car ça change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = id - m.getColCount();
        if (m.isInMap(indice) && y > 0) {
            voisinsDirection[Tile::NE] = indice;
            voisins.emplace_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
           voisins.emplace_back(indice);
        }
        // SE
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
           voisins.emplace_back(indice);
        }
        // SW 
        indice = id + m.getColCount() - 1;
        if (m.isInMap(indice) && y < m.getRowCount() - 1 && x > 0) {
           voisinsDirection[Tile::SW] = indice;
           voisins.emplace_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
           voisins.emplace_back(indice);
        }
        // NW
        indice = id - m.getColCount() - 1;
        if (m.isInMap(indice) && y > 0 && x > 0) {
           voisinsDirection[Tile::NW] = indice;
           voisins.emplace_back(indice);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = id - m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::NE] = indice;
           voisins.emplace_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
           voisins.emplace_back(indice);
        }
        // SE
        indice = id + m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1 && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
           voisins.emplace_back(indice);
        }
        // SW
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SW] = indice;
           voisins.emplace_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
           voisins.emplace_back(indice);
        }
        // NW
        indice = id - m.getColCount();
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisinsDirection[Tile::NW] = indice;
           voisins.emplace_back(indice);
        }
    }
}

void MapTile::setTileDecouverte(const TileInfo tile) {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAvecEtat(const Etats etat, const int id) const noexcept {
    return std::find_if(voisins.begin(), voisins.end(), [&](const Voisin& v) {
        return v.getTuileIndex() == id && v.estEtat(etat);
    }) != voisins.end();
}

int MapTile::getVoisinByDirection(Tile::ETilePosition direction) const noexcept {
   return voisinsDirection[direction];
}

void MapTile::removeEtat(const Etats etat, const int id) {
    auto it = std::find_if(voisins.begin(), voisins.end(), [&id](const Voisin& v) {
        return v.getTuileIndex() == id;
    });

    if (it != voisins.end()) {
        it->setEtat(etat, false);
    }
}

bool MapTile::existe() {
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

vector<Voisin> MapTile::getVoisinsParEtat(const Etats etat) const noexcept {

    vector<Voisin> resultat;

    std::copy_if(begin(voisins), end(voisins), back_inserter(resultat),[&etat](const Voisin& v) {
        return v.estEtat(etat);
    });

    return resultat;
}

MapTile::Statut MapTile::getStatut() const noexcept {
    return statut;
}

void MapTile::setStatut(MapTile::Statut new_statut) {
    statut = new_statut;
}
