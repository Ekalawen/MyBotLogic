
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

bool Map::isInMap(int idTile) const noexcept {
    return idTile >= 0 && idTile < rowCount * colCount;
}

map<unsigned int, MapTile> Map::getObjectifs() const noexcept {
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
Chemin Map::WAStar(int depart, int arrivee, float coefEvaluation) noexcept {
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
            if (tiles.find(voisin) != tiles.end()) {
                // On construit le nouveau noeud
                Noeud nouveauNoeud = Noeud(tiles[voisin], noeudCourant.cout + 1, distanceL2(voisin, arrivee), noeudCourant.tile.id);
                // On vérifie s'il existe dans closedList avec un cout inférieur ou dans openList avec un cout inférieur
                auto itClose = find(closedList.begin(), closedList.end(), nouveauNoeud);
                    //[nouveauNoeud](Noeud n) {return n.tile.id == nouveauNoeud.tile.id; });
                auto itOpen = find(openList.begin(), openList.end(), nouveauNoeud);
                    //[nouveauNoeud](Noeud n) {return n.tile.id == nouveauNoeud.tile.id; });

                if (itClose == closedList.end() && itOpen == openList.end()) {
                    openList.push_back(nouveauNoeud);
                } else if (itClose != closedList.end() && itOpen == openList.end()) {
                    //int indice = distance(closedList.begin(), itClose);
                    //closedList[indice] = nouveauNoeud;
                    // (*itClose) = nouveauNoeud;
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

// AStar doit prendre en compte l'heuristique (la distance supposée à l'arrivée) et le coût (la distance réelle au départ)
// pour chaque case pour être sur que l'algorithme trouve réellement un des chemins optimaux ! <3
Chemin Map::aStar(int depart, int arrivee) noexcept {
    Chemin path; // le chemin final
    MapTile currentTile = tiles[depart];
    vector<int> visitees; // Les cases que l'on a déjà découvertes
    vector<int> toVisit; // Les cases qu'il faut visiter !
    vector<int> toVisitAntecedants; // Les cases qu'il faut visiter !
    vector<tuple<int, float>> toVisitCoutPoids; // En premier le le cout, puis le poids (cad la somme cout + heuristique)
    vector<int> antecedants; // à partir de quelle case on l'a décourverte

    // On marque le point de départ
    visitees.push_back(depart);
    antecedants.push_back(depart);

    // Et on ajoute ses voisins accessibles à la liste des cases à visiter !
    for (auto voisin : tiles[depart].voisinsAccessibles) {
        if (tiles.find(voisin) != tiles.end()) {
            toVisit.push_back(voisin);
            toVisitAntecedants.push_back(depart);
            float heuristique = distanceL2(voisin, arrivee);
            int cout = 1; // Ce sont les premiers voisins !
            toVisitCoutPoids.push_back(tuple<int, float>(cout, cout + heuristique));
        }
    }

    // On trie, pour a*
    sortByDistance(toVisitCoutPoids, toVisit, toVisitAntecedants);

    // Tant qu'il reste des voisins à visiter
    while (currentTile.id != arrivee && !toVisit.empty()) {
        // Que l'on prenne la première ou la dernière ça ne change rien !
        // On prends la première case à visiter
        currentTile = tiles[toVisit.back()]; // On prend la DERNIERE case !
        toVisit.pop_back();
        antecedants.push_back(toVisitAntecedants.back()); // On retient l'antecedant !
        toVisitAntecedants.pop_back();
        int coutCurrentTile = get<int>(toVisitCoutPoids.back()); // On retient le cout de la case avant de la supprimer !
        toVisitCoutPoids.pop_back();

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
                  float heuristique = distanceL2(voisin, arrivee);
                  int cout = coutCurrentTile + 1;
                  GameManager::Log("voisinID = " + to_string(voisin));
                  GameManager::Log("cout = " + to_string(cout));
                  GameManager::Log("heuristique = " + to_string(heuristique));
                  GameManager::Log("POIDS = " + to_string(cout + heuristique));
                  toVisitCoutPoids.push_back(tuple<int, float>(cout, cout + heuristique));
              }
           }
        }

        // On trie, pour a*

        sortByDistance(toVisitCoutPoids, toVisit, toVisitAntecedants);
    }

    // On regarde si on a trouvé l'arrivée :
    if (currentTile.id == arrivee) {
        // On a plus qu'à construire le chemin !
        while (currentTile.id != depart) {
            // On enregistre la case
            path.chemin.push_back(currentTile.id);

            // Faut trouver son indice dans le vecteur ...
            int indice;
            for (int i = 0; i < visitees.size(); ++i) {
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
        // GameManager::Log("On ne peut pas accéder à cette tile ! (chemin = " + to_string(depart) + ", " + to_string(arrivee) + ")");
        path.setInaccessible();
    }

    return path;
}

bool Map::areAccessible(int ind1, int ind2) noexcept {
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

bool Map::areVisible(int ind1, int ind2) const noexcept {
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

bool Map::areMysterious(int ind1, int ind2) noexcept {
    if (areAccessible(ind1, ind2) || areVisible(ind1, ind2)) {
        if (tiles.find(ind2) == tiles.end()) {
            return true;
        }
    }
    return false;
}

bool Map::areMysteriousAccessible(int ind1, int ind2) noexcept {
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
        GameManager::Log("La direction demandé dans getAdjacentTileAt n'existe pas !");
        GameManager::Log("origin = " + to_string(tileSource) + " direction = " + to_string(direction));
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
    if(tile.tileType == Tile::TileAttribute_Goal) {
        tiles[tile.tileID].type = Tile::TileAttribute_Goal;
        objectifs[tile.tileID] = tiles[tile.tileID];
    }

    // On le note !
    GameManager::Log("Decouverte de la tile " + to_string(tile.tileID));
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Map::addObject(ObjectInfo object) noexcept {
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
