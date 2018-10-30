
#include "MapTile.h"
#include "Carte.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(unsigned int _id, Carte &_map) :
    id{ static_cast<int>(_id) },
    x{ static_cast<int>(_id) % _map.getColCount() },
    y{ static_cast<int>(_id) / _map.getColCount() },
	voisins{ std::vector<int>{} },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
	voisins.reserve(6);
    // On regarde sur quelle ligne on est, car ça change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = _id - _map.getColCount();
        if (_map.isInMap(indice) && y > 0) {
            voisinsDirection[Tile::NE] = indice;
            voisins.push_back(indice);
        }
        // E
        indice = _id + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = _id + _map.getColCount();
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
            voisins.push_back(indice);
        }
        // SW 
        indice = _id + _map.getColCount() - 1;
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1 && x > 0) {
           voisinsDirection[Tile::SW] = indice;
            voisins.push_back(indice);
        }
        // W
        indice = _id - 1;
        if (_map.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = _id - _map.getColCount() - 1;
        if (_map.isInMap(indice) && y > 0 && x > 0) {
           voisinsDirection[Tile::NW] = indice;
           voisins.push_back(indice);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = _id - _map.getColCount() + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisinsDirection[Tile::NE] = indice;
            voisins.push_back(indice);
        }
        // E
        indice = _id + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = _id + _map.getColCount() + 1;
        if (_map.isInMap(indice) && x < _map.getColCount() - 1 && y < _map.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
            voisins.push_back(indice);
        }
        // SW
        indice = _id + _map.getColCount();
        if (_map.isInMap(indice) && y < _map.getRowCount() - 1) {
           voisinsDirection[Tile::SW] = indice;
            voisins.push_back(indice);
        }
        // W
        indice = _id - 1;
        if (_map.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = _id - _map.getColCount();
        if (_map.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisinsDirection[Tile::NW] = indice;
            voisins.push_back(indice);
        }
    }

    voisinsVisibles = voisins;
    voisinsAccessibles = voisins;
    voisinsMysterieux = voisins;
}

void MapTile::setTileDecouverte(const TileInfo _tile) {
   type = _tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAccessible(int _id) const noexcept {
	return std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), _id) != voisinsAccessibles.end();
}

bool MapTile::isVoisinVisible(int _id) const noexcept {
	return std::find(voisinsVisibles.begin(), voisinsVisibles.end(), _id) != voisinsVisibles.end();
}

bool MapTile::isVoisinMysterious(int _id) const noexcept {
	return std::find(voisinsMysterieux.begin(), voisinsMysterieux.end(), _id) != voisinsMysterieux.end();
}

int MapTile::getVoisinByDirection(Tile::ETilePosition _direction) const noexcept {
   return voisinsDirection[_direction];
}

void MapTile::removeMysterieux(int _id) {
    auto it = find(voisinsMysterieux.begin(), voisinsMysterieux.end(), _id);
    if (it != voisinsMysterieux.end()) {
        voisinsMysterieux.erase(it);
    }
}

void MapTile::removeAccessible(int _id) {
    auto it = find(voisinsAccessibles.begin(), voisinsAccessibles.end(), _id);
    if (it != voisinsAccessibles.end()) {
        voisinsAccessibles.erase(it);
    }
}

void MapTile::removeVisible(int _id) {
    auto it = find(voisinsVisibles.begin(), voisinsVisibles.end(), _id);
    if (it != voisinsVisibles.end()) {
        voisinsVisibles.erase(it);
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

std::vector<int> MapTile::getVoisins() const noexcept {
    return voisins;
}

std::vector<int> MapTile::getVoisinsAccessibles() const noexcept {
    return voisinsAccessibles;
}

std::vector<int> MapTile::getVoisinsVisibles() const noexcept {
    return voisinsVisibles;
}

std::vector<int> MapTile::getVoisinsMysterieux() const noexcept {
    return voisinsMysterieux;
}

bool MapTile::isInVoisins(int _id) const noexcept {
    return find(voisins.begin(), voisins.end(), _id) != voisins.end();
}

bool MapTile::isInVoisinsAccessibles(int _id) const noexcept {
    return find(voisinsAccessibles.begin(), voisinsAccessibles.end(), _id) != voisinsAccessibles.end();
}

bool MapTile::isInVoisinsVisibles(int _id) const noexcept {
    return find(voisinsVisibles.begin(), voisinsVisibles.end(), _id) != voisinsVisibles.end();
}

bool MapTile::isInVoisinsMysterieux(int _id) const noexcept {
    return find(voisinsMysterieux.begin(), voisinsMysterieux.end(), _id) != voisinsMysterieux.end();
}


MapTile::Statut MapTile::getStatut() const noexcept {
    return statut;
}

void MapTile::setStatut(MapTile::Statut _newStatut) {
    statut = _newStatut;
}
