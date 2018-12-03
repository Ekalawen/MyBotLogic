
#include "Npc.h"
#include "Globals.h"
#include "Voisin.h"
#include "GameManager.h"
#include "Noeud.h"
#include <chrono>
#include "MyBotLogic/Tools/Minuteur.h"
#include "MyBotLogic/Tools/Profiler.h"

#include <algorithm>
#include <sstream>
#include <vector>
#include <memory>

using namespace std;
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
      Chemin cheminTrouve = cheminsPossibles[i];
      // Si le chemin n'est pas deja pris et qu'il est plus court !
      int destination = (cheminTrouve.empty()) ? tileId : cheminTrouve.destination(); // si le npc est deja arrive il reste la
      if (cheminTrouve.isAccessible()
         && cheminTrouve.distance() < distMin
         && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
         cheminMin = cheminTrouve;
         distMin = cheminTrouve.distance();
      }
   }

   return cheminMin;

}

Scores::iterator Npc::chercherMeilleurScore(Scores& _scores) {
   ProfilerDebug profiler{ GameManager::getLogger(), "chercherMeilleurScore", false };

   return max_element(begin(_scores), end(_scores),
      [](const ScoreType& scoreDroite, const ScoreType& scoreGauche) {
      return scoreDroite.score < scoreGauche.score;
   });
}

int Npc::affecterMeilleurChemin(GameManager& gm) noexcept {
   ProfilerDebug profiler{ GameManager::getLogger(), "affecterMeilleurChemin", false };

   if (scoresAssocies.empty()) {
      // Dans ce cas-la on reste sur place !
      chemin = Chemin{};
      profiler << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
      return tileId;
   }

   // On cherche le meilleur score
   auto bestIter = chercherMeilleurScore(scoresAssocies);

   // On affecte son chemin, mais il nous faut le calculer ! =)
   chemin = gm.c.aStar(tileId, bestIter->tuileID, getId(), gm);
   profiler << "Le Npc " << to_string(id) << " va rechercher la tile " << chemin.destination() << endl;
   // On renvoie la destination
   return chemin.destination();
}

void Npc::floodfill(GameManager& gm) {
   ProfilerDebug profiler{ GameManager::getLogger(), "floodfill NPC " + to_string(getId()) };
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
            int tempsAvantOuverture = 0;
            //bool doorOk = true;
            //vector<Contrainte> contraintes{};
            //if (voisinTile.hasDoor(voisin, gm.c)) {
            //    doorOk = voisinTile.canPassDoor(voisin, { getId() }, courant.tile.getId(), gm, tempsAvantOuverture, contraintes);
            //}

            //if (!voisinTile.hasClosedDoorSwitch(voisin, gm.c) // On vérifie qu'il n'y a pas de porte à switch devant
            //    || voisinTile.getContrainte(voisin, gm.c).isSolvableWithout(vector<int>{getId()}, gm, cout)) { // Où qu'elle n'est pas génante !

            // Si toutes les contraintes avec les portes sont favorables
            //if(doorOk) {
            if (porteNonBloquante(gm, voisinTile, voisin, courant, tempsAvantOuverture)) {
               //int cout = std::max(!gm.c.getTile(courant.tile.getId()).hasDoorPoigneeVoisin(voisin, gm.c) ? 1 : 2, tempsAvantOuverture); // Si il y a une porte à poignée c'est 2 fois plus long !
               //Noeud nouveau{ gm.c.getTile(voisin), courant.cout + cout };
               //auto itFermee = find(fermees.begin(), fermees.end(), nouveau);
               //auto itOuvert = find(ouverts.begin(), ouverts.end(), nouveau);

               //if (itFermee == fermees.end() && itOuvert == ouverts.end()) {
               //    ouverts.push_back(nouveau);
               //}
               //else if (itFermee != fermees.end() && itOuvert == ouverts.end()) {
               //    // Do nothing
               //}
               //else if (itFermee == fermees.end() && itOuvert != ouverts.end()) {
               //    if ((*itOuvert).cout > nouveau.cout) {
               //        (*itOuvert) = nouveau;
               //    }
               //}
               //else {
               //    LOG("Problème dans le floodfill !");
               //}
               ajoutNoeudPorte(gm, voisin, courant, tempsAvantOuverture, fermees, ouverts);
            }
         }
      }

      // Donc celui qui minimise et le cout, et l'evaluation !
      //sort(ouverts.begin(), ouverts.end(), [](const Noeud a, const Noeud b) {
          //return a.cout > b.cout; // Par ordre decroissant
      //});

      fermees.push_back(courant);
   }

   // On copie les fermees dans ensembleAccessible
   for (auto noeud : fermees) {
      ensembleAccessible.emplace_back(noeud.tile.getId(), static_cast<int>(noeud.cout));
   }

   //ensembleAccessible.clear();
   //vector<int> oldOpen;
   //// On ajoute le noeud initial
   //vector<int> newOpen { tileId };

   //int cout = 0;
   //// Tant qu'il reste des noeuds ï¿½ traiter ...
   //while (!newOpen.empty()) {
   //    oldOpen = newOpen;
   //    newOpen = vector<int>();
   //    // On regarde les voisins des dernieres tuiles ajoutï¿½es
   //    for (int tileID : oldOpen) {
   //        for (auto voisinID : c.getTile(tileID).getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
   //            // Si elle est connu
   //            if (c.getTile(voisinID).existe()) {
   //                // Si elle n'est pas dï¿½jï¿½ ajoutï¿½
   //                if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&voisinID](const DistanceType& type) {
   //                    return type.tuileID == voisinID; }) == ensembleAccessible.end()) {
   //                    // On l'ajoute comme nouvelle tuile ouverte
   //                    newOpen.push_back(voisinID);
   //                }
   //            }
   //        }

   //        // On dï¿½finit les derniï¿½res tuiles ajoutï¿½s avec leur coï¿½t courant
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

int Npc::distanceToTile(const int _tuileID) const {
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

bool Npc::porteNonBloquante(GameManager& gm, const MapTile& voisinTile, const int voisin, const Noeud& courant, int& tempsAvantOuverture) const noexcept {
   bool doorOk = true;
   vector<Contrainte> contraintes{};
   if (voisinTile.hasDoor(voisin, gm.c)) {
      doorOk = voisinTile.canPassDoor(voisin, { getId() }, courant.tile.getId(), gm, tempsAvantOuverture, contraintes);
   }
   return doorOk;
}

void Npc::ajoutNoeudPorte(GameManager& gm, const int voisin, const Noeud& courant, int& tempsAvantOuverture, vector<Noeud>& fermees, vector<Noeud>& ouverts) const noexcept {
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
      LOG("Problème dans le floodfill !");
   }
}