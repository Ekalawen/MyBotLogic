
#include "Carte.h"
#include "MapTile.h"
#include "GameManager.h"
#include "Globals.h"
#include "Voisin.h"
#include "Porte.h"
#include "Noeud.h"

#include <algorithm>
#include <chrono>
#include <sstream>
using std::stringstream;
using std::max;

Carte::Carte(const LevelInfo& _levelInfo) :
    rowCount{ _levelInfo.rowCount },
    colCount{ _levelInfo.colCount },
    nbTilesDecouvertes{ 0 }
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

bool Carte::isInMap(const int idTile) const noexcept {
    return idTile > -1 && idTile < rowCount * colCount;
}

vector<unsigned int> Carte::getObjectifs() const noexcept {
    return objectifs;
}


// Il s'agit de l'algorithme AStar auquel on peut rajouter un coefficiant à l'évaluation pour modifier l'heuristique.
// Par défaut sa valeur est 1. Si on l'augmente l'algorithme ira plus vite au détriment de trouver un chemin optimal.
// Si on le diminue l'algorithme se rapproche de plus en plus d'un parcours en largeur.
Chemin Carte::aStar(const int depart, const int arrivee, int npcActif, GameManager& gm, const vector<Contrainte>& contraintesDejaNecessaires) const noexcept {
    Noeud::coefEvaluation = 1.f;
    // On crée nos liste et notre noeud courrant
    vector<Noeud> closedList{};
    vector<Noeud> openList{};
    Noeud noeudCourant = Noeud(tiles[depart], 0, distanceL2(depart, arrivee), depart);
    Chemin path;
    vector<Contrainte> allContraintes{};

    // Il nous faut ajouter toutes les contraintes déjà nécessaires à notre chemin !
    for (Contrainte contrainte : contraintesDejaNecessaires)
        path.addContrainte(contrainte);

    // On ajoute le noeud initial
    openList.push_back(noeudCourant);
    // Tant qu'il reste des noeuds à traiter ...
    while (!openList.empty() && noeudCourant.tile.getId() != arrivee) {
        // On récupère le premier noeud de notre liste
        noeudCourant = openList.back();
        openList.pop_back();
        // Pour tous les voisins du noeud courant ...
        for (int tileVoisineID : noeudCourant.tile.getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
            // On vérifie que le voisin existe ...
            MapTile& voisinTile = gm.c.getTile(noeudCourant.tile.getId());
            if (voisinTile.existe()) {

                // On vérifie que l'on peut passer à travers nos portes
                bool doorOk = true;
                int tempsAvantOuverture = 0;
                vector<Contrainte> contraintesForPassingDoor = contraintesDejaNecessaires;
                if (voisinTile.hasDoor(tileVoisineID, gm.c)) {
                    // On vérifie que on a pas déjà essayé de faire cette contrainte !
                    //vector<Contrainte>::iterator it;
                    //if (!npcsOccupesIds.empty()) {
                    //     it = find_if(contraintesNecessaires.begin(), contraintesNecessaires.end(), [caseAvant = noeudCourant.tile.getId(),
                    //                                                                                 caseApres = voisinID,
                    //                                                                                 npcId = npcsOccupesIds[0]](Contrainte& contrainte) { // Je fais le [0] car pour le moment la liste des npcsOccupes est toujours au maximum de taille 1
                    //        return contrainte.getCaseAvantPorte() == caseAvant && contrainte.getCaseApresPorte() == caseApres && contrainte.getNpcAssocie() == npcId;
                    //    });
                    //}
                    //if (!npcsOccupesIds.empty() || it == contraintesNecessaires.end()) {
                        doorOk = voisinTile.canPassDoor(tileVoisineID, npcActif, noeudCourant.tile.getId(), gm, tempsAvantOuverture, contraintesForPassingDoor);
                        if (doorOk)
                            allContraintes.insert(allContraintes.end(), contraintesForPassingDoor.begin(), contraintesForPassingDoor.end());
                    //} else {
                    //    doorOk = false;
                    //}
                }

                //// On vérifie que on peut passer par les portes ici qui gêneront !
                //// TODO !
                //int nbToursAvantOuverture = 0;
                //if (!getTile(noeudCourant.tile.getId()).hasClosedDoorSwitch(voisinID, *this) // On vérifie qu'il n'y a pas de porte à switch devant
                //    || getTile(noeudCourant.tile.getId()).getContrainte(voisinID, *this).isSolvableWithout(npcsOccupesIds, gm, nbToursAvantOuverture)) { // Où qu'elle n'est pas génante !

                // On construit le nouveau noeud
                if(doorOk) {
                    int nouveauCout = std::max(static_cast<int>(noeudCourant.cout + 1), tempsAvantOuverture);
                    Noeud nouveauNoeud = Noeud(tiles[tileVoisineID], nouveauCout, distanceL2(tileVoisineID, arrivee), noeudCourant.tile.getId());
                    // On vérifie s'il existe dans closedList avec un cout inférieur ou dans openList avec un cout inférieur
                    auto itClose = find(closedList.begin(), closedList.end(), nouveauNoeud);
                    auto itOpen = find(openList.begin(), openList.end(), nouveauNoeud);

                    if (itClose == closedList.end() && itOpen == openList.end()) {
                        openList.push_back(nouveauNoeud);
                    }
                    else if (itClose != closedList.end() && itOpen == openList.end()) {
                        // Do nothing
                    }
                    else if (itClose == closedList.end() && itOpen != openList.end()) {
                        if ((*itOpen).heuristique > nouveauNoeud.heuristique) {
                            (*itOpen) = nouveauNoeud;
                        }
                    }
                    else {
                        GameManager::log("OMG On a fait n'imp !");
                    }
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
            for (Noeud n : closedList) {
                if (n.tile.getId() == noeudCourant.idPrecedant) {

                    // TODO : il faut récupérer les contraintes des aStars intermédiaire également ??

                    // On regarde si on est passé par une contrainte !
                    auto it = find_if(allContraintes.begin(), allContraintes.end(), [noeudCourant, n](const Contrainte& contrainte) {
                        return (contrainte.getCaseAvantPorte() == noeudCourant.tile.getId() && contrainte.getCaseApresPorte() == n.tile.getId())
                            || (contrainte.getCaseApresPorte() == noeudCourant.tile.getId() && contrainte.getCaseAvantPorte() == n.tile.getId());
                    });
                    // Si oui on ajoute nos contraintes à notre chemin !
                    if (it != allContraintes.end()) {
                        path.addContrainte(*it);
                    }

                    // On remet à jour le noeud ...
                    noeudCourant = n;
                    break;
                }
            }

        }

    }
    else {
        // Si non le path est inaccessible !
        path.setInaccessible();
    }

    return path;
}

Tile::ETilePosition Carte::getDirection(const int ind1, const int ind2) const noexcept {
    int y = getY(ind1);
    bool pair = (y % 2 == 0);
    if (pair) {
        if (ind2 == ind1 - colCount) {
            return Tile::NE;
        }
        else if (ind2 == ind1 + 1) {
            return Tile::E;
        }
        else if (ind2 == ind1 + colCount) {
            return Tile::SE;
        }
        else if (ind2 == ind1 + colCount - 1) {
            return Tile::SW;
        }
        else if (ind2 == ind1 - 1) {
            return Tile::W;
        }
        else if (ind2 == ind1 - colCount - 1) {
            return Tile::NW;
        }
    }
    else {
        if (ind2 == ind1 - colCount + 1) {
            return Tile::NE;
        }
        else if (ind2 == ind1 + 1) {
            return Tile::E;
        }
        else if (ind2 == ind1 + colCount + 1) {
            return Tile::SE;
        }
        else if (ind2 == ind1 + colCount) {
            return Tile::SW;
        }
        else if (ind2 == ind1 - 1) {
            return Tile::W;
        }
        else if (ind2 == ind1 - colCount) {
            return Tile::NW;
        }
    }

    GameManager::log("Erreur dans l'appel de getDirection() !");
    return Tile::CENTER;
}

int Carte::getAdjacentTileAt(const int tileSource, const Tile::ETilePosition direction) const noexcept {
    int y = getY(tileSource);
    bool pair = (y % 2 == 0);
    int res;
    switch (direction)
    {
    case Tile::NE:
        if (pair) {
            res = tileSource - colCount;
        }
        else {
            res = tileSource - colCount + 1;
        }
        break;
    case Tile::E:
        res = tileSource + 1;
        break;
    case Tile::SE:
        if (pair) {
            res = tileSource + colCount;
        }
        else {
            res = tileSource + colCount + 1;
        }
        break;
    case Tile::SW:
        if (pair) {
            res = tileSource + colCount - 1;
        }
        else {
            res = tileSource + colCount;
        }
        break;
    case Tile::W:
        res = tileSource - 1;
        break;
    case Tile::NW:
        if (pair) {
            res = tileSource - colCount - 1;
        }
        else {
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
        return -1;
    }
}

float Carte::distanceL2(const int depart, const int arrivee) const noexcept {
    int xd = depart % colCount;
    int yd = depart / colCount;
    int xa = arrivee % colCount;
    int ya = arrivee / colCount;
    return (float)sqrt(pow(xd - xa, 2) + pow(yd - ya, 2));
}

int Carte::distanceHex(const int tile1ID, const int tile2ID) const noexcept {
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

int Carte::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Carte::addTile(const TileInfo& tile) noexcept {
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
    GameManager::log(ss.str());
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Carte::addObject(const ObjectInfo& object) noexcept {
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
        }
        else {
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
    // Porte
    // Dans l'implémentation actuelle, les portes ne bloques plus l'accessibilitée !
    if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
        // On crée notre porte
        portes[object.objectID] = Porte{ object, *this };
        // On l'ajoute à ses voisins et on les sets comme existants, car les voisins d'une porte existent forcément !
        if (isInMap(voisin1)) {
            tiles[voisin1].addPorte(object.objectID);
            //tiles[voisin1].setStatut(MapTile::Statut::CONNU); // Attention ça c'est pas top top !
            presumerConnu(voisin1);
        }
        if (isInMap(voisin2)) {
            tiles[voisin2].addPorte(object.objectID);
            //tiles[voisin2].setStatut(MapTile::Statut::CONNU); // Attention ça c'est pas top top !
            presumerConnu(voisin2);
        }
        //Porte Ferme
        if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
            // Porte Fenetre
            if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
                //if (isInMap(voisin1))
                //   tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
                //if (isInMap(voisin2))
                //   tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
                // Porte
            }
            else {
                if (isInMap(voisin1)) {
                    //tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
                    tiles[voisin1].removeEtat(Etats::VISIBLE, voisin2);
                }
                if (isInMap(voisin2)) {
                    //tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
                    tiles[voisin2].removeEtat(Etats::VISIBLE, voisin1);
                }
            }
            // Porte ouverte
        }
        else {
            // Si la porte est ouverte et est accessible ET visible ! =)
        }
    }
    // Activateur
    if (object.objectTypes.find(Object::ObjectType_PressurePlate) != object.objectTypes.end()) {
        activateurs[object.objectID] = Activateur(object.objectID, object.tileID);
    }

    // On le note !

    stringstream ss;
    ss << "Decouverte de l'objet " << object.objectID << " sur la tuile " << object.tileID << " orienté en " << object.position;
    GameManager::log(ss.str());
}

void Carte::presumerConnu(const int idTile) noexcept {
    // Si on ne la connaissais pas déjà
    if (!tiles[idTile].existe()) {
        tiles[idTile].presumerConnu();

        // On le note !
        stringstream ss;
        ss << "Présomption de connaissance de la tile " << idTile;
        GameManager::log(ss.str());
    }
}

int Carte::getX(const int id) const noexcept {
    return id % colCount;
}
int Carte::getY(const int id) const noexcept {
    return id / colCount;
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

MapTile& Carte::getTile(const int id) {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

const MapTile& Carte::getTile(const int id) const {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

vector<unsigned int> Carte::getObjectifs() {
    return objectifs;
}

map<unsigned int, ObjectInfo> Carte::getMurs() {
    return murs;
}

map<int, Porte>& Carte::getPortes() {
    return portes;
}

Porte Carte::getPorte(const int id) const noexcept {
    return portes.at(id);
}

Porte& Carte::getPorte(const int tileIdVoisine1, const int tileIdVoisine2) {
    for (auto& pair : portes) {
        Porte& porte = pair.second;
        if (porte.isVoisine(tileIdVoisine1) && porte.isVoisine(tileIdVoisine2))
            return porte;
    }
    throw porte_inexistante{};
}

map<unsigned int, ObjectInfo> Carte::getFenetres() {
    return fenetres;
}

map<unsigned int, Activateur> Carte::getActivateurs() const noexcept {
    return activateurs;
}

bool Carte::isKnownActivateur(const int activateurId) const noexcept {
    return activateurs.find(activateurId) != activateurs.end();
}

bool Carte::isActivateurUnderTileId(const int tileId) const noexcept {
    for (auto& pair : activateurs) {
        Activateur activateur = pair.second;
        if (activateur.getTileId() == tileId)
            return true;
    }
    return false;
}

bool Carte::objectExist(const int objet) const noexcept {
    return murs.find(objet) != murs.end()
        || portes.find(objet) != portes.end()
        || fenetres.find(objet) != fenetres.end()
        || activateurs.find(objet) != activateurs.end();
}
