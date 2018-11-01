
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

#include <algorithm>
#include <tuple>
#include <chrono>
#include <sstream>
#include <memory>

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo _info) :
   map{ Carte(_info) },
   objectifPris{ std::vector<int>{} }
{
   // On r�cup�re l'ensemble des npcs !
   for (auto pair_npc : _info.npcs) {
      NPCInfo npc = pair_npc.second;
      npcs[npc.npcID] = Npc(npc);
   }
}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Cr�ation du behaviorTree Manager
   std::unique_ptr<ObjectifsForAllNpcs> objectifs = std::make_unique<ObjectifsForAllNpcs>(*this);
   std::unique_ptr<CheminsForAllNpcs> chemins = std::make_unique< CheminsForAllNpcs>(*this);
   std::unique_ptr<Exploitation> exploitation = std::make_unique<Exploitation>(*this);
   std::unique_ptr<ScoreStrategie> expedition = std::make_unique<Expedition>(*this, "Expedition");
   std::unique_ptr<ScoreStrategie> exploration = std::make_unique<Exploration>(*this, "Exploration");

   std::vector<std::unique_ptr<BT_Noeud>> vecSequ1{};
   vecSequ1.push_back(std::move(chemins));
   vecSequ1.push_back(std::move(exploitation));
   std::unique_ptr<Sequenceur> sequenceur1 = std::make_unique<Sequenceur>(std::move(vecSequ1));

   std::vector<std::unique_ptr<BT_Noeud>> vecSelec{};
   vecSelec.push_back(std::move(sequenceur1));
   vecSelec.push_back(std::move(expedition));
   std::unique_ptr<Selecteur> selecteur = std::make_unique<Selecteur>(std::move(vecSelec));

   std::vector<std::unique_ptr<BT_Noeud>> vecSequ2{};
   vecSequ2.push_back(std::move(objectifs));
   vecSequ2.push_back(std::move(selecteur));
   std::unique_ptr<Sequenceur> sequenceur2 = std::make_unique<Sequenceur>(std::move(vecSequ2));

   std::vector<std::unique_ptr<BT_Noeud>> vecBehaviorTree;
   vecBehaviorTree.push_back(std::move(sequenceur2));
   vecBehaviorTree.push_back(std::move(exploration));
   behaviorTreeManager = Selecteur(std::move(vecBehaviorTree));
}

std::vector<Mouvement> GameManager::getAllMouvements() {
   // On va r�cup�rer la liste des mouvements
   std::vector<Mouvement> mouvements;

   // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
   for (auto& npc : npcs) {
      std::stringstream ss;
      ss << "NPC = " << npc.second.getId() << std::endl
         << "chemin = " << npc.second.getChemin().toString() << std::endl
         << "case actuelle = " + npc.second.getTileId() << std::endl;



      // Si le npc doit aller quelquepart !!!
      if (!npc.second.getChemin().empty()) {
         // On r�cup�re la case o� il doit aller
         int caseCible = npc.second.getChemin().getFirst();
         ss << "case cible = " << caseCible << std::endl;

         Tile::ETilePosition direction = map.getDirection(npc.second.getTileId(), caseCible);
         ss << "direction = " << direction << std::endl;

         // On enregistre le mouvement
         mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

         npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
      }
      else {
         ss << "case cible = Ne Bouge Pas" << std::endl;
         // M�me si le Npc ne bouge pas, il a quand m�me un mouvement statique !
         mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
      }

      GameManager::log(ss.str());
   }
   return mouvements;
}

void GameManager::moveNpcs(std::vector<Action*>& _actionList) noexcept {
   // TODO !
   // Il faut r�ordonner les chemins entre les npcs !
   // Cad que si deux Npcs peuvent �changer leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
   reaffecterObjectifsSelonDistance();

   // On r�cup�re tous les mouvements
   std::vector<Mouvement> mouvements = getAllMouvements();

   // Puis on va l'ordonner pour laisser la priorit� � celui qui va le plus loin !
   ordonnerMouvements(mouvements);

   // Puis pour chaque mouvement
   for (auto mouvement : mouvements) {
      // On ne prend en compte notre mouvement que s'il compte
      if (mouvement.isNotStopped()) {
         // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
         _actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
         // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
         npcs[mouvement.getNpcId()].move(mouvement.getDirection(), map);
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

std::vector<int> getIndicesMouvementsSurMemeCaseCible(std::vector<Mouvement>& _mouvements, int _caseCible) {
   std::vector<int> indices;
   for (int i = 0; i < _mouvements.size(); ++i) {
      if (_mouvements[i].getTileDestination() == _caseCible) indices.push_back(i);
   }
   return indices;
}

int GameManager::getIndiceMouvementPrioritaire(std::vector<Mouvement>& _mouvements, const std::vector<int>& _indicesAConsiderer) {
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

void GameManager::stopNonPrioritaireMouvements(std::vector<Mouvement>& _mouvements, const std::vector<int>& _indicesMouvementsSurMemeCaseCible, const int _indiceMouvementPrioritaire, bool& _continuer) {
   std::stringstream ss;

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

void GameManager::gererCollisionsMemeCaseCible(std::vector<Mouvement>& _mouvements) {
   // Tant que l'on a fait une modification
   bool continuer = true;
   // Pour toutes les cases cibles
   while (continuer) {
      continuer = false;
      for (auto& mouvement : _mouvements) {
         // On r�cup�re tous les indices des mouvements qui vont sur cette case
         std::vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(_mouvements, mouvement.getTileDestination());

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

void GameManager::ordonnerMouvements(std::vector<Mouvement>& _mouvements) noexcept {
   // Si deux npcs veulent aller sur la m�me case, alors celui qui a le plus de chemin � faire passe, et tous les autres restent sur place !
   gererCollisionsMemeCaseCible(_mouvements);
}

void GameManager::addNewTiles(const TurnInfo& _tile) noexcept {
   if (map.getNbTilesDecouvertes() < map.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : _tile.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas �t� d�couvertes
            if (map.getTile(tileId).getStatut() == MapTile::INCONNU) {
               // On les setDecouverte
               map.addTile(_tile.tiles.at(tileId));
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
         if (!map.objectExist(objet)) {
            map.addObject(_tile.objects.at(objet));
         }
      }
   }
}

void GameManager::updateModel(const TurnInfo &_tile) noexcept {

   std::stringstream ss;

   // On essaye de rajouter les nouvelles tiles !
   auto pre = Minuteur::now();
   addNewTiles(_tile);
   auto post = Minuteur::now();
   ss << "Dur�e AddTile = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;

   // On essaye de rajouter les nouvelles tiles !
   pre = Minuteur::now();
   addNewObjects(_tile);
   post = Minuteur::now();
   ss << "Dur�e AddObjects = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;

   // Mettre � jour nos NPCs
   pre = Minuteur::now();
   for (auto &npc : npcs) {
      npc.second.floodfill(map);
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
std::map<int, Npc>& GameManager::getNpcs() {
   return npcs;
}
void GameManager::addNpc(Npc npc) {
   if (npcs.find(npc.getId()) != npcs.end())
      throw npc_deja_existant{};
   npcs[npc.getId()] = npc;
}

void GameManager::reaffecterObjectifsSelonDistance() {
   // Tant que l'on fait des modifications on continue ...
   std::stringstream ss;
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
            int tempsMaxChemins = std::max(npc.getChemin().distance(), autreNpc.getChemin().distance());
            if (npc.getId() != autreNpc.getId()) {
               // Si l'interversion des objectifs est b�n�fique pour l'un deux et ne co�te rien � l'autre (ou lui est aussi b�n�fique)
               if (npc.isAccessibleTile(objectifAutreNpc) // D�j� on v�rifie que l'intervertion est "possible"
                  && autreNpc.isAccessibleTile(objectifNpc)) {
                  if (std::max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                      // Alors on intervertit !                           
                     ss << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " �changent leurs objectifs !" << std::endl;
                     npc.getChemin() = map.aStar(npc.getTileId(), objectifAutreNpc);
                     autreNpc.getChemin() = map.aStar(autreNpc.getTileId(), objectifNpc);
                     continuer = true; // Et on devra continuer pour v�rifier que cette intervertion n'en a pas entrain� de nouvelles !
                  }
               }
            }
         }
      }
   }

   GameManager::log(ss.str());
}
