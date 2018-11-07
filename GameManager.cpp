
#include "GameManager.h"
#include "Mouvement.h"

#include "BehaviorTree/BT_Noeud.h"
#include "BehaviorTree/BT_Composite.h"
#include "BehaviorTree/Composite/Sequenceur.h"
#include "BehaviorTree/Composite/Selecteur.h"
#include "BT_Tests/ObjectifsForAllNpcs.h"
#include "BT_Tests/CheminsForAllNpcs.h"
#include "Strategies/Expedition.h"
#include "Strategies/Exploration.h"
#include "Strategies/Exploitation.h"

#include "MyBotLogic/Tools/Minuteur.h"

#include <algorithm>
#include <tuple>
#include <chrono>
#include <sstream>
#include <memory>

using std::make_unique;
using std::stringstream;
using std::endl;
using std::max;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo _info) :
   carte{ Carte(_info) },
   objectifPris{ vector<int>{} }
{
   // On r�cup�re l'ensemble des npcs !
   for (auto pair_npc : _info.npcs) {
      NPCInfo npc = pair_npc.second;
      npcs[npc.npcID] = Npc(npc);
   }
}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Cr�ation du behaviorTree Manager
   unique_ptr<ObjectifsForAllNpcs> objectifs = make_unique<ObjectifsForAllNpcs>(*this);
   unique_ptr<CheminsForAllNpcs> chemins = make_unique< CheminsForAllNpcs>(*this);
   unique_ptr<Exploitation> exploitation = make_unique<Exploitation>(*this);
   unique_ptr<ScoreStrategie> expedition = make_unique<Expedition>(*this, "Expedition");
   unique_ptr<ScoreStrategie> exploration = make_unique<Exploration>(*this, "Exploration");

   vector<unique_ptr<BT_Noeud>> vecSequ1{};
   vecSequ1.push_back(move(chemins));
   vecSequ1.push_back(move(exploitation));
   unique_ptr<Sequenceur> sequenceur1 = make_unique<Sequenceur>(move(vecSequ1));

   vector<unique_ptr<BT_Noeud>> vecSelec{};
   vecSelec.push_back(move(sequenceur1));
   vecSelec.push_back(move(expedition));
   unique_ptr<Selecteur> selecteur = make_unique<Selecteur>(move(vecSelec));

   vector<unique_ptr<BT_Noeud>> vecSequ2{};
   vecSequ2.push_back(move(objectifs));
   vecSequ2.push_back(move(selecteur));
   unique_ptr<Sequenceur> sequenceur2 = make_unique<Sequenceur>(move(vecSequ2));

   vector<unique_ptr<BT_Noeud>> vecBehaviorTree;
   vecBehaviorTree.push_back(move(sequenceur2));
   vecBehaviorTree.push_back(move(exploration));
   behaviorTreeManager = Selecteur(move(vecBehaviorTree));
}

vector<Mouvement> GameManager::getAllMouvements() {
   // On va r�cup�rer la liste des mouvements
   vector<Mouvement> mouvements;

   // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
   for (auto& npc : npcs) {
      stringstream ss;
      ss << "NPC = " << npc.second.getId() << endl
         << "chemin = " << npc.second.getChemin().toString() << endl
         << "case actuelle = " + npc.second.getTileId() << endl;



      // Si le npc doit aller quelquepart !!!
      if (!npc.second.getChemin().empty()) {
         // On r�cup�re la case o� il doit aller
         int caseCible = npc.second.getChemin().getFirst();
         ss << "case cible = " << caseCible << endl;

         Tile::ETilePosition direction = carte.getDirection(npc.second.getTileId(), caseCible);
         ss << "direction = " << direction << endl;

         // On enregistre le mouvement
         mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

         npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
      }
      else {
         ss << "case cible = Ne Bouge Pas" << endl;
         // M�me si le Npc ne bouge pas, il a quand m�me un mouvement statique !
         mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
      }

      GameManager::log(ss.str());
   }
   return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& _actionList) noexcept {
   // TODO !
   // Il faut r�ordonner les chemins entre les npcs !
   // Cad que si deux Npcs peuvent �changer leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
   reaffecterObjectifsSelonDistance();

   // On r�cup�re tous les mouvements
   vector<Mouvement> mouvements = getAllMouvements();

   // Puis on va l'ordonner pour laisser la priorit� � celui qui va le plus loin !
   ordonnerMouvements(mouvements);

   // Puis pour chaque mouvement
   for (auto mouvement : mouvements) {
      // On ne prend en compte notre mouvement que s'il compte
      if (mouvement.isNotStopped()) {
         // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
         _actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
         // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
         npcs[mouvement.getNpcId()].move(mouvement.getDirection(), carte);
         // TEST : pour chaque npc qui se d�place sur son objectif � ce tour, alors mettre estArrive � vrai
         if (mouvement.getDirection() != Tile::ETilePosition::CENTER && npcs[mouvement.getNpcId()].getTileObjectif() == mouvement.getTileDestination())
            // il faut aussi v�rifier si tous les NPC ont un objectif atteignable, donc si on est en mode Exploitation
         {
            npcs[mouvement.getNpcId()].setArrived(true);
         }
         else {
            npcs[mouvement.getNpcId()].setArrived(false);
         }
      }
   }
}

vector<int> getIndicesMouvementsSurMemeCaseCible(vector<Mouvement>& _mouvements, int _caseCible) {
   vector<int> indices;
   for (int i = 0; i < _mouvements.size(); ++i) {
      if (_mouvements[i].getTileDestination() == _caseCible) indices.push_back(i);
   }
   return indices;
}

int GameManager::getIndiceMouvementPrioritaire(vector<Mouvement>& _mouvements, const vector<int>& _indicesAConsiderer) {
   int indiceMax = _indicesAConsiderer[0];
   int distanceMax = getNpcById(_mouvements[_indicesAConsiderer[0]].getNpcId()).getChemin().distance();
   for (int i = 0; i < _indicesAConsiderer.size(); ++i) {
      // Si un mouvement est stationnaire, alors personne n'est autoris� � passer !
      if (!_mouvements[_indicesAConsiderer[i]].isNotStopped())
         return -1;
      int dist = getNpcById(_mouvements[_indicesAConsiderer[i]].getNpcId()).getChemin().distance();
      if (dist > distanceMax) {
         indiceMax = i;
         distanceMax = dist;
      }
   }
   return indiceMax;
}

void GameManager::stopNonPrioritaireMouvements(vector<Mouvement>& _mouvements, const vector<int>& _indicesMouvementsSurMemeCaseCible, const int _indiceMouvementPrioritaire, bool& _continuer) {
   stringstream ss;

   for (int i = 0; i < _indicesMouvementsSurMemeCaseCible.size(); ++i) {
      if (_indicesMouvementsSurMemeCaseCible[i] != _indiceMouvementPrioritaire) {
         int indice = _indicesMouvementsSurMemeCaseCible[i];
         // Si le mouvement n'�tait pas d�j� � l'arr�t alors on a r�ellement effectu� un changement !
         if (_mouvements[indice].isNotStopped())
            _continuer = true;
         _mouvements[indice].stop();
         npcs[_mouvements[indice].getNpcId()].getChemin().resetChemin();

         ss << "Npc " << _mouvements[indice].getNpcId();
         if (_indiceMouvementPrioritaire != -1)
            ss << " a stopp� son mouvement pour laisser la place � Npc " << _mouvements[_indiceMouvementPrioritaire].getNpcId();
         else
            ss << " a stopp� son mouvement car quelqu'un est immobile.";
      }
   }

   GameManager::log(ss.str());
}

void GameManager::gererCollisionsMemeCaseCible(vector<Mouvement>& _mouvements) {
   // Tant que l'on a fait une modification
   bool continuer = true;
   // Pour toutes les cases cibles
   while (continuer) {
      continuer = false;
      for (auto& mouvement : _mouvements) {
         // On r�cup�re tous les indices des mouvements qui vont sur cette case
         vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(_mouvements, mouvement.getTileDestination());

         // Si ils sont plusieurs � vouloir aller sur cette case
         if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
            // On r�cup�re le mouvement associ� au Npc ayant le plus de chemin � faire
            int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(_mouvements, indicesMouvementsSurMemeCaseCible);

            // On passe tous les autres mouvements en Center !
            stopNonPrioritaireMouvements(_mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
         }
      }
   }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& _mouvements) noexcept {
   // Si deux npcs veulent aller sur la m�me case, alors celui qui a le plus de chemin � faire passe, et tous les autres restent sur place !
   gererCollisionsMemeCaseCible(_mouvements);
}

void GameManager::addNewTiles(const TurnInfo& _tile) noexcept {
   if (carte.getNbTilesDecouvertes() < carte.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : _tile.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas �t� d�couvertes
            if (carte.getTile(tileId).getStatut() == MapTile::INCONNU) {
               // On les setDecouverte
               carte.addTile(_tile.tiles.at(tileId));
            }
         }
      }
   }
}

void GameManager::addNewObjects(const TurnInfo& _tile) noexcept {
   // Tous les objets visibles par tous les npcs ...
   for (auto npc : _tile.npcs) {
      for (auto objet : npc.second.visibleObjects) {
         // Si on ne conna�t pas cet objet on l'ajoute
         if (!carte.objectExist(objet)) {
            carte.addObject(_tile.objects.at(objet));
         }
      }
   }
}

void GameManager::updateModel(const TurnInfo &_tile) noexcept {

   stringstream ss;

   // On essaye de rajouter les nouvelles tiles !
   auto pre = Minuteur::now();
   addNewTiles(_tile);
   auto post = Minuteur::now();
   ss << "Dur�e AddTile = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;

   // On essaye de rajouter les nouvelles tiles !
   pre = Minuteur::now();
   addNewObjects(_tile);
   post = Minuteur::now();
   ss << "Dur�e AddObjects = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;

   // Mettre � jour nos NPCs
   pre = Minuteur::now();
   for (auto &npc : npcs) {
      npc.second.floodfill(carte);
   }
   post = Minuteur::now();
   ss << "Dur�e FloodFill = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";

   GameManager::log(ss.str());

}


Npc& GameManager::getNpcById(int _id) {
   if (npcs.find(_id) == npcs.end())
      throw npc_inexistant{};
   return npcs[_id];
}
map<int, Npc>& GameManager::getNpcs() {
   return npcs;
}
void GameManager::addNpc(Npc npc) {
   if (npcs.find(npc.getId()) != npcs.end())
      throw npc_deja_existant{};
   npcs[npc.getId()] = npc;
}

void GameManager::reaffecterObjectifsSelonDistance() {
   // Tant que l'on fait des modifications on continue ...
   stringstream ss;
   bool continuer = true;
   while (continuer) {
      continuer = false;

      // Pour tous les npcs ...
      for (auto& npcPair : npcs) {
         Npc& npc = npcPair.second;
         for (auto& autreNpcPair : npcs) {
            Npc& autreNpc = autreNpcPair.second;
            int objectifNpc = npc.getChemin().empty() ? npc.getTileId() : npc.getChemin().destination();
            int objectifAutreNpc = autreNpc.getChemin().empty() ? autreNpc.getTileId() : autreNpc.getChemin().destination();
            int tempsMaxChemins = max(npc.getChemin().distance(), autreNpc.getChemin().distance());
            if (npc.getId() != autreNpc.getId()) {
               // Si l'interversion des objectifs est b�n�fique pour l'un deux et ne co�te rien � l'autre (ou lui est aussi b�n�fique)
               if (npc.isAccessibleTile(objectifAutreNpc) // D�j� on v�rifie que l'intervertion est "possible"
                  && autreNpc.isAccessibleTile(objectifNpc)) {
                  if (max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                      // Alors on intervertit !                           
                     ss << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " �changent leurs objectifs !" << endl;
                     npc.getChemin() = carte.aStar(npc.getTileId(), objectifAutreNpc);
                     autreNpc.getChemin() = carte.aStar(autreNpc.getTileId(), objectifNpc);
                     continuer = true; // Et on devra continuer pour v�rifier que cette intervertion n'en a pas entrain� de nouvelles !
                  }
               }
            }
         }
      }
   }

   GameManager::log(ss.str());
}
