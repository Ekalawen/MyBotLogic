
#include "Npc.h"
#include "Globals.h"
#include "Voisin.h"
#include "GameManager.h"
#include "Noeud.h"
#include <chrono>
#include "MyBotLogic/Tools/Minuteur.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <memory>

using std::stringstream;
using std::vector;
using std::to_string;

Npc::Npc(const NPCInfo info) :
	id{ static_cast<int>(info.npcID) },
	tileId{ static_cast<int>(info.tileID) },
	tileObjectif{ -1 },
	chemin{ Chemin{} },
	estArrive{ false }
{
}

void Npc::move(const Tile::ETilePosition _direction, Carte& _carte) noexcept {
   tileId = _carte.getAdjacentTileAt(tileId, _direction);
   _carte.getTile(tileId).setStatut(MapTile::Statut::VISITE);
}

void Npc::resetChemins() noexcept {
   cheminsPossibles.clear();
   scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) noexcept {
   cheminsPossibles.push_back(chemin);
}

void Npc::addScore(ScoreType _score) noexcept {
   scoresAssocies.emplace_back(std::move(_score));
}

Chemin Npc::getCheminMinNonPris(const vector<int>& objectifsPris, const int tailleCheminMax) const noexcept {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas déjà pris et qu'il est plus court !
		int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est déjà arrivé il reste là
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;

}

int Npc::affecterMeilleurChemin(GameManager& gm) noexcept {
    stringstream ss;

    if (scoresAssocies.empty()) {
        // Dans ce cas-là on reste sur place !
        chemin = Chemin{};
        ss << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
        GameManager::log(ss.str());
        return tileId;
    }
	
    // On cherche le meilleur score
    auto preScore = std::chrono::high_resolution_clock::now();
    auto bestIter = std::max_element(begin(scoresAssocies), end(scoresAssocies),
        [](const ScoreType& scoreDroite, const ScoreType& scoreGauche){
            return scoreDroite.score < scoreGauche.score;
        });
    auto postScore = std::chrono::high_resolution_clock::now();
    ss << "Durée chercher meilleur score = " << std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f << "ms" <<std::endl;

    // On affecte son chemin, mais il nous faut le calculer ! =)
    auto preAStar = std::chrono::high_resolution_clock::now();
    chemin = gm.c.aStar(tileId, bestIter->tuileID, getId(), gm);
    auto postAStar = std::chrono::high_resolution_clock::now();

    ss << "Le Npc " << to_string(id) << " va rechercher la tile " << chemin.destination() << std::endl;
    ss << "Durée a* = " << std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f << "ms" << std::endl;
    GameManager::log(ss.str());

    // On renvoie la destination
    return chemin.destination();
}

void Npc::floodfill(GameManager& gm) {
    ensembleAccessible.clear();

    vector<Noeud> fermees;
    vector<Noeud> ouverts;

    ouverts.push_back(Noeud(gm.c.getTile(tileId), 0));

    while (!ouverts.empty()) {
        Noeud courant = ouverts[0];
        ouverts.erase(ouverts.begin());

        for (int voisin : courant.tile.getVoisinsIDParEtat(ACCESSIBLE)) { // Pour chaque voisins du noeud courant
            if (gm.c.getTile(voisin).existe()) { // Si le voisin existe
                MapTile& voisinTile = gm.c.getTile(courant.tile.getId());

                // On gère les potentielles portes
                bool doorOk = true;
                int tempsAvantOuverture = 0;
                vector<Contrainte> contraintes{};
                if (voisinTile.hasDoor(voisin, gm.c)) {
                    doorOk = voisinTile.canPassDoor(voisin, { getId() }, courant.tile.getId(), gm, tempsAvantOuverture, contraintes);
                }

                //if (!voisinTile.hasClosedDoorSwitch(voisin, gm.c) // On vérifie qu'il n'y a pas de porte à switch devant
                //    || voisinTile.getContrainte(voisin, gm.c).isSolvableWithout(vector<int>{getId()}, gm, cout)) { // Où qu'elle n'est pas génante !

                // Si toutes les contraintes avec les portes sont favorables
                if(doorOk) {
                    int cout = std::max(!gm.c.getTile(courant.tile.getId()).hasDoorPoigneeVoisin(voisin, gm.c) ? 1 : 2, tempsAvantOuverture); // Si il y a une porte à poignée c'est 2 fois plus long !
                    Noeud nouveau{ gm.c.getTile(voisin), courant.cout + cout };
                    auto itFermee = find(fermees.begin(), fermees.end(), nouveau);
                    auto itOuvert = find(ouverts.begin(), ouverts.end(), nouveau);

                    if (itFermee == fermees.end() && itOuvert == ouverts.end()) {
                        ouverts.push_back(nouveau);
                    }
                    else if (itFermee != fermees.end() && itOuvert == ouverts.end()) {
                        // Do nothing
                    }
                    else if (itFermee == fermees.end() && itOuvert != ouverts.end()) {
                        if ((*itOuvert).cout > nouveau.cout) {
                            (*itOuvert) = nouveau;
                        }
                    }
                    else {
                        GameManager::log("Problème dans le floodfill !");
                    }
                }
            }
        }

        // Donc celui qui minimise et le cout, et l'évaluation !
        //sort(ouverts.begin(), ouverts.end(), [](const Noeud a, const Noeud b) {
            //return a.cout > b.cout; // Par ordre décroissant
        //});

        fermees.push_back(courant);
    }

    // On copie les fermées dans ensembleAccessible
    for (auto noeud : fermees) {
        ensembleAccessible.emplace_back(noeud.tile.getId(), noeud.cout);
    }

    //ensembleAccessible.clear();
    //vector<int> oldOpen;
    //// On ajoute le noeud initial
    //vector<int> newOpen { tileId };

    //int cout = 0;
    //// Tant qu'il reste des noeuds à traiter ...
    //while (!newOpen.empty()) {
    //    oldOpen = newOpen;
    //    newOpen = vector<int>();
    //    // On regarde les voisins des dernieres tuiles ajoutées
    //    for (int tileID : oldOpen) {
    //        for (auto voisinID : c.getTile(tileID).getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
    //            // Si elle est connu
    //            if (c.getTile(voisinID).existe()) {
    //                // Si elle n'est pas déjà ajouté
    //                if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&voisinID](const DistanceType& type) {
    //                    return type.tuileID == voisinID; }) == ensembleAccessible.end()) {
    //                    // On l'ajoute comme nouvelle tuile ouverte
    //                    newOpen.push_back(voisinID);
    //                }
    //            }
    //        }

    //        // On définit les dernières tuiles ajoutés avec leur coût courant
    //        if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&](const DistanceType& type) {
    //            return type.tuileID == tileID; }) == ensembleAccessible.end()) {
    //            ensembleAccessible.emplace_back(tileID, cout);
    //        }
    //    }
    //    cout++;
    //}
}

int Npc::getId() const noexcept {
   return id;
}

int Npc::getTileId() const noexcept {
   return tileId;
}

int Npc::getTileObjectif() const noexcept {
   return tileObjectif;
}

void Npc::setTileObjectif(const int idTile) noexcept {
   tileObjectif = idTile;
}

Chemin& Npc::getChemin() noexcept {
   return chemin;
}

void Npc::setChemin(Chemin& _chemin) noexcept {
    chemin = _chemin;
}

Distances& Npc::getEnsembleAccessible() noexcept {
   return ensembleAccessible;
}

bool Npc::isAccessibleTile(const int _tuileID) const noexcept {
   return find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
      return type.tuileID == _tuileID;
   }) != end(ensembleAccessible);
}

int Npc::distanceToTile(const int _tuileID) {
   if (!isAccessibleTile(_tuileID))
      throw tile_inaccessible{};

   return find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
      return type.tuileID == _tuileID;
   })->score;
}

bool Npc::isArrived() const noexcept {
   return estArrive;
}

void Npc::setArrived(const bool etat) noexcept {
   estArrive = etat;
}
