
#include "Map.h"
#include "MapTile.h"
#include "GameManager.h"
#include "Globals.h"
#include <map>
#include <algorithm>
using namespace std;

Map::Map(const LevelInfo levelInfo) :
    rowCount{ levelInfo.rowCount },
    colCount{ levelInfo.colCount },
    nbTiles{ static_cast<int>(levelInfo.tiles.size()) },
    tiles{},
    murs{},
    fenetres{},
    portes{},
    activateurs{}
{
    // Cr�er toutes les tiles !
    for (auto pair_t : levelInfo.tiles) {
        TileInfo t = pair_t.second;
        MapTile mt = MapTile(t, rowCount, colCount);
        tiles[t.tileID] = mt;
    }

    // Enregistrer les objets
    for (auto object : levelInfo.objects) {
        // Si notre objet est une porte ...
        if (object.second.objectTypes.find(Object::ObjectType_Wall) != object.second.objectTypes.end()) {
            murs[object.second.objectID] = object.second;
        }
        if (object.second.objectTypes.find(Object::ObjectType_Door) != object.second.objectTypes.end()) {
            portes[object.second.objectID] = object.second;
        }
        if (object.second.objectTypes.find(Object::ObjectType_Window) != object.second.objectTypes.end()) {
            fenetres[object.second.objectID] = object.second;
        }
        if (object.second.objectTypes.find(Object::ObjectType_PressurePlate) != object.second.objectTypes.end()) {
            activateurs[object.second.objectID] = object.second;
        }
    }

    // Associer tous les voisins de toutes les tiles
    for (auto pair_t : tiles) {
        MapTile t = pair_t.second;
        t.setVoisins(*this);
        tiles[t.id] = t;

        // On enregistre tous les objectifs !
        if (t.type == Tile::TileAttribute_Goal) {
            objectifs[t.id] = t;
        }
    }
}

bool Map::isInMap(int idTile) const {
    return idTile >= 0 && idTile < rowCount * colCount;
}

map<unsigned int, MapTile> Map::getObjectifs() {
    return objectifs;
}

Chemin Map::chemin(int depart, int arrivee) {
    Chemin path; // le chemin final
    MapTile currentTile = tiles[depart];
    vector<int> visitees; // Les cases que l'on a d�j� d�couvertes
    vector<int> toVisit; // Les cases qu'il faut visiter !
    vector<int> toVisitAntecedants; // Les cases qu'il faut visiter !
    vector<float> toVisitDistance; // La distance L1 � l'arriv�e
    vector<int> antecedants; // � partir de quelle case on l'a d�courverte

    // On marque le point de d�part
    visitees.push_back(depart);
    antecedants.push_back(depart);

    // Et on ajoute ses voisins accessibles � la liste des cases � visiter !
    for (auto voisin : tiles[depart].voisinsAccessibles) {
        toVisit.push_back(voisin);
        toVisitAntecedants.push_back(depart);
        toVisitDistance.push_back(distanceL2(voisin, arrivee));
    }

    // On trie, pour a*
    sortByDistance(toVisitDistance, toVisit, toVisitAntecedants);

    // Tant qu'il reste des voisins � visiter
    while (currentTile.id != arrivee && !toVisit.empty()) {
        // Que l'on prenne la premi�re ou la derni�re �a ne change rien !
        // On prends la premi�re case � visiter
        currentTile = tiles[toVisit.back()]; // On prend la DERNIERE case !
        toVisit.pop_back();
        antecedants.push_back(toVisitAntecedants.back()); // On retient l'antecedant !
        toVisitAntecedants.pop_back();
        toVisitDistance.pop_back();

        // On la marque
        visitees.push_back(currentTile.id);

        // On r�cup�re tous ses voisins, et on les ajoutes � la liste de lecture s'ils ne sont pas encore marqu�s ! =)
        // Ou qu'ils ne sont pas d�j� pr�vu d'�tre visit�s !!!
        for (auto voisin : currentTile.voisinsAccessibles) {
            bool estMarque = false;
            for (auto visitee : visitees) {
                if (visitee == voisin) {
                    estMarque = true;
                    break;
                }
            }
            for (auto visitee : toVisit) {
                if (visitee == voisin) {
                    estMarque = true;
                    break;
                }
            }
            // Du coup si on est pas d�j� marqu�
            if (!estMarque) {
                // On l'ajoute � la liste !
                toVisit.push_back(voisin);
                toVisitAntecedants.push_back(currentTile.id);
                toVisitDistance.push_back(distanceL2(voisin, arrivee));
            }
        }

        // On trie, pour a*

        sortByDistance(toVisitDistance, toVisit, toVisitAntecedants);
    }

    // On regarde si on a trouv� l'arriv�e :
    if (currentTile.id == arrivee) {
        // On a plus qu'� construire le chemin !
        while (currentTile.id != depart) {
            // On enregistre la case
            path.chemin.push_back(currentTile.id);


            // Faut trouver son indice dans le vecteur ...
            int indice;
            for (int i = 0; i < visitees.size(); i++) {
                if (visitees[i] == currentTile.id) {
                    indice = i;
                    break;
                }
            }

            // Puis on passe � son ant�c�dante
            currentTile = tiles[antecedants[indice]];
        }
    }
    else {
        GameManager::Log("On ne peut pas acc�der � cette tile ! (chemin = " + to_string(depart) + ", " + to_string(arrivee) + ")");
        path.setInaccessible();
    }

    return path;
}

bool Map::areAccessible(int ind1, int ind2) {
    // On v�rifie que les indices sont � cot� l'un de l'autre !
    MapTile mt1 = tiles[ind1];
    bool pair = (mt1.y % 2 == 0);
    if (pair) {
        if (!(ind2 == ind1 - colCount // NE
            || ind2 == ind1 + 1 // E
            || ind2 == ind1 + colCount // SE
            || ind2 == ind1 + colCount - 1 // SW
            || ind2 == ind1 - 1 // W
            || ind2 == ind1 - colCount - 1)) { // NW
            return false;
        }
    } else { // impair
        if (!(ind2 == ind1 - colCount + 1 // NE
            || ind2 == ind1 + 1 // E
            || ind2 == ind1 + colCount + 1 // SE
            || ind2 == ind1 + colCount // SW
            || ind2 == ind1 - 1 // W
            || ind2 == ind1 - colCount)) { // NW
            return false;
        }
    }

    // On v�rifie �galement que la case d'indice 2 n'est pas une case bloqu� !
    if (tiles[ind2].type == Tile::TileAttribute_Forbidden) {
        return false;
    }

    // Puis on v�rifie qu'il n'y a pas de murs, de porte ferm�e, et de fenetres entre eux !
    // Il faut v�rifier la pr�sence des objets sur les 2 tiles !
    // Murs
    for (auto mur : murs) {
        if (mur.second.tileID == ind1 && mur.second.position == getDirection(ind1, ind2)) {
            return false;
        }
        if (mur.second.tileID == ind2 && mur.second.position == getDirection(ind2, ind1)) {
            return false;
        }
    }
    // Portes
    for (auto porte : portes) {
        if (porte.second.tileID == ind1 && porte.second.position == getDirection(ind1, ind2)
            && porte.second.objectStates.find(Object::ObjectState_Closed) != porte.second.objectStates.end()) {
            return false;
        }
        if (porte.second.tileID == ind2 && porte.second.position == getDirection(ind2, ind1)
            && porte.second.objectStates.find(Object::ObjectState_Closed) != porte.second.objectStates.end()) {
            return false;
        }
    }
    // Fenetres
    for (auto fenetre : fenetres) {
        if (fenetre.second.tileID == ind1 && fenetre.second.position == getDirection(ind1, ind2)) {
            return false;
        }
        if (fenetre.second.tileID == ind2 && fenetre.second.position == getDirection(ind2, ind1)) {
            return false;
        }
    }
    return true;
}

Tile::ETilePosition Map::getDirection(int ind1, int ind2) {
    MapTile mt1 = tiles[ind1];
    bool pair = (mt1.y % 2 == 0);
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

int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) {
    int y = tileSource / colCount;
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
        GameManager::Log("La direction demand� dans getAdjacentTileAt n'existe pas !");
        GameManager::Log("origin = " + to_string(tileSource) + " direction = " + to_string(direction));
        return -1;
    }
}

float Map::distanceL2(int depart, int arrivee) {
    int xd = depart % colCount;
    int yd = depart / colCount;
    int xa = arrivee % colCount;
    int ya = arrivee / colCount;
    return (float)sqrt(pow(xd - xa, 2) + pow(yd - ya, 2));
}

void Map::sortByDistance(vector<float>& base, vector<int>& autre1, vector<int>& autre2) {
    // On va vouloir trier base dans l'ordre d�croissant (la plus petite valeur en derni�re)
    // Puis retenir la permutation
    // Et l'appliquer aux deux autres vecteurs =)

    // Les index dans l'ordre
    vector<int> index(base.size(), 0);
    for (int i = 0; i < index.size(); i++) {
        index[i] = i;
    }

    // La permutation � appliquer � base
    sort(index.begin(), index.end(),
        [&](const int& a, const int& b) {
            return (base[a] > base[b]); // d�croissant
    });

    // On applique la permutation � base, autre1 et autre2
    vector<float> basebis = base;
    vector<int> autre1bis = autre1;
    vector<int> autre2bis = autre2;
    for (int i = 0; i < index.size(); i++) {
        basebis[i] = base[index[i]];
        autre1bis[i] = autre1[index[i]];
        autre2bis[i] = autre2[index[i]];
    }
    base = basebis;
    autre1 = autre1bis;
    autre2 = autre2bis;
}

int Map::tailleCheminMax() {
    return colCount * rowCount + 1;
}
