
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
    nbTiles{ rowCount * colCount },
    nbtilesDecouvertes{ 0 },
    tiles{},
    murs{},
    fenetres{},
    portes{},
    activateurs{}
{
    // Créer toutes les tiles !
    tiles.reserve(nbTiles);
    for (int id = 0; id < nbTiles; ++id) {
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
        return g.tile.id == d.tile.id;
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
    Noeud noeudCourant;
    Chemin path;

    // On ajoute le noeud initial
    openList.push_back(Noeud(tiles[depart], 0, distanceL2(depart, arrivee), depart));

    // Tant qu'il reste des noeuds à traiter ...
    while (!openList.empty() && noeudCourant.tile.id != arrivee) {
        // On récupère le premier noeud de notre liste
        noeudCourant = openList.back();
        openList.pop_back();

        // Pour tous les voisins du noeud courant ...
        for (auto voisin : noeudCourant.tile.voisinsAccessibles) {
            // On vérifie que le voisin existe ...
            if (tiles[voisin].existe()) {
                // On construit le nouveau noeud
                Noeud nouveauNoeud = Noeud(tiles[voisin], noeudCourant.cout + 1, distanceL2(voisin, arrivee), noeudCourant.tile.id);
                // On vérifie s'il existe dans closedList avec un cout inférieur ou dans openList avec un cout inférieur
                auto itClose = find(closedList.begin(), closedList.end(), nouveauNoeud);
                auto itOpen = find(openList.begin(), openList.end(), nouveauNoeud);

                if (itClose == closedList.end() && itOpen == openList.end()) {
                    openList.push_back(nouveauNoeud);
                } else if (itClose != closedList.end() && itOpen == openList.end()) {
                    // Do nothing
                } else if (itClose == closedList.end() && itOpen != openList.end()) {
                    (*itOpen) = nouveauNoeud;
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
    if (noeudCourant.tile.id == arrivee) {
        // Si oui on reconstruit le path !
        while (noeudCourant.tile.id != depart) {
            // On enregistre dans le path ...
            path.chemin.push_back(noeudCourant.tile.id);

            // On cherche l'antécédant ...
            for (auto n : closedList) {
                if (n.tile.id == noeudCourant.idPrecedant) {
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

void Map::sortByDistance(vector<tuple<int, float>>& base, vector<int>& autre1, vector<int>& autre2) noexcept {
    // On va vouloir trier base dans l'ordre décroissant (la plus petite valeur en dernière)
    // Puis retenir la permutation
    // Et l'appliquer aux deux autres vecteurs =)

    // Les index dans l'ordre
    vector<int> index(base.size(), 0);
    for (int i = 0; i < index.size(); ++i) {
        index[i] = i;
    }

    // La permutation à appliquer à base
    sort(index.begin(), index.end(),
        [&](const int& a, const int& b) {
            return (get<float>(base[a]) > get<float>(base[b])); // décroissant
    });

    // On applique la permutation à base, autre1 et autre2
    vector<tuple<int, float>> basebis = base;
    vector<int> autre1bis = autre1;
    vector<int> autre2bis = autre2;
    for (int i = 0; i < index.size(); ++i) {
        basebis[i] = base[index[i]];
        autre1bis[i] = autre1[index[i]];
        autre2bis[i] = autre2[index[i]];
    }
    base = basebis;
    autre1 = autre1bis;
    autre2 = autre2bis;
}

int Map::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Map::addTile(TileInfo tile) noexcept {
    // On met à jour le nombre de tiles
    ++nbtilesDecouvertes;

    // On la rajoute aux tiles
    tiles[tile.tileID].setTileDecouverte(tile);
    if (tiles[tile.tileID].type == Tile::TileAttribute_Goal) {
        objectifs.push_back(tile.tileID);
    }
    if (tiles[tile.tileID].type == Tile::TileAttribute_Forbidden) {
        for (auto voisin : tiles[tile.tileID].voisins) {
            tiles[voisin].removeAccessible(tile.tileID);
        }
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisin : tiles[tile.tileID].voisins) { // On pourrait parcourir les voisinsVisibles
        // Si ce voisin l'a en voisin mystérieux, on le lui enlève
        tiles[voisin].removeMysterieux(tile.tileID);
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
    if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
        portes[object.objectID] = object;
        if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) { // c'est une porte fenetre
            if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
                if (isInMap(voisin1))
                    tiles[voisin1].removeAccessible(voisin2);
                if (isInMap(voisin2))
                    tiles[voisin2].removeAccessible(voisin1);
            } else {
                // Si la porte est ouverte on est accessible ET visible ! =)
            }
        } else { // C'est une porte point.
            if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
                if (isInMap(voisin1)) {
                    tiles[voisin1].removeAccessible(voisin2);
                    tiles[voisin1].removeVisible(voisin2);
                }
                if (isInMap(voisin2)) {
                    tiles[voisin2].removeAccessible(voisin1);
                    tiles[voisin2].removeVisible(voisin1);
                }
            } else {
                // Si la porte est ouverte on est accessible ET visible ! =)
            }
        }
    }
    if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
        fenetres[object.objectID] = object;
        if (isInMap(voisin1))
            tiles[voisin1].removeAccessible(voisin2);
        if (isInMap(voisin2))
            tiles[voisin2].removeAccessible(voisin1);
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
