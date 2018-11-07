
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
   c{ Carte(_info) },
   objectifPris{ vector<int>{} }
{
   // On récupére l'ensemble des npcs !
   for (auto pair_npc : _info.npcs) {
      NPCInfo npc = pair_npc.second;
      npcs[npc.npcID] = Npc(npc);
   }
}

void GameManager::InitializeBehaviorTree() noexcept {
   //  Création du behaviorTree Manager
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
    // On va récupérer la liste des mouvements
    vector<Mouvement> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        stringstream ss;
        ss << "NPC = " << npc.second.getId() << std::endl
           << "chemin = " << npc.second.getChemin().toString() << std::endl
           << "case actuelle = " <<  npc.second.getTileId() << std::endl;
       

        // Si le npc doit aller quelquepart !!!
        if (!npc.second.getChemin().empty()) {
            // On récupère la case où il doit aller
            int caseCible = npc.second.getChemin().getFirst();
            ss << "case cible = " << caseCible << std::endl;
            
            Tile::ETilePosition direction = c.getDirection(npc.second.getTileId(), caseCible);
            ss << "direction = " << direction << std::endl;

            // Si le mouvement est bloqué par une porte à poignée
            if (c.getTile(npc.second.getTileId()).hasDoorPoigneeVoisin(caseCible, c)) {
                // Alors on enregistre un mouvement statique
                mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
                // Et on lui précise qu'il s'agit d'un mouvement d'ouverture de porte et non de déplacement !
                mouvements[mouvements.size() - 1].setActivateDoor();
            } else {

                // On enregistre le mouvement
                mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

                npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
            }
        } else {
            ss << "case cible = Ne Bouge Pas" << std::endl;
            // Même si le Npc ne bouge pas, il a quand même un mouvement statique !
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
        }

        GameManager::log(ss.str());
    }
    return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
    // Il faut réordonner les chemins entre les npcs !
    // Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
    reafecterObjectifsSelonDistance();

    // On récupère tous les mouvements
    vector<Mouvement> mouvements = getAllMouvements();

    // Puis on va l'ordonner pour laisser la priorité à celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {

        // On applique notre mouvement
        mouvement.apply(actionList, npcs, c);
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
      // Si un mouvement est stationnaire, alors personne n'est autorisé à passer !
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
         // Si le mouvement n'était pas déjà à l'arrêt alors on a réellement effectué un changement !
         if (_mouvements[indice].isNotStopped())
            _continuer = true;
         _mouvements[indice].stop();
         npcs[_mouvements[indice].getNpcId()].getChemin().resetChemin();

         ss << "Npc " << _mouvements[indice].getNpcId();
         if (_indiceMouvementPrioritaire != -1)
            ss << " a stoppé son mouvement pour laisser la place à Npc " << _mouvements[_indiceMouvementPrioritaire].getNpcId();
         else
            ss << " a stoppé son mouvement car quelqu'un est immobile.";
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
         // On récupére tous les indices des mouvements qui vont sur cette case
         vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(_mouvements, mouvement.getTileDestination());

         // Si ils sont plusieurs à vouloir aller sur cette case
         if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
            // On récupére le mouvement associé au Npc ayant le plus de chemin à faire
            int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(_mouvements, indicesMouvementsSurMemeCaseCible);

            // On passe tous les autres mouvements en Center !
            stopNonPrioritaireMouvements(_mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
         }
      }
   }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& _mouvements) noexcept {
   // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
   gererCollisionsMemeCaseCible(_mouvements);
}

void GameManager::addNewTiles(const TurnInfo& _tile) noexcept {
   if (c.getNbTilesDecouvertes() < c.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : _tile.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas été découvertes
            if (c.getTile(tileId).getStatut() == MapTile::INCONNU) {
               // On les setDecouverte
               c.addTile(_tile.tiles.at(tileId));
            }
         }
      }
   }
}

void GameManager::addNewObjects(const TurnInfo& _tile) noexcept {
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

   stringstream ss;

   // On essaye de rajouter les nouvelles tiles !
   auto pre = Minuteur::now();
   addNewTiles(_tile);
   auto post = Minuteur::now();
   ss << "Durée AddTile = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;

   // On essaye de rajouter les nouvelles tiles !
   pre = Minuteur::now();
   addNewObjects(_tile);
   post = Minuteur::now();
   ss << "Durée AddObjects = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;

   // Mettre à jour nos NPCs
   pre = Minuteur::now();
   for (auto &npc : npcs) {
      npc.second.floodfill(c);
   }
   post = Minuteur::now();
   ss << "Durée FloodFill = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";

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

void GameManager::reafecterObjectifsSelonDistance() {
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
                    // Si l'interversion des objectifs est bénéfique pour l'un deux et ne coûte rien à l'autre (ou lui est aussi bénéfique)
                    if (npc.isAccessibleTile(objectifAutreNpc) // Déjà on vérifie que l'intervertion est "possible"
                        && autreNpc.isAccessibleTile(objectifNpc)) {
                        //if (max(m.distanceHex(npc.getTileId(), objectifAutreNpc), m.distanceHex(autreNpc.getTileId(), objectifNpc))) {
                        if (max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                            // Alors on intervertit !                           
                            ss << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " échangent leurs objectifs !" << std::endl;
                            npc.getChemin() = c.aStar(npc.getTileId(), objectifAutreNpc);
                            autreNpc.getChemin() = c.aStar(autreNpc.getTileId(), objectifNpc);
                            continuer = true; // Et on devra continuer pour vérifier que cette intervertion n'en a pas entrainé de nouvelles !
                        }
                    }
                }
            }
        }
    }
    GameManager::log(ss.str());
}
