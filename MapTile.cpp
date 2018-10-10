
#include "MapTile.h"
#include "Map.h"
#include "TileInfo.h"
#include "GameManager.h"

MapTile::MapTile(const TileInfo ti, int rowCount, int colCount) :
    id{static_cast<int>(ti.tileID)},
    x{id % colCount},
    y{id / colCount},
    voisins{vector<int>()},
    voisinsAccessibles{vector<int>()},
    type{ti.tileType}
{
}

void MapTile::setVoisins(Map m) {
    voisins = vector<int>();

    // Si quelqu'un peut me dire comment faire ça mieux, je suis preneur ! x)

    // On regarde sur quelle ligne on est, car ça change les indices
    int indice;
    if (y % 2 == 0) { // Ligne paire
        // NE
        indice = id - m.colCount;
        if (m.isInMap(indice) && y > 0) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // SE
        indice = id + m.colCount;
        if (m.isInMap(indice) && y < m.rowCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // SW
        indice = id + m.colCount - 1;
        if (m.isInMap(indice) && y < m.rowCount-1 && x > 0) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // NW
        indice = id - m.colCount - 1;
        if (m.isInMap(indice) && y > 0 && x > 0) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }

    } else { // Ligne impaire !
        // NE
        indice = id - m.colCount + 1;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // E
        indice = id + 1;
        if (m.isInMap(indice) && x < m.colCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // SE
        indice = id + m.colCount + 1;
        if (m.isInMap(indice) && x < m.colCount-1 && y < m.rowCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // SW
        indice = id + m.colCount;
        if (m.isInMap(indice) && y < m.rowCount-1) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // W
        indice = id - 1;
        if (m.isInMap(indice) && x > 0) {
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
        // NW
        indice = id - m.colCount;
        if (m.isInMap(indice)) { // Pas de conditions, c'est marrant ! =)
            voisins.push_back(indice);
            if (m.areAccessible(id, indice)) {
                voisinsAccessibles.push_back(indice);
            }
        }
    }
    GameManager::Log("Voisins accessibles de la tile " + to_string(id));
    for (auto v : voisinsAccessibles) {
        GameManager::Log(to_string(v));
    }
}

