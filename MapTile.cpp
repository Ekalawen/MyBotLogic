
#include "MapTile.h"
#include "Carte.h"
#include "TileInfo.h"
#include "GameManager.h"

#include <algorithm>

MapTile::MapTile(unsigned int _id, Carte& _map) :
    id{ static_cast<int>(_id) },
    x{ static_cast<int>(_id) % _map.getColCount() },
    y{ static_cast<int>(_id) / _map.getColCount() },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
	voisins.reserve(6);
    // On regarde sur quelle ligne on est, car �a change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = _id - _map.getColCount();
        if (_map.isInMap(indice) && y > 0) {
            voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = _id + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = _id + _map.getColCount();
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW 
        indice = _id + _map.getColCount() - 1;
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1 && x > 0) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = _id - 1;
        if (_map.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = _id - _map.getColCount() - 1;
        if (_map.isInMap(indice) && y > 0 && x > 0) {
           voisins.emplace_back(indice, Tile::NW);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = _id - _map.getColCount() + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::NE);
        }
        // E
        indice = _id + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisins.emplace_back(indice, Tile::E);
        }
        // SE
        indice = _id + _map.getColCount() + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1 && y < _map.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SE);
        }
        // SW
        indice = _id + _map.getColCount();
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1) {
           voisins.emplace_back(indice, Tile::SW);
        }
        // W
        indice = _id - 1;
        if (_map.isInMap(indice) && x > 0) {
           voisins.emplace_back(indice, Tile::W);
        }
        // NW
        indice = _id - _map.getColCount();
        if (_map.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisins.emplace_back(indice, Tile::NW);
        }
    }
}

void MapTile::setTileDecouverte(const TileInfo& _tile) noexcept {
   type = _tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAvecEtat(const Etats _etat, const int _id) const noexcept {
    return std::find_if(voisins.begin(), voisins.end(), [&](const Voisin& v) {
        return v.getTuileIndex() == _id && v.estEtat(_etat);
    }) != voisins.end();
}

void MapTile::removeEtat(const Etats _etat, const int _id) {
    auto it = std::find_if(voisins.begin(), voisins.end(), [&_id](const Voisin& v) {
        return v.getTuileIndex() == _id;
    });

    if (it != voisins.end()) {
        it->setEtat(_etat, false);
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

std::vector<int> MapTile::getVoisinsIDParEtat(const Etats _etat) const noexcept {

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

void MapTile::setStatut(MapTile::Statut _newStatut) {
    statut = _newStatut;
}
