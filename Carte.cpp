
#include "Carte.h"
#include "MapTile.h"
#include "GameManager.h"
#include "Globals.h"
#include <map>
#include <algorithm>
#include <chrono>

Carte::Carte(const LevelInfo _levelInfo) :
    rowCount{ _levelInfo.rowCount },
    colCount{ _levelInfo.colCount },
    nbTilesDecouvertes{ 0 },
	tiles{ std::vector<MapTile>{} },
	murs{ std::map<unsigned int, ObjectInfo>{} },
	fenetres{ std::map<unsigned int, ObjectInfo>{} },
	portes{ std::map<unsigned int, ObjectInfo>{} },
	activateurs{ std::map<unsigned int, ObjectInfo>{} }
{
    // Créer toutes les tiles !
    tiles.reserve(getNbTiles());
    for (int id = 0; id < getNbTiles(); ++id) {
       tiles.push_back(MapTile(id, *this));
    }

    // Mettre à jour les tiles connues
    for (auto tile : _levelInfo.tiles) {
        addTile(tile.second);
    }

    // Enregistrer les objets
    for (auto object : _levelInfo.objects) {
        addObject(object.second);
    }

    // Mettre à visiter les cases initiales des NPCs
    for (auto pair_npc : _levelInfo.npcs) {
        tiles[pair_npc.second.tileID].setStatut(MapTile::Statut::VISITE);
    }
}

bool Carte::isInMap(int _idTile) const noexcept {
    return _idTile >= 0 && _idTile < rowCount * colCount;
}

std::vector<unsigned int> Carte::getObjectifs() const noexcept {
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
    Noeud(MapTile _tile, float _cout, float _evaluation, int _idPrecedant)
        : tile{ _tile }, cout{ _cout }, evaluation{ _evaluation }, idPrecedant{ _idPrecedant } {
        heuristique = _cout + _evaluation * coefEvaluation;
    }
    friend bool operator==(const Noeud& _left, const Noeud& _right) {
        return _left.tile.getId() == _right.tile.getId();
    }
};
float Noeud::coefEvaluation = 1;

// Il s'agit de l'algorithme AStar auquel on peut rajouter un coefficiant à l'évaluation pour modifier l'heuristique.
// Par défaut sa valeur est 1. Si on l'augmente l'algorithme ira plus vite au détriment de trouver un chemin optimal.
// Si on le diminue l'algorithme se rapproche de plus en plus d'un parcours en largeur.
Chemin Carte::aStar(int _depart, int _arrivee, float _coefEvaluation) noexcept {
    Noeud::coefEvaluation = _coefEvaluation;
    // On crée nos liste et notre noeud courrant
    std::vector<Noeud> closedList{};
    std::vector<Noeud> openList{};
    Noeud noeudCourant = Noeud(tiles[_depart], 0, distanceL2(_depart, _arrivee), _depart);
    Chemin path;

    // On ajoute le noeud initial
    openList.push_back(noeudCourant);
    // Tant qu'il reste des noeuds à traiter ...
    while (!openList.empty() && noeudCourant.tile.getId() != _arrivee) {
        // On récupère le premier noeud de notre liste
        noeudCourant = openList.back();
        openList.pop_back();
        // Pour tous les voisins du noeud courant ...
        for (auto voisin : noeudCourant.tile.getVoisinsAccessibles()) {
            // On vérifie que le voisin existe ...
            if (tiles[voisin].existe()) {
                // On construit le nouveau noeud
                Noeud nouveauNoeud = Noeud(tiles[voisin], noeudCourant.cout + 1, distanceL2(voisin, _arrivee), noeudCourant.tile.getId());
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
                    GameManager::log("OMG On a fait n'imp !");
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
    if (noeudCourant.tile.getId() == _arrivee) {
        // Si oui on reconstruit le path !
        while (noeudCourant.tile.getId() != _depart) {
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

Tile::ETilePosition Carte::getDirection(int _tileFrom, int _tileTo) const noexcept {
    int y = getY(_tileFrom);
    bool pair = (y % 2 == 0);
    if (pair) {
        if (_tileTo == _tileFrom - colCount) {
            return Tile::NE;
        } else if (_tileTo == _tileFrom + 1) {
            return Tile::E;
        } else if (_tileTo == _tileFrom + colCount) {
            return Tile::SE;
        } else if (_tileTo == _tileFrom + colCount - 1) {
            return Tile::SW;
        } else if (_tileTo == _tileFrom - 1) {
            return Tile::W;
        } else if (_tileTo == _tileFrom - colCount - 1) {
            return Tile::NW;
        }
    } else {
        if (_tileTo == _tileFrom - colCount + 1) {
            return Tile::NE;
        } else if (_tileTo == _tileFrom + 1) {
            return Tile::E;
        } else if (_tileTo == _tileFrom + colCount + 1) {
            return Tile::SE;
        } else if (_tileTo == _tileFrom + colCount) {
            return Tile::SW;
        } else if (_tileTo == _tileFrom - 1) {
            return Tile::W;
        } else if (_tileTo == _tileFrom - colCount) {
            return Tile::NW;
        }
    }

    GameManager::log("Erreur dans l'appel de getDirection() !");
    return Tile::CENTER;
}

int Carte::getAdjacentTileAt(int _tileSource, Tile::ETilePosition _direction) const noexcept {
    int y = getY(_tileSource);
    bool pair = (y % 2 == 0);
    int res;
    switch (_direction)
    {
    case Tile::NE:
        if (pair) {
            res = _tileSource - colCount;
        } else {
            res = _tileSource - colCount + 1;
        }
        break;
    case Tile::E:
        res = _tileSource + 1;
        break;
    case Tile::SE:
        if (pair) {
            res = _tileSource + colCount;
        } else {
            res = _tileSource + colCount + 1;
        }
        break;
    case Tile::SW:
        if (pair) {
            res = _tileSource + colCount - 1;
        } else {
            res = _tileSource + colCount;
        }
        break;
    case Tile::W:
        res = _tileSource - 1;
        break;
    case Tile::NW:
        if (pair) {
            res = _tileSource - colCount - 1;
        } else {
            res = _tileSource - colCount;
        }
        break;
    case Tile::CENTER:
        res = _tileSource;
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

float Carte::distanceL2(int _depart, int _arrivee) const noexcept {
    int xd = _depart % colCount;
    int yd = _depart / colCount;
    int xa = _arrivee % colCount;
    int ya = _arrivee / colCount;
    return (float)sqrt(pow(xd - xa, 2) + pow(yd - ya, 2));
}

int Carte::distanceHex(int _tile1ID, int _tile2ID) const noexcept {
   int ligne1 = _tile1ID / colCount;
   int colonne1 = _tile1ID % colCount;
   int ligne2 = _tile2ID / colCount;
   int colonne2 = _tile2ID % colCount;
   int x1 = colonne1 - (ligne1 - ligne1 % 2) / 2;
   int z1 = ligne1;
   int y1 = -x1 - z1;
   int x2 = colonne2 - (ligne2 - ligne2 % 2) / 2;
   int z2 = ligne2;
   int y2 = -x2 - z2;
   return std::max(std::max(abs(x1 - x2), abs(y1 - y2)), abs(z1 - z2));
}

int Carte::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Carte::addTile(TileInfo _tile) noexcept {
    // On met à jour le nombre de tiles
    ++nbTilesDecouvertes;

    // On la rajoute aux tiles
    tiles[_tile.tileID].setTileDecouverte(_tile);

    if (tiles[_tile.tileID].getType() == Tile::TileAttribute_Goal) {
        objectifs.push_back(_tile.tileID);
    }

    if (tiles[_tile.tileID].getType() == Tile::TileAttribute_Forbidden) {
        for (auto voisin : tiles[_tile.tileID].getVoisins()) {
            tiles[voisin].removeAccessible(_tile.tileID);
        }
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisin : tiles[_tile.tileID].getVoisins()) { // On pourrait parcourir les voisinsVisibles
        // Si ce voisin l'a en voisin mystérieux, on le lui enlève
        tiles[voisin].removeMysterieux(_tile.tileID);
    }

    // On le note !
    GameManager::log("Decouverte de la tile " + std::to_string(_tile.tileID));
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Carte::addObject(ObjectInfo _object) noexcept {
    int voisin1 = _object.tileID;
	int voisin2 = getAdjacentTileAt(_object.tileID, _object.position);

    // On ajoute notre objet à l'ensemble de nos objets
    if (_object.objectTypes.find(Object::ObjectType_Wall) != _object.objectTypes.end()) {
       // Fenetre
       if (_object.objectTypes.find(Object::ObjectType_Window) != _object.objectTypes.end()) {
          fenetres[_object.objectID] = _object;
          if (isInMap(voisin1))
             tiles[voisin1].removeAccessible(voisin2);
          if (isInMap(voisin2))
             tiles[voisin2].removeAccessible(voisin1);
       // Mur
       } else {
          murs[_object.objectID] = _object;
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
    if (_object.objectTypes.find(Object::ObjectType_Door) != _object.objectTypes.end()) {
        portes[_object.objectID] = _object;
        //Porte Ferme
        if (_object.objectStates.find(Object::ObjectState_Closed) != _object.objectStates.end()) {
           // Porte Fenetre
           if (_object.objectTypes.find(Object::ObjectType_Window) != _object.objectTypes.end()) {
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
    if (_object.objectTypes.find(Object::ObjectType_PressurePlate) != _object.objectTypes.end()) {
        activateurs[_object.objectID] = _object;
        // prout !
    }
    
    // On le note !
    GameManager::log("Decouverte de l'objet " + std::to_string(_object.objectID) + " sur la tuile " + std::to_string(_object.tileID) + " orienté en " + std::to_string(_object.position));
}

int Carte::getX(int _id) const noexcept {
    return _id % colCount;
}
int Carte::getY(int _id) const noexcept {
    return _id / colCount;
}

std::vector<int> Carte::getVoisins(int _id) const noexcept {
   std::vector<int> voisins;
    int x = getX(_id);
    int y = getY(_id);
    int indice;
    if (y % 2 == 0) { // Ligne paire
        // NE
        indice = _id - colCount;
        if (isInMap(indice) && y > 0) {
            voisins.push_back(indice);
        }
        // E
        indice = _id + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // SE
        indice = _id + colCount;
        if (isInMap(indice) && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // SW
        indice = _id + colCount - 1;
        if (isInMap(indice) && y < rowCount-1 && x > 0) {
            voisins.push_back(indice);
        }
        // W
        indice = _id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = _id - colCount - 1;
        if (isInMap(indice) && y > 0 && x > 0) {
            voisins.push_back(indice);
        }

    } else { // Ligne impaire !
        // NE
        indice = _id - colCount + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // E
        indice = _id + 1;
        if (isInMap(indice) && x < colCount-1) {
            voisins.push_back(indice);
        }
        // SE
        indice = _id + colCount + 1;
        if (isInMap(indice) && x < colCount-1 && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // SW
        indice = _id + colCount;
        if (isInMap(indice) && y < rowCount-1) {
            voisins.push_back(indice);
        }
        // W
        indice = _id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = _id - colCount;
        if (isInMap(indice)) { // Pas de conditions, c'est marrant ! =)
            voisins.push_back(indice);
        }
    }

    return voisins;
}

int Carte::getRowCount() const noexcept {
    return rowCount;
}

int Carte::getColCount() const noexcept {
    return colCount;
}

int Carte::getNbTiles() const noexcept {
    return getRowCount() * getColCount();
}

int Carte::getNbTilesDecouvertes() const noexcept {
    return nbTilesDecouvertes;
}

MapTile& Carte::getTile(int _id) {
    if (_id < 0 || _id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[_id];
}

std::vector<unsigned int> Carte::getObjectifs() {
    return objectifs;
}

std::map<unsigned int, ObjectInfo> Carte::getMurs() {
    return murs;
}

std::map<unsigned int, ObjectInfo> Carte::getPortes() {
    return portes;
}

std::map<unsigned int, ObjectInfo> Carte::getFenetres() {
    return fenetres;
}

std::map<unsigned int, ObjectInfo> Carte::getActivateurs() {
    return activateurs;
}

bool Carte::objectExist(int _objet) {
    return murs.find(_objet) != murs.end()
        || portes.find(_objet) != portes.end()
        || fenetres.find(_objet) != fenetres.end()
        || activateurs.find(_objet) != activateurs.end();
}
