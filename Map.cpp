
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
    nbTiles{ rowCount * colCount },
    nbtilesDecouvertes{ static_cast<int>(levelInfo.tiles.size()) },
    tiles{},
    murs{},
    fenetres{},
    portes{},
    activateurs{}
{
    // Créer toutes les tiles !
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

Chemin Map::aStar(int depart, int arrivee) {
    Chemin path; // le chemin final
    MapTile currentTile = tiles[depart];
    vector<int> visitees; // Les cases que l'on a déjà découvertes
    vector<int> toVisit; // Les cases qu'il faut visiter !
    vector<int> toVisitAntecedants; // Les cases qu'il faut visiter !
    vector<float> toVisitDistance; // La distance L1 à l'arrivée
    vector<int> antecedants; // à partir de quelle case on l'a décourverte

    // On marque le point de départ
    visitees.push_back(depart);
    antecedants.push_back(depart);

    // Et on ajoute ses voisins accessibles à la liste des cases à visiter !
    for (auto voisin : tiles[depart].voisinsAccessibles) {
        if (tiles.find(voisin) != tiles.end()) {
            toVisit.push_back(voisin);
            toVisitAntecedants.push_back(depart);
            toVisitDistance.push_back(distanceL2(voisin, arrivee));
        }
    }

    // On trie, pour a*
    sortByDistance(toVisitDistance, toVisit, toVisitAntecedants);

    // Tant qu'il reste des voisins à visiter
    while (currentTile.id != arrivee && !toVisit.empty()) {
        // Que l'on prenne la première ou la dernière ça ne change rien !
        // On prends la première case à visiter
        currentTile = tiles[toVisit.back()]; // On prend la DERNIERE case !
        toVisit.pop_back();
        antecedants.push_back(toVisitAntecedants.back()); // On retient l'antecedant !
        toVisitAntecedants.pop_back();
        toVisitDistance.pop_back();

        // On la marque
        visitees.push_back(currentTile.id);

        // On récupère tous ses voisins, et on les ajoutes à la liste de lecture s'ils ne sont pas encore marqués ! =)
        // Ou qu'ils ne sont pas déjà prévu d'être visités !!!
        for (auto voisin : currentTile.voisinsAccessibles) {
           // Si notre voisin n'appartient pas aux cases visitées ou à visiter, alors on l'ajoute à cette dernière
           if (!(find(visitees.begin(), visitees.end(), voisin) != visitees.end()) && !(find(toVisit.begin(), toVisit.end(), voisin) != toVisit.end())) {
              // On l'ajoute que si cette case a déjà été découverte !
              if (tiles.find(voisin) != tiles.end()) {
                  toVisit.push_back(voisin);
                  toVisitAntecedants.push_back(currentTile.id);
                  toVisitDistance.push_back(distanceL2(voisin, arrivee));
              }
           }
        }

        // On trie, pour a*

        sortByDistance(toVisitDistance, toVisit, toVisitAntecedants);
    }

    // On regarde si on a trouvé l'arrivée :
    if (currentTile.id == arrivee) {
        // On a plus qu'à construire le chemin !
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

            // Puis on passe à son antécédante
            currentTile = tiles[antecedants[indice]];
        }
    }
    else {
        GameManager::Log("On ne peut pas accéder à cette tile ! (chemin = " + to_string(depart) + ", " + to_string(arrivee) + ")");
        path.setInaccessible();
    }

    return path;
}

bool Map::areAccessible(int ind1, int ind2) {
    int y = getY(ind1);
    bool pair = (y % 2 == 0);
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

    // On vérifie également que la case d'indice 2 n'est pas une case bloqué !
    if (tiles.find(ind2) != tiles.end()
    && tiles[ind2].type == Tile::TileAttribute_Forbidden) {
        return false;
    }

    // Puis on vérifie qu'il n'y a pas de murs, de porte fermée, et de fenetres entre eux !
    // Il faut vérifier la présence des objets sur les 2 tiles !
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

bool Map::areVisible(int ind1, int ind2) {
	// On vérifie que les indices sont à coté l'un de l'autre !
    int y = getY(ind1);
	bool pair = (y % 2 == 0);
	if (pair) {
		if (!(ind2 == ind1 - colCount // NE
			|| ind2 == ind1 + 1 // E
			|| ind2 == ind1 + colCount // SE
			|| ind2 == ind1 + colCount - 1 // SW
			|| ind2 == ind1 - 1 // W
			|| ind2 == ind1 - colCount - 1)) { // NW
			return false;
		}
	}
	else { // impair
		if (!(ind2 == ind1 - colCount + 1 // NE
			|| ind2 == ind1 + 1 // E
			|| ind2 == ind1 + colCount + 1 // SE
			|| ind2 == ind1 + colCount // SW
			|| ind2 == ind1 - 1 // W
			|| ind2 == ind1 - colCount)) { // NW
			return false;
		}
	}

	// Puis on vérifie qu'il n'y a pas de murs, de porte fermée entre eux !
	// Il faut vérifier la présence des objets sur les 2 tiles !
	// Mur
	for (auto mur : murs) {
		if (mur.second.tileID == ind1 && mur.second.position == getDirection(ind1, ind2)) {
			return false;
		}
		if (mur.second.tileID == ind2 && mur.second.position == getDirection(ind2, ind1)) {
			return false;
		}
	}
	// Porte fermée
	for (auto porte : portes) {
        if (find(porte.second.objectTypes.begin(), porte.second.objectTypes.end(), Object::ObjectType_Window) == porte.second.objectTypes.end()) { // Non fenêtré ! :D
            if (porte.second.tileID == ind1 && porte.second.position == getDirection(ind1, ind2)
                && porte.second.objectStates.find(Object::ObjectState_Closed) != porte.second.objectStates.end()) {
                return false;
            }
            if (porte.second.tileID == ind2 && porte.second.position == getDirection(ind2, ind1)
                && porte.second.objectStates.find(Object::ObjectState_Closed) != porte.second.objectStates.end()) {
                return false;
            }
        }
	}

	return true;
}

bool Map::areMysterious(int ind1, int ind2) {
    if (areAccessible(ind1, ind2) || areVisible(ind1, ind2)) {
        if (tiles.find(ind2) == tiles.end()) {
            return true;
        }
    }
    return false;
}

bool Map::areMysteriousAccessible(int ind1, int ind2)
{
	// On vérifie que les indices sont à coté l'un de l'autre !
    int y = getY(ind1);
	bool pair = (y % 2 == 0);
	if (pair) {
		if (!(ind2 == ind1 - colCount // NE
			|| ind2 == ind1 + 1 // E
			|| ind2 == ind1 + colCount // SE
			|| ind2 == ind1 + colCount - 1 // SW
			|| ind2 == ind1 - 1 // W
			|| ind2 == ind1 - colCount - 1)) { // NW
			return false;
		}
	}
	else { // impair
		if (!(ind2 == ind1 - colCount + 1 // NE
			|| ind2 == ind1 + 1 // E
			|| ind2 == ind1 + colCount + 1 // SE
			|| ind2 == ind1 + colCount // SW
			|| ind2 == ind1 - 1 // W
			|| ind2 == ind1 - colCount)) { // NW
			return false;
		}
	}

	// On vérifie également que la case d'indice 2 n'est pas une case bloqué !
	if (tiles.find(ind2) != tiles.end()
    && tiles[ind2].type == Tile::TileAttribute_Forbidden) {
		return false;
	}

	// Puis on vérifie qu'il n'y a pas de murs, de porte fermée entre eux !
	// Il faut vérifier la présence des objets sur les 2 tiles !
	// Mur
	for (auto mur : murs) {
		if (mur.second.tileID == ind1 && mur.second.position == getDirection(ind1, ind2)) {
			return false;
		}
		if (mur.second.tileID == ind2 && mur.second.position == getDirection(ind2, ind1)) {
			return false;
		}
	}
	// Porte fermée
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

int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) {
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
        GameManager::Log("La direction demandé dans getAdjacentTileAt n'existe pas !");
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

int Map::distanceHex(int tile1ID, int tile2ID) {
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

void Map::sortByDistance(vector<float>& base, vector<int>& autre1, vector<int>& autre2) {
    // On va vouloir trier base dans l'ordre décroissant (la plus petite valeur en dernière)
    // Puis retenir la permutation
    // Et l'appliquer aux deux autres vecteurs =)

    // Les index dans l'ordre
    vector<int> index(base.size(), 0);
    for (int i = 0; i < index.size(); i++) {
        index[i] = i;
    }

    // La permutation à appliquer à base
    sort(index.begin(), index.end(),
        [&](const int& a, const int& b) {
            return (base[a] > base[b]); // décroissant
    });

    // On applique la permutation à base, autre1 et autre2
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

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Map::addTile(TileInfo tile) {
    // On met à jour le nombre de tiles
    nbtilesDecouvertes++;

    // On la rajoute aux tiles
    tiles[tile.tileID] = MapTile(tile, rowCount, colCount);

    // On met à jour ses voisins
    if (tiles.find(tile.tileID) != tiles.end()) {
        tiles[tile.tileID].setVoisins(*this);
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisin : tiles[tile.tileID].voisins) {
        // Il faut vérifier que cette tile existe déjà ! Canard !
        if (tiles.find(voisin) != tiles.end()) {
            tiles[voisin].setVoisins(*this);
        }
    }

    // Si c'est un objectif, on le retient !
    if (tiles[tile.tileID].type == Tile::TileAttribute_Goal) {
        objectifs[tile.tileID] = tiles[tile.tileID];
    }

    // On le note !
    GameManager::Log("Decouverte de la tile " + to_string(tile.tileID));
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Map::addObject(ObjectInfo object) {
    // On ajoute notre objet à l'ensemble de nos objets
    if (object.objectTypes.find(Object::ObjectType_Wall) != object.objectTypes.end()) {
        murs[object.objectID] = object;
    }
    if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
        portes[object.objectID] = object;
    }
    if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
        fenetres[object.objectID] = object;
    }
    if (object.objectTypes.find(Object::ObjectType_PressurePlate) != object.objectTypes.end()) {
        activateurs[object.objectID] = object;
    }
    
    // Puis on met à jour les voisins de la case de notre objet
    if (tiles.find(object.tileID) != tiles.end()) {
        tiles[object.tileID].setVoisins(*this);
    }

    // Puis on met à jour les voisins des voisins de la case de notre objet
    for (auto voisin : getVoisins(object.tileID)) {
        if (tiles.find(voisin) != tiles.end()) {
            tiles[voisin].setVoisins(*this);
        }
    }

    // On le note !
    GameManager::Log("Decouverte de l'objet " + to_string(object.objectID) + " sur la tuile " + to_string(object.tileID) + " orienté en " + to_string(object.position));
}

int Map::getX(int id) {
    return id % colCount;
}
int Map::getY(int id) {
    return id / colCount;
}

vector<int> Map::getVoisins(int id) {
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
