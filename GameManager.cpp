
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
#include "MyBotLogic/Tools/Profiler.h"
#include "MyBotLogic/Tools/ThreadPool.h"

#include <algorithm>
#include <tuple>
#include <chrono>
#include <sstream>
#include <memory>
#include <thread>

using std::make_unique;
using std::stringstream;
using std::endl;
using std::max;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo _info) :
   c{ Carte(_info) },
   objectifPris{ vector<int>{} }
{
   // On r�cup�re l'ensemble des npcs !
   for (auto pair_npc : _info.npcs) {
      NPCInfo npc = pair_npc.second;
      npcs[npc.npcID] = Npc(npc);
   }
}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Creation du behaviorTree Manager
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
      ss << "NPC = " << npc.second.getId() << std::endl
         << "chemin = " << npc.second.getChemin().toString() << std::endl
         << "case actuelle = " << npc.second.getTileId() << std::endl;


      // Si le npc doit aller quelquepart !!!
      if (!npc.second.getChemin().empty()) {
         // On r�cup�re la case o� il doit aller
         int caseCible = npc.second.getChemin().getFirst();
         ss << "case cible = " << caseCible << std::endl;

         Tile::ETilePosition direction = c.getDirection(npc.second.getTileId(), caseCible);
         ss << "direction = " << direction << std::endl;

         // Si le mouvement est bloqu� par une porte � poign�e
         if (c.getTile(npc.second.getTileId()).hasDoorPoigneeVoisin(caseCible, c)) {
            // Alors on enregistre un mouvement statique
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
            // Et on lui pr�cise qu'il s'agit d'un mouvement d'ouverture de porte et non de d�placement !
            mouvements[mouvements.size() - 1].setActivateDoor();
         }
         else {

            // On enregistre le mouvement
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

            npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
         }
      }
      else {
         ss << "case cible = Ne Bouge Pas" << std::endl;
         // M�me si le Npc ne bouge pas, il a quand m�me un mouvement statique !
         mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
      }

      GAME_MANAGER_LOG_DEBUG(ss.str());
   }
   return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& _actionList) noexcept {
   ProfilerDebug profiler{ getLogger(), "moveNpcs" };
   ProfilerRelease profilerRelease{ getLoggerRelease(), "moveNpcs" };
   // TODO !
   // Il faut réordonner les chemins entre les npcs !
   // Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
   reaffecterObjectifsSelonDistance();


   // On recupere tous les mouvements
   vector<Mouvement> mouvements = getAllMouvements();

   // Puis on va l'ordonner pour laisser la priorite a celui qui va le plus loin !
   ordonnerMouvements(mouvements);

   // Puis pour chaque mouvement
   for (auto mouvement : mouvements) {

      // On applique notre mouvement
      mouvement.apply(_actionList, npcs, c);
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

   GAME_MANAGER_LOG_DEBUG(ss.str());
}

void GameManager::gererCollisionsMemeCaseCible(vector<Mouvement>& _mouvements) {
   // Tant que l'on a fait une modification
   bool continuer = true;
   // Pour toutes les cases cibles
   while (continuer) {
      continuer = false;
      for (auto& mouvement : _mouvements) {
         // On recupere tous les indices des mouvements qui vont sur cette case
         vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(_mouvements, mouvement.getTileDestination());

         // Si ils sont plusieurs a vouloir aller sur cette case
         if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
            // On recupere le mouvement associe au Npc ayant le plus de chemin a faire
            int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(_mouvements, indicesMouvementsSurMemeCaseCible);

            // On passe tous les autres mouvements en Center !
            stopNonPrioritaireMouvements(_mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
         }
      }
   }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& _mouvements) noexcept {
   // Si deux npcs veulent aller sur la meme case, alors celui qui a le plus de chemin a faire passe, et tous les autres restent sur place !
   gererCollisionsMemeCaseCible(_mouvements);
}

void GameManager::addNewTiles(const TurnInfo& _tile) noexcept {

   ProfilerDebug profiler{ GameManager::getLogger(), "addNewTiles" };

   if (c.getNbTilesDecouvertes() < c.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : _tile.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas ete decouvertes
            if (c.getTile(tileId).getStatut() == MapTile::INCONNU) {
               // On les setDecouverte
               c.addTile(_tile.tiles.at(tileId));
            }
         }
      }
   }
}

void GameManager::addNewObjects(const TurnInfo& _tile) noexcept {

   ProfilerDebug profiler{ GameManager::getLogger(), "addNewObjects" };

   // Tous les objets visibles par tous les npcs ...
   for (auto npc : _tile.npcs) {
      for (auto objet : npc.second.visibleObjects) {
         // Si on ne connait pas cet objet on l'ajoute
         if (!c.objectExist(objet)) {
            c.addObject(_tile.objects.at(objet));
         }
      }
   }
}

void GameManager::updateModel(const TurnInfo &_tile) noexcept {

   ProfilerDebug profiler{ GameManager::getLogger(), "updateModel" };
   ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "updateModel" };

   // On essaye de rajouter les nouvelles tiles !
   addNewTiles(_tile);

   // On essaye de rajouter les nouvelles tiles !
   addNewObjects(_tile);

   // Mettre a jour nos NPCs
   refreshFloodfill();
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
   ProfilerDebug profiler{ GameManager::getLogger(), "reaffecterObjectifsSelonDistance" };
   // Tant que l'on fait des modifications on continue ...
   bool continuer = true;
   while (continuer) {
      continuer = false;

      for (auto& npcPair : npcs) {
         Npc& npc = npcPair.second;
         for (auto& autreNpcPair : npcs) {
            Npc& autreNpc = autreNpcPair.second;
            int objectifActuelNpc = npc.getChemin().empty() ? npc.getTileId() : npc.getChemin().destination();

            if (npc.getId() != autreNpc.getId() && permutationUtile(npc, autreNpc)) {
               int objectifAutreNpc = autreNpc.getChemin().empty() ? autreNpc.getTileId() : autreNpc.getChemin().destination();
               profiler << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " echangent leurs objectifs !" << std::endl;
               profiler << "Npc " << npc.getId() << " vers " << objectifAutreNpc << " et " << "Npc " << autreNpc.getId() << " vers " << objectifActuelNpc << std::endl;
               
               npc.getChemin() = c.aStar(npc.getTileId(), objectifAutreNpc);
               autreNpc.getChemin() = c.aStar(autreNpc.getTileId(), objectifActuelNpc);

               continuer = true; // Et on devra continuer pour v�rifier que cette intervertion n'en a pas entrain� de nouvelles !
            }
         }
      }
   }
}


void GameManager::refreshFloodfill() {
   ProfilerDebug profiler{ GameManager::getLogger(), "refreshFloodfill" };

   ThreadPool workers;

   for (auto &npc : npcs) {
      //npc.second.floodfill(c);
      std::thread th{
         [&npc](Carte& carte) {
            npc.second.floodfill(carte);
         }
         , c
      };
      workers.addThread(std::move(th));
   }
   workers.joinAll();
}

bool GameManager::permutationUtile(Npc& npc1, Npc& npc2) {
   int objectifNpc1 = npc1.getChemin().empty() ? npc1.getTileId() : npc1.getChemin().destination();
   int objectifNpc2 = npc2.getChemin().empty() ? npc2.getTileId() : npc2.getChemin().destination();
   int tempsMaxChemins = max(npc1.getChemin().distance(), npc2.getChemin().distance());

   // Si l'interversion des objectifs est possible et benefique pour l'un deux et ne coute rien a l'autre (ou lui est aussi benefique)
   return npc1.isAccessibleTile(objectifNpc2) && npc2.isAccessibleTile(objectifNpc1) && (max(npc1.distanceToTile(objectifNpc2), npc2.distanceToTile(objectifNpc1)) < tempsMaxChemins);
}