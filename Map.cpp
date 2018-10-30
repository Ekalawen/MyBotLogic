
#include "Map.h"
#include "MapTile.h"
#include "GameManager.h"
#include "Globals.h"
#include <map>
#include <algorithm>
#include <chrono>
using namespace std;

Map::Map(const LevelInfo levelInfo) :
    rowCount{ levelInfo.rowCount },
    colCount{ levelInfo.colCount },
    nbTilesDecouvertes{ 0 },
	tiles{ vector<MapTile>{} },
	murs{ map<unsigned int, ObjectInfo>{} },
	fenetres{ map<unsigned int, ObjectInfo>{} },
	portes{ map<unsigned int, ObjectInfo>{} },
	activateurs{ map<unsigned int, ObjectInfo>{} }
{
    // Créer toutes les tiles !
    tiles.reserve(getNbTiles());
    for (int id = 0; id < getNbTiles(); ++id) {
       tiles.push_back(MapTile(id, *this));
    }

    // Mettre à jour les tiles connues
    for (auto tile : levelInfo.tiles) {
        addTile(tile.second);
    }

    // Enregistrer les objets
    for (auto object : levelInfo.objects) {
        addObject(object.second);
    }

    // Mettre à visiter les cases initiales des NPCs
    for (auto pair_npc : levelInfo.npcs) {
        tiles[pair_npc.second.tileID].setStatut(MapTile::Statut::VISITE);
    }
}

bool Map::isInMap(int idTile) const noexcept {
    return idTile >= 0 && idTile < rowCount * colCount;
}

vector<unsigned int> Map::getObjectifs() const noexcept {
    return objectifs;
}

struct Noeud {
    static float coefEvaluation;
    MapTile tile;
    float cout; // La distance calculé depuis le départ
    float evaluation; // La distance estimée à l'arrivée
    float heuristique; // La somme du cout et de l'evaluation
    int idPrecedant;
    Noeud() = default;
    Noeud(MapTile tile, float cout, float evaluation, int idPrecedant)
        : tile{ tile }, cout{ cout }, evaluation{ evaluation }, idPrecedant{ idPrecedant } {
        heuristique = cout + evaluation * coefEvaluation;
    }
    friend bool operator==(const Noeud& g, const Noeud& d) {
        return g.tile.getId() == d.tile.getId();
    }
};
float Noeud::coefEvaluation = 1;

// Il s'agit de l'algorithme AStar auquel on peut rajouter un coefficiant à l'évaluation pour modifier l'heuristique.
// Par défaut sa valeur est 1. Si on l'augmente l'algorithme ira plus vite au détriment de trouver un chemin optimal.
// Si on le diminue l'algorithme se rapproche de plus en plus d'un parcours en largeur.
Chemin Map::aStar(int depart, int arrivee, float coefEvaluation) noexcept {
    Noeud::coefEvaluation = coefEvaluation;
    // On crée nos liste et notre noeud courrant
    vector<Noeud> closedList{};
    vector<Noeud> openList{};
    Noeud noeudCourant = Noeud(tiles[depart], 0, distanceL2(depart, arrivee), depart);
    Chemin path;

    // On ajoute le noeud initial
    openList.push_back(noeudCourant);
    // Tant qu'il reste des noeuds à traiter ...
    while (!openList.empty() && noeudCourant.tile.getId() != arrivee) {
        // On récupère le premier noeud de notre liste
        noeudCourant = openList.back();
        openList.pop_back();
        // Pour tous les voisins du noeud courant ...
        for (auto voisin : noeudCourant.tile.getVoisins()) {
            // On vérifie que le voisin existe ...
            if (voisin.estAccessible && tiles[voisin.getTuileIndex()].existe()) {
                // On construit le nouveau noeud
                Noeud nouveauNoeud = Noeud(tiles[voisin.getTuileIndex()], noeudCourant.cout + 1, distanceL2(voisin.getTuileIndex(), arrivee), noeudCourant.tile.getId());
                // On vérifie s'il existe dans closedList avec un cout inférieur ou dans openList avec un cout inférieur
                auto itClose = find(closedList.begin(), closedList.end(), nouveauNoeud);
                auto itOpen = find(openList.begin(), openList.end(), nouveauNoeud);

                if (itClose == closedList.end() && itOpen == openList.end()) {
                    openList.push_back(nouveauNoeud);
                } else if (itClose != closedList.end() && itOpen == openList.end()) {
                    // Do nothing
                } else if (itClose == closedList.end() && itOpen != openList.end()) {
                   if ((*itOpen).heuristique > nouveauNoeud.heuristique) {
                      (*itOpen) = nouveauNoeud;
                   }
                } else {
                    GameManager::Log("OMG On a fait n'imp !");
                }
            }
        }
        // On trie notre openList pour que le dernier soit le meilleur !
        // Donc celui qui minimise et le cout, et l'évaluation !
        sort(openList.begin(), openList.end(), [](const Noeud a, const Noeud b) {
            return a.heuristique > b.heuristique; // Par ordre décroissant
        });

        // On ferme notre noeud
        closedList.push_back(noeudCourant);
    }

    // On test si on a atteint l'objectif ou pas
    if (noeudCourant.tile.getId() == arrivee) {
        // Si oui on reconstruit le path !
        while (noeudCourant.tile.getId() != depart) {
            // On enregistre dans le path ...
            path.addFirst(noeudCourant.tile.getId());
            // On cherche l'antécédant ...
            for (auto n : closedList) {
                if (n.tile.getId() == noeudCourant.idPrecedant) {
                    // On remet à jour le noeud ...
                    noeudCourant = n;
                    break;
                }
            }
        }

    } else {
        // Si non le path est inaccessible !
        path.setInaccessible();
    }

    return path;
}

Tile::ETilePosition Map::getDirection(int ind1, int ind2) const noexcept {
    int y = getY(ind1);
    bool pair = (y % 2 == 0);
    if (pair) {
        if (ind2 == ind1 - colCount) {
            return Tile::NE;
        } else if (ind2 == ind1 + 1) {
            return Tile::E;
        } else if (ind2 == ind1 + colCount) {
            return Tile::SE;
        } else if (ind2 == ind1 + colCount - 1) {
            return Tile::SW;
        } else if (ind2 == ind1 - 1) {
            return Tile::W;
        } else if (ind2 == ind1 - colCount - 1) {
            return Tile::NW;
        }
    } else {
        if (ind2 == ind1 - colCount + 1) {
            return Tile::NE;
        } else if (ind2 == ind1 + 1) {
            return Tile::E;
        } else if (ind2 == ind1 + colCount + 1) {
            return Tile::SE;
        } else if (ind2 == ind1 + colCount) {
            return Tile::SW;
        } else if (ind2 == ind1 - 1) {
            return Tile::W;
        } else if (ind2 == ind1 - colCount) {
            return Tile::NW;
        }
    }

    GameManager::Log("Erreur dans l'appel de getDirection() !");
    return Tile::CENTER;
}

int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) const noexcept {
    int y = getY(tileSource);
    bool pair = (y % 2 == 0);
    int res;
    switch (direction)
    {
    case Tile::NE:
        if (pair) {
            res = tileSource - colCount;
        } else {
            res = tileSource - colCount + 1;
        }
        break;
    case Tile::E:
        res = tileSource + 1;
        break;
    case Tile::SE:
        if (pair) {
            res = tileSource + colCount;
        } else {
            res = tileSource + colCount + 1;
        }
        break;
    case Tile::SW:
        if (pair) {
            res = tileSource + colCount - 1;
        } else {
            res = tileSource + colCount;
        }
        break;
    case Tile::W:
        res = tileSource - 1;
        break;
    case Tile::NW:
        if (pair) {
            res = tileSource - colCount - 1;
        } else {
            res = tileSource - colCount;
        }
        break;
    case Tile::CENTER:
        res = tileSource;
        break;
    default:
        break;
    }

    if (isInMap(res)) {
        return res;
    }
    else {
        //GameManager::Log("La direction demandé dans getAdjacentTileAt n'existe pas !");
        //GameManager::Log("origin = " + to_string(tileSource) + " direction = " + to_string(direction));
        return -1;
    }
}

float Map::distanceL2(int depart, int arrivee) const noexcept {
    int xd = depart % colCount;
    int yd = depart / colCount;
    int xa = arrivee % colCount;
    int ya = arrivee / colCount;
    return (float)sqrt(pow(xd - xa, 2) + pow(yd - ya, 2));
}

int Map::distanceHex(int tile1ID, int tile2ID) const noexcept {
   int ligne1 = tile1ID / colCount;
   int colonne1 = tile1ID % colCount;
   int ligne2 = tile2ID / colCount;
   int colonne2 = tile2ID % colCount;
   int x1 = colonne1 - (ligne1 - ligne1 % 2) / 2;
   int z1 = ligne1;
   int y1 = -x1 - z1;
   int x2 = colonne2 - (ligne2 - ligne2 % 2) / 2;
   int z2 = ligne2;
   int y2 = -x2 - z2;
   return max(max(abs(x1 - x2), abs(y1 - y2)), abs(z1 - z2));
}

int Map::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Map::addTile(TileInfo tile) noexcept {
    // On met à jour le nombre de tiles
    ++nbTilesDecouvertes;

    // On la rajoute aux tiles
    tiles[tile.tileID].setTileDecouverte(tile);

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Goal) {
        objectifs.push_back(tile.tileID);
    }

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Forbidden) {
        for (auto voisin : tiles[tile.tileID].getVoisins()) {
            tiles[voisin.getTuileIndex()].removeAccessible(tile.tileID);
        }
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisin : tiles[tile.tileID].getVoisins()) { // On pourrait parcourir les voisinsVisibles
        // Si ce voisin l'a en voisin mystérieux, on le lui enlève
        tiles[voisin.getTuileIndex()].removeMysterieux(tile.tileID);
    }

    // On le note !
    GameManager::Log("Decouverte de la tile " + to_string(tile.tileID));
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Map::addObject(ObjectInfo object) noexcept {
    int voisin1 = object.tileID;
	int voisin2 = getAdjacentTileAt(object.tileID, object.position);

    // On ajoute notre objet à l'ensemble de nos objets
    if (object.objectTypes.find(Object::ObjectType_Wall) != object.objectTypes.end()) {
       // Fenetre
       if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
          fenetres[object.objectID] = object;
          if (isInMap(voisin1))
             tiles[voisin1].removeAccessible(voisin2);
          if (isInMap(voisin2))
             tiles[voisin2].removeAccessible(voisin1);
       // Mur
       } else {
          murs[object.objectID] = object;
          if (isInMap(voisin1)) {
             tiles[voisin1].removeMysterieux(voisin2);
             tiles[voisin1].removeAccessible(voisin2);
             tiles[voisin1].removeVisible(voisin2);
          }
          if (isInMap(voisin2)) {
             tiles[voisin2].removeMysterieux(voisin1);
             tiles[voisin2].removeAccessible(voisin1);
             tiles[voisin2].removeVisible(voisin1);
          }
       }
    }
    if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
        portes[object.objectID] = object;
        //Porte Ferme
        if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
           // Porte Fenetre
           if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
              if (isInMap(voisin1))
                 tiles[voisin1].removeAccessible(voisin2);
              if (isInMap(voisin2))
                 tiles[voisin2].removeAccessible(voisin1);
            // Porte
           } else {
              if (isInMap(voisin1)) {
                 tiles[voisin1].removeAccessible(voisin2);
                 tiles[voisin1].removeVisible(voisin2);
              }
              if (isInMap(voisin2)) {
                 tiles[voisin2].removeAccessible(voisin1);
                 tiles[voisin2].removeVisible(voisin1);
              }
           }
           // Porte ouverte
        } else {
           // Si la porte est ouverte on est accessible ET visible ! =)
        }
    }
    if (object.objectTypes.find(Object::ObjectType_PressurePlate) != object.objectTypes.end()) {
        activateurs[object.objectID] = object;
        // prout !
    }
    
    // On le note !
    GameManager::Log("Decouverte de l'objet " + to_string(object.objectID) + " sur la tuile " + to_string(object.tileID) + " orienté en " + to_string(object.position));
}

int Map::getX(int id) const noexcept {
    return id % colCount;
}
int Map::getY(int id) const noexcept {
    return id / colCount;
}

vector<int> Map::getVoisins(int id) const noexcept {
    vector<int> voisins;
    int x = getX(id);
    int y = getY(id);
    int indice;
    if (y % 2 == 0) { // Ligne paire
        // NE
        indice = id - colCount;
        if (isInMap(indice) && y > 0) {
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // SE
        indice = id + colCount;
        if (isInMap(indice) && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // SW
        indice = id + colCount - 1;
        if (isInMap(indice) && y < rowCount-1 && x > 0) {
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = id - colCount - 1;
        if (isInMap(indice) && y > 0 && x > 0) {
            voisins.push_back(indice);
        }

    } else { // Ligne impaire !
        // NE
        indice = id - colCount + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // SE
        indice = id + colCount + 1;
        if (isInMap(indice) && x < colCount-1 && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // SW
        indice = id + colCount;
        if (isInMap(indice) && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = id - colCount;
        if (isInMap(indice)) { // Pas de conditions, c'est marrant ! =)
            voisins.push_back(indice);
        }
    }

    return voisins;
}

int Map::getRowCount() const noexcept {
    return rowCount;
}

int Map::getColCount() const noexcept {
    return colCount;
}

int Map::getNbTiles() const noexcept {
    return getRowCount() * getColCount();
}

int Map::getNbTilesDecouvertes() const noexcept {
    return nbTilesDecouvertes;
}

MapTile& Map::getTile(int id) {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

vector<unsigned int> Map::getObjectifs() {
    return objectifs;
}

map<unsigned int, ObjectInfo> Map::getMurs() {
    return murs;
}

map<unsigned int, ObjectInfo> Map::getPortes() {
    return portes;
}

map<unsigned int, ObjectInfo> Map::getFenetres() {
    return fenetres;
}

map<unsigned int, ObjectInfo> Map::getActivateurs() {
    return activateurs;
}

bool Map::objectExist(int objet) {
    return murs.find(objet) != murs.end()
        || portes.find(objet) != portes.end()
        || fenetres.find(objet) != fenetres.end()
        || activateurs.find(objet) != activateurs.end();
}
