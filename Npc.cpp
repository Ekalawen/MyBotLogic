
#include "Npc.h"
#include "Globals.h"
#include "Voisin.h"
#include "GameManager.h"
#include "MyBotLogic/Tools/Minuteur.h"
#include "MyBotLogic/Tools/Profiler.h"

#include <algorithm>
#include <sstream>
#include <vector>
#include <memory>

using std::stringstream;
using std::vector;
using std::to_string;
using std::endl;

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
      // Si le chemin n'est pas d�j� pris et qu'il est plus court !
      int destination = (cheminTrouve.empty()) ? tileId : cheminTrouve.destination(); // si le npc est d�j� arriv� il reste l�
      if (cheminTrouve.isAccessible()
         && cheminTrouve.distance() < distMin
         && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
         cheminMin = cheminTrouve;
         distMin = cheminTrouve.distance();
      }
   }

   return cheminMin;
}

auto Npc::chercherMeilleurScore(Scores& _scores) {
   Profiler profiler2{ GameManager::getLogger(), "chercherMeilleurScore" };

   return max_element(begin(_scores), end(_scores),
      [](const ScoreType& scoreDroite, const ScoreType& scoreGauche) {
      return scoreDroite.score < scoreGauche.score;
   });
}

int Npc::affecterMeilleurChemin(const Carte&_carte) noexcept {
   Profiler profiler{ GameManager::getLogger(), "affecterMeilleurChemin" };

   if (scoresAssocies.empty()) {
      // Dans ce cas-l� on reste sur place !
      chemin = Chemin{};
      profiler << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
      return tileId;
   }

   // On cherche le meilleur score
   auto bestIter = chercherMeilleurScore(scoresAssocies);

   // On affecte son chemin, mais il nous faut le calculer ! =)
   chemin = _carte.aStar(tileId, bestIter->tuileID);

   profiler << "Le Npc " << to_string(id) << " va rechercher la tile " << chemin.destination() << endl;

   // On renvoie la destination
   return chemin.destination();
}

void Npc::floodfill(const Carte& _carte) {
   ensembleAccessible.clear();

   vector<int> oldOpen;
   // On ajoute le noeud initial
   vector<int> newOpen{ tileId };

   int cout = 0;
   // Tant qu'il reste des noeuds � traiter ...
   while (!newOpen.empty()) {
      oldOpen = newOpen;
      newOpen = vector<int>();
      // On regarde les voisins des dernieres tuiles ajout�es
      for (int tileID : oldOpen) {
         for (auto voisinID : _carte.getTile(tileID).getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
            // Si elle est connu
            if (_carte.getTile(voisinID).existe()) {
               // Si elle n'est pas d�j� ajout�
               if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&voisinID](const DistanceType& type) {
                  return type.tuileID == voisinID; }) == ensembleAccessible.end()) {
                  // On l'ajoute comme nouvelle tuile ouverte
                  newOpen.push_back(voisinID);
               }
            }
         }

         // On d�finit les derni�res tuiles ajout�s avec leur co�t courant
         if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&](const DistanceType& type) {
            return type.tuileID == tileID; }) == ensembleAccessible.end()) {
            ensembleAccessible.emplace_back(tileID, cout);
         }
      }
      cout++;
   }
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