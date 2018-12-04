
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
#include "Strategies/LookingForHiddenDoors.h"
#include "Strategies/CheckingHiddenDoors.h"
#include "MyBotLogic/Tools/Minuteur.h"
#include "TurnInfo.h"
#include "Chemin.h"
#include "MyBotLogic/Tools/Profiler.h"
#include "MyBotLogic/Tools/ThreadPool.h"

#include <algorithm>
#include <tuple>
#include <chrono>
#include <sstream>
#include <memory>
#include <string>
#include <thread>

using std::make_unique;
using std::stringstream;
using std::endl;
using std::max;
using namespace std;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo _info) :
   c{ Carte(_info) },
   objectifPris{ vector<int>{} }
{
   // On rï¿½cupï¿½re l'ensemble des npcs !
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
   unique_ptr<CheckingHiddenDoors> checkingHiddenDoors = make_unique<CheckingHiddenDoors>(*this);
   //unique_ptr<ScoreStrategie> lookingForHiddenDoors = make_unique<LookingForHiddenDoors>(*this, "LookingForHiddenDoors");

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
   vecBehaviorTree.push_back(move(checkingHiddenDoors));
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
       
        // On vérifie si le Npc doit checker un mur ou non
        if (npc.second.getIsCheckingDoor()) {
            // Alors on enregistre un mouvement statique
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));

            // Et on lui précise qu'il s'agit d'un mouvement de checking de mur !
            mouvements[mouvements.size() - 1].setCheckingDoor(npc.second.getDirectionCheckingDoor());

            // Et on passe au npc suivant
            continue;
        }

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
                mouvements[mouvements.size() - 1].setActivateDoor(direction);

            // Si il y a porte fermée à switch, il faut attendre
            } else if(c.getTile(npc.second.getTileId()).hasClosedDoor(caseCible, c)) {
                // Alors on enregistre un mouvement statique
                mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));

            // Si il n'y a pas de portes
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

        LOG(ss.str());
    }
    return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
    ProfilerDebug profiler{ getLogger(), "MOVE NPCS" };
    ProfilerRelease profilerRelease{ getLoggerRelease(), "moveNpcs" };
    // Code déplace juste après l'execute, il faudra peut-être aussi le laisser ici, à voir !!! Finalement j'ai dû le laisser ici aussi, c'est nécessaire mais ça coûte !
    //// Il faut réordonner les chemins entre les npcs !
    //// Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
    reaffecterObjectifsSelonDistance();

   // On recupere tous les mouvements
   vector<Mouvement> mouvements = getAllMouvements();

   // Puis on va l'ordonner pour laisser la priorite a celui qui va le plus loin !
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
      // Si un mouvement est stationnaire, alors personne n'est autorisï¿½ ï¿½ passer !
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
         // Si le mouvement n'ï¿½tait pas dï¿½jï¿½ ï¿½ l'arrï¿½t alors on a rï¿½ellement effectuï¿½ un changement !
         if (_mouvements[indice].isNotStopped())
            _continuer = true;
         _mouvements[indice].stop();
         npcs[_mouvements[indice].getNpcId()].getChemin().resetChemin();

         ss << "Npc " << _mouvements[indice].getNpcId();
         if (_indiceMouvementPrioritaire != -1)
            ss << " a stoppï¿½ son mouvement pour laisser la place ï¿½ Npc " << _mouvements[_indiceMouvementPrioritaire].getNpcId();
         else
            ss << " a stoppï¿½ son mouvement car quelqu'un est immobile.";
      }
   }

   LOG(ss.str());
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
    // Ordonner les mouvements pour que ceux qui vont sur des switchs soient les premiers ! Et les derniers s'ils en sortent !
    ordonnerMouvementsSelonSwitchs(_mouvements);

    // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
    gererCollisionsMemeCaseCible(_mouvements);
}

void GameManager::ordonnerMouvementsSelonSwitchs(vector<Mouvement>& mouvements) {
    vector<Mouvement> mouvementsOrdonnes{};

    // On range d'abord les mouvements qui vont sur un switch et qui n'en viennent pas
    for (Mouvement mouvement : mouvements) {
        if (c.isActivateurUnderTileId(mouvement.getTileDestination())
            && !c.isActivateurUnderTileId(mouvement.getTileSource()))
            mouvementsOrdonnes.push_back(mouvement);
    }

    // Puis les mouvements qui sortent et entrent d'un switch, même si c'est pas parfait on fera avec !
    for (Mouvement mouvement : mouvements) {
        if (c.isActivateurUnderTileId(mouvement.getTileDestination())
            && c.isActivateurUnderTileId(mouvement.getTileSource()))
            mouvementsOrdonnes.push_back(mouvement);
    }

    // Puis les mouvements qui n'ont pas de rapport avec un switch
    for (Mouvement mouvement : mouvements) {
        if (!c.isActivateurUnderTileId(mouvement.getTileDestination())
            && !c.isActivateurUnderTileId(mouvement.getTileSource()))
            mouvementsOrdonnes.push_back(mouvement);
    }

    // Puis les mouvements qui sortent d'un switch et qui ne vont pas sur un switch
    for (Mouvement mouvement : mouvements) {
        if (!c.isActivateurUnderTileId(mouvement.getTileDestination())
            && c.isActivateurUnderTileId(mouvement.getTileSource()))
            mouvementsOrdonnes.push_back(mouvement);
    }

    // Puis on affecte !
    mouvements = mouvementsOrdonnes;
}


void GameManager::addNewTiles(const TurnInfo& _tile) noexcept {

   ProfilerDebug profiler{ GameManager::getLogger(), "addNewTiles" };

   if (c.getNbTilesDecouvertes() < c.getNbTiles()) {
      // pour tous les npcs
      for (auto& npc : _tile.npcs) {
         // On regarde les tuiles qu'ils voyent
         for (auto& tileId : npc.second.visibleTiles) {
            // Si ces tuiles n'ont pas été découvertes
            if (c.getTile(tileId).getStatut() == MapTile::INCONNU || c.getTile(tileId).getStatut() == MapTile::PRESUME_CONNU) {
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
   ProfilerDebug profiler{ GameManager::getLogger(), "UPDATE MODEL" };
   ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "updateModel" };

   // On essaye de rajouter les nouvelles tiles !
   addNewTiles(_tile);

   // On essaye de rajouter les nouvelles tiles !
   addNewObjects(_tile);

   // On met à jour les portes à switchs 
   majPortesASwitch();

   // Mettre a jour nos NPCs
   refreshFloodfill();
}

void GameManager::majPortesASwitch() noexcept {
    ProfilerDebug profiler{ GameManager::getLogger(), "majPortesASwitch" };

    // Pour chaque porte à switch, on regarde pour tous ses interrupteurs s'il y a un npc dessus
    for (auto& pair : c.getPortes()) {
        Porte& porte = pair.second;
        if (porte.getType() == Porte::A_SWITCH) {
            bool isOpen = false;
            for (int tileSwitchId : porte.getSwitchsTilesIds(c)) {
                for (auto pair2 : npcs) {
                    Npc npc = pair2.second;
                    if (npc.getTileId() == tileSwitchId) {
                        porte.ouvrirPorte();
                        isOpen = true;
                    }
                }
            }
            if (!isOpen)
                porte.fermerPorte();
        }
    }
}

Npc& GameManager::getNpcById(int _id) {
   if (npcs.find(_id) == npcs.end())
      throw npc_inexistant{};
   return npcs[_id];
}
int GameManager::getNpcIdAtTileId(int tileId) {
    for (auto pair : npcs) {
        Npc npc = pair.second;
        if (npc.getTileId() == tileId)
            return npc.getId();
    }
    LOG("Mauvaise utilisation de la fonction getNpcIdAtTileId() !");
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
    ProfilerDebug profiler{ GameManager::getLogger(), "REAFECTER OBJECTIFS SELON DISTANCE" };
    ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "reaffecterObjectifsSelonDistance" };

    LOG("Objectif initiaux");
    for (auto& npcPair : npcs) {
        LOG("npc = " + to_string(npcPair.second.getId()) + " va en " + to_string(npcPair.second.getTileObjectif()));
    }
    // Tant que l'on fait des modifications on continue ...
    bool continuer = true;
    int k = 0;
    int kmax = 20;
    while (continuer) {
        continuer = false;

        for (auto& npcPair : npcs) {
            Npc& npc = npcPair.second;
            int objectifNpc = npc.getTileObjectif();

            for (auto& autreNpcPair : npcs) {
                Npc& autreNpc = autreNpcPair.second;
                int objectifAutreNpc = autreNpc.getTileObjectif();

                if (npc.getId() != autreNpc.getId() && permutationUtile(npc, autreNpc)) {
                    profiler << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " echangent leurs objectifs !";
                    profiler << "Npc " << npc.getId() << " vers " << objectifAutreNpc << " et " << "Npc " << autreNpc.getId() << " vers " << objectifNpc << std::endl;

                    //npc.setChemin(c.aStar(npc.getTileId(), objectifAutreNpc, npc.getId(), *this));
                    //autreNpc.setChemin(c.aStar(autreNpc.getTileId(), objectifNpc, autreNpc.getId(), *this));
                    npc.setTileObjectif(objectifAutreNpc);
                    autreNpc.setTileObjectif(objectifNpc);
                    objectifNpc = npc.getTileObjectif();

                    continuer = true; // Et on devra continuer pour vérifier que cette intervertion n'en a pas entrainé de nouvelles !
                }
            }
        }
        if (++k > kmax)
            continuer = false;
    }

    // Une fois que toutes les permutations ont étées effectuées, on peut enfin calculer les aStars !!! =)
    for (auto& npcPair : npcs) {
        Npc& npc = npcPair.second;
        npc.setChemin(c.aStar(npc.getTileId(), npc.getTileObjectif(), npc.getId(), *this));
    }
}

void GameManager::affecterContraintes() noexcept {
    ProfilerDebug profiler{ GameManager::getLogger(), "AFFECTER CONTRAINTES" };
    ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "affecterContraintes" };
    for (auto pair : npcs) {
        Npc npc = pair.second;
        profiler << "Npc " << npc.getId() << " veut aller en " << npc.getTileObjectif() << endl;
    }

    // On va clean les contraintes non résolues qui trainent
    cleanContraintes();

    // On construit un vecteur trié des npcs dans l'ordre croissant de la longueur du chemin
    // Comme ça le plus long écrase tous les autres ! Niark !
    vector<Npc*> npcsSelonTailleChemin{};
    for (auto& pair : npcs) {
        Npc* npc = &pair.second;
        npcsSelonTailleChemin.push_back(npc);
    }
    sort(npcsSelonTailleChemin.begin(), npcsSelonTailleChemin.end(), [](Npc* a, Npc* b) {
        return a->getChemin().distance() < b->getChemin().distance();
    });

    // Pour chaque chemin de nos Npcs, on regarde s'ils ont des contraintes, si oui on affecte ses contraintes
    // On termine par le npc le plus loin comme ça il écrasera les autres !
    for (Npc* npc : npcsSelonTailleChemin) {
        vector<int> npcAffectes = npc->getChemin().affecterContraintes(npc->getId(), *this);
    }
    //while (!npcsSelonTailleChemin.empty()) {
    //    Npc* npc = npcsSelonTailleChemin.back(); // On prend celui qui a le plus long chemin en premier

    //    // Cette fonction affecte les chemins aux bonc npcs récursivement et les enlèves de la liste npcsSelonTailleChemin !
    //    vector<int> npcAffectes = npc->getChemin().affecterContraintes(npc->getId(), *this);

    //    // On supprime les npcs affectes de notre liste
    //    for (int npcId : npcAffectes) {
    //        auto it = find_if(npcsSelonTailleChemin.begin(), npcsSelonTailleChemin.end(), [npcId](Npc* npc) {
    //            return npc->getId() == npcId;
    //        });
    //        if (it != npcsSelonTailleChemin.end())
    //            npcsSelonTailleChemin.erase(it);
    //        else
    //            LOG("Erreur pas normal ! x)");
    //    }
    //}
}

void GameManager::cleanContraintes() noexcept {
    for (auto& pair : npcs) {
        Npc& npc = pair.second;
        npc.getChemin().cleanContraintes();
    }
}


void GameManager::refreshFloodfill() {
   ProfilerDebug profiler{ GameManager::getLogger(), "refreshFloodfill" };

   ThreadPool workers;

   for (auto &npc : npcs) {
      //npc.second.floodfill(c);
      std::thread th{
         [&npc](GameManager& gm) {
            npc.second.floodfill(gm);
         }
         , std::ref(*this)
      };
      workers.addThread(std::move(th));

      // On en profite pour réinitialiser un attribut par npcs :)
      npc.second.setIsCheckingDoor(false);
   }
   workers.joinAll();
}

bool GameManager::permutationUtile(Npc& npc1, Npc& npc2) {
    int objectifNpc1 = npc1.getTileObjectif();
    int objectifNpc2 = npc2.getTileObjectif();
    int tempsMaxChemins = max(npc1.distanceToTile(objectifNpc1), npc2.distanceToTile(objectifNpc2));

   // Si l'interversion des objectifs est possible et benefique pour l'un deux et ne coute rien a l'autre (ou lui est aussi benefique)
   return npc1.isAccessibleTile(objectifNpc2) && npc2.isAccessibleTile(objectifNpc1) && (max(npc1.distanceToTile(objectifNpc2), npc2.distanceToTile(objectifNpc1)) < tempsMaxChemins);
}

void GameManager::execute() noexcept { 
   ProfilerDebug profiler{ GameManager::getLogger(), "EXECUTE" }; 
   ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "execute" };

   // On calcul où doivent se rendre les npcs
   behaviorTreeManager.execute(); 

   // On fait des swaps si nécessaires + on calculs les aStars UNE SEULE FOIS !
   reaffecterObjectifsSelonDistance();

   // On affecte les contraintes, donc potentiellement d'autres aStars
   affecterContraintes();
};
