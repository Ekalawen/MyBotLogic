
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(unsigned int id, int colCount) :
   id{ static_cast<int>(id) },
   x{ static_cast<int>(id) % colCount },
   y{ static_cast<int>(id) / colCount },
   voisins{ vector<int>() },
   voisinsAccessibles{ vector<int>() },
   type{},
   NE{ -1 },
   E{ -1 },
   SE{ -1 },
   NW{ -1 },
   W{ -1 },
   SW{ -1 },
   statut {INCONNU}
{
}

MapTile::MapTile(const TileInfo ti, int rowCount, int colCount) :
	id{ static_cast<int>(ti.tileID) },
	x{ id % colCount },
	y{ id / colCount },
	voisins{ vector<int>() },
	voisinsAccessibles{ vector<int>() },
	type{ ti.tileType },
	NE{ -1 },
	E{ -1 },
	SE{ -1 },
	NW{ -1 },
	W{ -1 },
	SW{ -1 }
{
}

void MapTile::setTile(const TileInfo tile) {
   type = tile.tileType;
   statut = CONNU;
}

void MapTile::putTileInVectors(Map m, int indice) noexcept {
    // Accessible
    if (m.areAccessible(id, indice)) {
        voisinsAccessibles.push_back(indice);
    }

    // Visibles
    if (m.areVisible(id, indice)) {
        voisinsVisibles.push_back(indice);
    }

    // Mystérieux
    if (m.areMysterious(id, indice)) {
        voisinsMysterious.push_back(indice);
    }
}

void MapTile::setVoisins(Map &m) noexcept {
    // On réinitialise nos voisins
    voisins = vector<int>();
    voisinsAccessibles = vector<int>();
    voisinsVisibles = vector<int>();
    voisinsMysterious = vector<int>();

    // Si quelqu'un peut me dire comment faire ça mieux, je suis preneur ! x)

    // On regarde sur quelle ligne on est, car ça change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
        // NE
        indice = id - m.colCount;
        NE = indice;
        if (m.isInMap(indice) && y > 0) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // E
        indice = id + 1;
        E = indice;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // SE
        indice = id + m.colCount;
        SE = indice;
        if (m.isInMap(indice) && y < m.rowCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // SW
        indice = id + m.colCount - 1;
        SW = indice;
        if (m.isInMap(indice) && y < m.rowCount-1 && x > 0) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // W
        indice = id - 1;
        W = indice;
        if (m.isInMap(indice) && x > 0) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // NW
        indice = id - m.colCount - 1;
        NW = indice;
        if (m.isInMap(indice) && y > 0 && x > 0) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }

    } else { // Ligne impaire !
        // NE
        indice = id - m.colCount + 1;
        NE = indice;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // E
        indice = id + 1;
        E = indice;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // SE
        indice = id + m.colCount + 1;
        SE = indice;
        if (m.isInMap(indice) && x < m.colCount-1 && y < m.rowCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // SW
        indice = id + m.colCount;
        SW = indice;
        if (m.isInMap(indice) && y < m.rowCount-1) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // W
        indice = id - 1;
        W = indice;
        if (m.isInMap(indice) && x > 0) {
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
        // NW
        indice = id - m.colCount;
        NW = indice;
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! =)
            voisins.push_back(indice);
			putTileInVectors(m, indice);
        }
    }
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
