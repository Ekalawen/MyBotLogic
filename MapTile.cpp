
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(unsigned int id, Map &m) :
    id{ static_cast<int>(id) },
    x{ static_cast<int>(id) % m.getColCount() },
    y{ static_cast<int>(id) / m.getColCount() },
	voisins{ vector<int>{} },
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
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
            voisins.push_back(indice);
        }
        // SW 
        indice = id + m.getColCount() - 1;
        if (m.isInMap(indice) && y < m.getRowCount() - 1 && x > 0) {
           voisinsDirection[Tile::SW] = indice;
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = id - m.getColCount() - 1;
        if (m.isInMap(indice) && y > 0 && x > 0) {
           voisinsDirection[Tile::NW] = indice;
           voisins.push_back(indice);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = id - m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::NE] = indice;
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1) {
           voisinsDirection[Tile::E] = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = id + m.getColCount() + 1;
        if (m.isInMap(indice) && x < m.getColCount() - 1 && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SE] = indice;
            voisins.push_back(indice);
        }
        // SW
        indice = id + m.getColCount();
        if (m.isInMap(indice) && y < m.getRowCount() - 1) {
           voisinsDirection[Tile::SW] = indice;
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
           voisinsDirection[Tile::W] = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = id - m.getColCount();
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
           voisinsDirection[Tile::NW] = indice;
            voisins.push_back(indice);
        }
    }

    voisinsVisibles = voisins;
    voisinsAccessibles = voisins;
    voisinsMysterieux = voisins;
}

void MapTile::setTileDecouverte(const TileInfo tile) {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAccessible(int id) const noexcept {
	return std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isVoisinVisible(int id) const noexcept {
	return std::find(voisinsVisibles.begin(), voisinsVisibles.end(), id) != voisinsVisibles.end();
}

bool MapTile::isVoisinMysterious(int id) const noexcept {
	return std::find(voisinsMysterieux.begin(), voisinsMysterieux.end(), id) != voisinsMysterieux.end();
}

int MapTile::getVoisinByDirection(Tile::ETilePosition direction) const noexcept {
   return voisinsDirection[direction];
}

void MapTile::removeMysterieux(int id) {
    auto it = find(voisinsMysterieux.begin(), voisinsMysterieux.end(), id);
    if (it != voisinsMysterieux.end()) {
        voisinsMysterieux.erase(it);
    }
}

void MapTile::removeAccessible(int id) {
    auto it = find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id);
    if (it != voisinsAccessibles.end()) {
        voisinsAccessibles.erase(it);
    }
}

void MapTile::removeVisible(int id) {
    auto it = find(voisinsVisibles.begin(), voisinsVisibles.end(), id);
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

vector<int> MapTile::getVoisins() const noexcept {
    return voisins;
}

vector<int> MapTile::getVoisinsAccessibles() const noexcept {
    return voisinsAccessibles;
}

vector<int> MapTile::getVoisinsVisibles() const noexcept {
    return voisinsVisibles;
}

vector<int> MapTile::getVoisinsMysterieux() const noexcept {
    return voisinsMysterieux;
}

bool MapTile::isInVoisins(int id) const noexcept {
    return find(voisins.begin(), voisins.end(), id) != voisins.end();
}

bool MapTile::isInVoisinsAccessibles(int id) const noexcept {
    return find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isInVoisinsVisibles(int id) const noexcept {
    return find(voisinsVisibles.begin(), voisinsVisibles.end(), id) != voisinsVisibles.end();
}

bool MapTile::isInVoisinsMysterieux(int id) const noexcept {
    return find(voisinsMysterieux.begin(), voisinsMysterieux.end(), id) != voisinsMysterieux.end();
}


MapTile::Statut MapTile::getStatut() const noexcept {
    return statut;
}

void MapTile::setStatut(MapTile::Statut new_statut) {
    statut = new_statut;
}
