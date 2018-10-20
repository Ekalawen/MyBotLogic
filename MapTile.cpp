
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(unsigned int id, Map &m) :
    id{ static_cast<int>(id) },
    x{ static_cast<int>(id) % m.colCount },
    y{ static_cast<int>(id) / m.colCount },
    voisins{ vector<int>() },
    type{},
    NE{ -1 },
    E{ -1 },
    SE{ -1 },
    NW{ -1 },
    W{ -1 },
    SW{ -1 },
    statut{ INCONNU }
{
    // On regarde sur quelle ligne on est, car ça change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
                      // NE
        indice = id - m.colCount;
        if (m.isInMap(indice) && y > 0) {
            NE = indice;
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.colCount - 1) {
            E = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = id + m.colCount;
        if (m.isInMap(indice) && y < m.rowCount - 1) {
            SE = indice;
            voisins.push_back(indice);
        }
        // SW 
        indice = id + m.colCount - 1;
        if (m.isInMap(indice) && y < m.rowCount - 1 && x > 0) {
            SW = indice;
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
            W = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = id - m.colCount - 1;
        if (m.isInMap(indice) && y > 0 && x > 0) {
            NW = indice;
            voisins.push_back(indice);
        }
    }
    else { // Ligne impaire !
           // NE
        indice = id - m.colCount + 1;
        if (m.isInMap(indice) && x < m.colCount - 1) {
            NE = indice;
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.colCount - 1) {
            E = indice;
            voisins.push_back(indice);
        }
        // SE
        indice = id + m.colCount + 1;
        if (m.isInMap(indice) && x < m.colCount - 1 && y < m.rowCount - 1) {
            SE = indice;
            voisins.push_back(indice);
        }
        // SW
        indice = id + m.colCount;
        if (m.isInMap(indice) && y < m.rowCount - 1) {
            SW = indice;
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
            W = indice;
            voisins.push_back(indice);
        }
        // NW
        indice = id - m.colCount;
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! :smiley:
            NW = indice;
            voisins.push_back(indice);
        }
    }

    voisinsVisibles = voisins;
    voisinsAccessibles = voisins;
    voisinsMysterious = voisins;
}

void MapTile::setTileDecouverte(const TileInfo tile) {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAccessible(Tile::ETilePosition direction) const noexcept {
	int id{};
	switch (direction)
	{
	case Tile::NE:
		id = NE;
		break;
	case Tile::E:
		id = E;
		break;
	case Tile::SE:
		id = SE;
		break;
	case Tile::SW:
		id = SW;
		break;
	case Tile::W:
		id = W;
		break;
	case Tile::NW:
		id = NW;
		break;
	default:
		GameManager::Log("Tentative d'obtenir un voisin n'existant pas ! id = " + to_string(id) + " direction = " + to_string(direction));
		return false;
		break;
	}

	return std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isVoisinAccessible(int id) const noexcept {
	return std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), id) != voisinsAccessibles.end();
}

bool MapTile::isVoisinVisible(int id) const noexcept {
	return std::find(voisinsVisibles.begin(), voisinsVisibles.end(), id) != voisinsVisibles.end();
}

bool MapTile::isVoisinMysterious(int id) const noexcept {
	return std::find(voisinsMysterious.begin(), voisinsMysterious.end(), id) != voisinsMysterious.end();
}

vector<int> MapTile::getVoisinFenetres() const noexcept {
	vector<int> voisinsFenetres{};
	for (auto idTile : voisinsVisibles) {
		if (std::find(voisinsAccessibles.begin(), voisinsAccessibles.end(), idTile) == voisinsAccessibles.end()) {
			voisinsFenetres.push_back(idTile);
		}
	}
	return voisinsFenetres;
}

int MapTile::getVoisinByDirection(Tile::ETilePosition direction) const noexcept {
    switch (direction)
    {
    case Tile::NE:
        return NE;
        break;
    case Tile::E:
        return E;
        break;
    case Tile::SE:
        return SE;
        break;
    case Tile::SW:
        return SW;
        break;
    case Tile::W:
        return W;
        break;
    case Tile::NW:
        return NW;
        break;
    default:
        GameManager::Log("Tentative d'obtenir un voisin n'existant pas ! id = " + to_string(id) + " direction = " + to_string(direction));
        return -1;
        break;
    }
}

void MapTile::removeMysterieux(int id) {
    auto it = find(voisinsMysterious.begin(), voisinsMysterious.end(), id);
    if (it != voisinsMysterious.end()) {
        voisinsMysterious.erase(it);
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
