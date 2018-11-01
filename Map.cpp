
#include "Map.h"
#include "Map.h"
#include "GameManager.h"

#include <algorithm>
#include <chrono>
#include <sstream>

Map::Map(const LevelInfo& _levelInfo) :
    rowCount{ _levelInfo.rowCount },
    colCount{ _levelInfo.colCount },
    nbTilesDecouvertes{ 0 }
{
    // Créer toutes les tiles !
    tiles.reserve(getNbTiles());
    // Créer toutes les tiles !
    tiles.reserve(getNbTiles());
    int offset = 1;
    for (auto i = 0; i < getRowCount(); ++i)
    {
        offset -= !(i & 1);
        unsigned int indexI = i * getColCount();

        for (auto j = 0; j < getColCount(); ++j)
        {
            tiles.emplace_back(
                indexI + j,
                MapPosition{ offset + j,  i }
            );
        }
    }

    // Connecte toutes les tuiles!
    for (auto& tuile : tiles)
    {
        // On test chaque voisin possible
        for (auto direction = 0; direction < MapTile::NB_VOISINS_TOTAL; ++direction)
        {
            auto voisinPositionOffset = tuile.getPosition().CalculerPositionOffset(static_cast<Tile::ETilePosition>(direction));

            // Si la position est définit dans la carte...
            if (estDefiniDansCarte(voisinPositionOffset))
            {
                //On le rajoute comme voisin
                tuile.getVoisins().emplace_back(ConvertirPositionOffsetAID(voisinPositionOffset), static_cast<Tile::ETilePosition>(direction));
            }
        }
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

bool Map::isInMap(const int idTile) const noexcept {
    return idTile > -1 && idTile < rowCount * colCount;
}

bool Map::estDefiniDansCarte(const MapPosition& _position) const noexcept {
    return !(_position.x < 0 || _position.x > getColCount() - 1 || _position.z < 0 || _position.z > getRowCount() - 1);
}

int Map::ConvertirPositionOffsetAID(const MapPosition& _position) const noexcept {
    return _position.z * getColCount() + _position.x;
}


struct Noeud {
    static float coefEvaluation;
    MapTile tile;
    float cout; // La distance calculé depuis le départ
    float evaluation; // La distance estimée à l'arrivée
    float heuristique; // La somme du cout et de l'evaluation
    int idPrecedant;
    Noeud() = default;
    Noeud(const MapTile& tile, const float cout, const float evaluation, const int idPrecedant)
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
Chemin Map::aStar(const int depart, const int arrivee, const float coefEvaluation) const noexcept {
    Noeud::coefEvaluation = coefEvaluation;
    // On crée nos liste et notre noeud courrant
    vector<Noeud> closedList{};
    vector<Noeud> openList{};
    Noeud noeudCourant = Noeud(tiles[depart], 0, distanceReel(depart, arrivee), depart);
    Chemin path;

    // On ajoute le noeud initial
    openList.push_back(noeudCourant);
    // Tant qu'il reste des noeuds à traiter ...
    while (!openList.empty() && noeudCourant.tile.getId() != arrivee) {
        // On récupère le premier noeud de notre liste
        noeudCourant = openList.back();
        openList.pop_back();
        // Pour tous les voisins du noeud courant ...
        for (auto voisinID : noeudCourant.tile.getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
            // On vérifie que le voisin existe ...
            if (tiles[voisinID].existe()) {
                // On construit le nouveau noeud
                Noeud nouveauNoeud = Noeud(tiles[voisinID], noeudCourant.cout + 1, distanceReel(voisinID, arrivee), noeudCourant.tile.getId());
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

Tile::ETilePosition Map::getDirection(const int _tuileID, const int _voisinID) const noexcept {
    auto voisins = tiles[_tuileID].getVoisins();
    auto result = std::find_if(begin(voisins), end(voisins), [&_voisinID](const Voisin& v) {
        return v.getTuileIndex() == _voisinID;
    });

    if (result != end(voisins)) {
        return result->getDirection();
    }

    GameManager::Log("Erreur dans l'appel de getDirection()!");
    return Tile::CENTER;
}

int Map::getAdjacentTileAt(const int tileSource, const Tile::ETilePosition direction) const noexcept {
    auto voisins = tiles[tileSource].getVoisins();
    auto result = std::find_if(begin(voisins), end(voisins), [&direction](const Voisin& v) {
        return v.getDirection() == direction;
    });

    if (result != end(voisins)) {
        return result->getTuileIndex();
    }

    GameManager::Log("Erreur dans l'appel de getAdjacentTileAt() !");
    return -1;
}

float Map::distanceReel(const int _depart, const int _fin) const noexcept {
    return tiles[_depart].getPosition().DistancreReelEntre(tiles[_fin].getPosition());
}

int Map::distanceNbTuiles(const int _depart, const int _fin) const noexcept {
    return tiles[_depart].getPosition().NbTuilesEntre(tiles[_fin].getPosition());
}

int Map::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Map::addTile(const TileInfo& tile) noexcept {
    // On met à jour le nombre de tiles
    ++nbTilesDecouvertes;

    // On la rajoute aux tiles
    tiles[tile.tileID].setTileDecouverte(tile);

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Goal) {
        objectifs.push_back(tile.tileID);
    }

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Forbidden) {
        for (auto voisin : tiles[tile.tileID].getVoisins()) {
            tiles[voisin.getTuileIndex()].removeEtat(Etats::ACCESSIBLE, tile.tileID);
        }
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisinID : tiles[tile.tileID].getVoisinsIDParEtat(Etats::VISIBLE)) {
        // Si ce voisin l'a en voisin mystérieux, on le lui enlève
        tiles[voisinID].removeEtat(Etats::MYSTERIEUX, tile.tileID);
    }

    // On le note !
    stringstream ss;
    ss << "Decouverte de la tile " << tile.tileID;
    GameManager::Log(ss.str());
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Map::addObject(const ObjectInfo& object) noexcept {
    int voisin1 = object.tileID;
	int voisin2 = getAdjacentTileAt(object.tileID, object.position);

    // On ajoute notre objet à l'ensemble de nos objets
    if (object.objectTypes.find(Object::ObjectType_Wall) != object.objectTypes.end()) {
       // Fenetre
       if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
          fenetres[object.objectID] = object;
          if (isInMap(voisin1))
             tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
          if (isInMap(voisin2))
             tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
       // Mur
       } else {
          murs[object.objectID] = object;
          if (isInMap(voisin1)) {
             tiles[voisin1].removeEtat(Etats::MYSTERIEUX, voisin2);
             tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
             tiles[voisin1].removeEtat(Etats::VISIBLE, voisin2);
          }
          if (isInMap(voisin2)) {
             tiles[voisin2].removeEtat(Etats::MYSTERIEUX, voisin1);
             tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
             tiles[voisin2].removeEtat(Etats::VISIBLE, voisin1);
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
                 tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
              if (isInMap(voisin2))
                 tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
            // Porte
           } else {
              if (isInMap(voisin1)) {
                 tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
                 tiles[voisin1].removeEtat(Etats::VISIBLE, voisin2);
              }
              if (isInMap(voisin2)) {
                 tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
                 tiles[voisin2].removeEtat(Etats::VISIBLE, voisin1);
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

    stringstream ss;
    ss << "Decouverte de l'objet " << object.objectID << " sur la tuile " << object.tileID << " orienté en " << object.position;
    GameManager::Log(ss.str());
}

int Map::getX(const int id) const noexcept {
    return id % colCount;
}
int Map::getY(const int id) const noexcept {
    return id / colCount;
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

MapTile& Map::getTile(const int id) {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

const MapTile& Map::getTile(const int id) const {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

vector<unsigned int> Map::getObjectifs() const noexcept {
    return objectifs;
}

vector<unsigned int>& Map::getObjectifs() {
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

bool Map::objectExist(const int objet) const noexcept {
    return murs.find(objet) != murs.end()
        || portes.find(objet) != portes.end()
        || fenetres.find(objet) != fenetres.end()
        || activateurs.find(objet) != activateurs.end();
}
