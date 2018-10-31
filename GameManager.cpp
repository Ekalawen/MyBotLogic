
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
using namespace std;
using namespace std::chrono;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo info) :
    m{ Map(info) },
	objectifPris{ vector<int>{} }
{
    // On r�cup�re l'ensemble des npcs !
    for (auto pair_npc : info.npcs) {
        NPCInfo npc = pair_npc.second;
        npcs[npc.npcID] = Npc(npc);
    }
}

void GameManager::InitializeBehaviorTree() noexcept {
    //  Cr�ation du behaviorTree Manager
    ObjectifsForAllNpcs *objectifs = new ObjectifsForAllNpcs(*this);
    CheminsForAllNpcs *chemins = new CheminsForAllNpcs(*this);
    Exploitation *exploitation = new Exploitation(*this);
    ScoreStrategie *expedition = new Expedition(*this, "Expedition");
    ScoreStrategie *exploration = new Exploration(*this, "Exploration");

    Sequenceur *sequenceur1 = new Sequenceur({ chemins, exploitation });

    Selecteur *selecteur = new Selecteur({ sequenceur1, expedition });

    Sequenceur *sequenceur2 = new Sequenceur({ objectifs, selecteur });

    behaviorTreeManager = Selecteur({ sequenceur2, exploration });
}

vector<Mouvement> GameManager::getAllMouvements() {
    // On va r�cup�rer la liste des mouvements
    vector<Mouvement> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        stringstream ss;
        ss << "NPC = " << npc.second.getId() << std::endl
           << "chemin = " << npc.second.getChemin().toString() << std::endl
           << "case actuelle = " + npc.second.getTileId() << std::endl;

       

        // Si le npc doit aller quelquepart !!!
        if (!npc.second.getChemin().empty()) {
            // On r�cup�re la case o� il doit aller
            int caseCible = npc.second.getChemin().getFirst();
            ss << "case cible = " << caseCible << std::endl;
            
            Tile::ETilePosition direction = m.getDirection(npc.second.getTileId(), caseCible);
            ss << "direction = " << direction << std::endl;

            // On enregistre le mouvement
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

			npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
        } else {
            ss << "case cible = Ne Bouge Pas" << std::endl;
            // M�me si le Npc ne bouge pas, il a quand m�me un mouvement statique !
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
        }

        GameManager::Log(ss.str());
    }
    return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
    // TODO !
    // Il faut r�ordonner les chemins entre les npcs !
    // Cad que si deux Npcs peuvent �changer leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
    reafecterObjectifsSelonDistance();

    // On r�cup�re tous les mouvements
    vector<Mouvement> mouvements = getAllMouvements();

    // Puis on va l'ordonner pour laisser la priorit� � celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {
        // On ne prend en compte notre mouvement que s'il compte
        if (mouvement.isNotStopped()) {
            // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
            actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
            // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
            npcs[mouvement.getNpcId()].move(mouvement.getDirection(), m);
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

vector<int> getIndicesMouvementsSurMemeCaseCible(vector<Mouvement>& mouvements, int caseCible) {
    vector<int> indices;
    for (int i = 0; i < mouvements.size(); ++i) {
        if (mouvements[i].getTileDestination() == caseCible) indices.push_back(i);
    }
    return indices;
}

int GameManager::getIndiceMouvementPrioritaire(vector<Mouvement>& mouvements, vector<int> indicesAConsiderer) {
    int indiceMax = indicesAConsiderer[0];
    int distanceMax = getNpcById(mouvements[indicesAConsiderer[0]].getNpcId()).getChemin().distance();
    for (int i = 0; i < indicesAConsiderer.size(); ++i) {
        // Si un mouvement est stationnaire, alors personne n'est autoris� � passer !
        if (!mouvements[indicesAConsiderer[i]].isNotStopped())
            return -1;
        int dist = getNpcById(mouvements[indicesAConsiderer[i]].getNpcId()).getChemin().distance();
        if (dist > distanceMax) {
            indiceMax = i;
            distanceMax = dist;
        }
    }
    return indiceMax;
}

void GameManager::stopNonPrioritaireMouvements(vector<Mouvement>& mouvements, vector<int> indicesMouvementsSurMemeCaseCible, int indiceMouvementPrioritaire, bool& continuer) {
    stringstream ss;

    for (int i = 0; i < indicesMouvementsSurMemeCaseCible.size(); ++i) {
        if (indicesMouvementsSurMemeCaseCible[i] != indiceMouvementPrioritaire) {
            int indice = indicesMouvementsSurMemeCaseCible[i];
            // Si le mouvement n'�tait pas d�j� � l'arr�t alors on a r�ellement effectu� un changement !
            if (mouvements[indice].isNotStopped())
                continuer = true;
            mouvements[indice].stop();
            npcs[mouvements[indice].getNpcId()].getChemin().resetChemin();
  
            ss << "Npc " << mouvements[indice].getNpcId();
            if (indiceMouvementPrioritaire != -1)
                ss << " a stopp� son mouvement pour laisser la place � Npc " << mouvements[indiceMouvementPrioritaire].getNpcId();
            else
                ss << " a stopp� son mouvement car quelqu'un est immobile.";
        }
    }

    GameManager::Log(ss.str());
}

void GameManager::gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements) {
    // Tant que l'on a fait une modification
    bool continuer = true;
    // Pour toutes les cases cibles
    while(continuer) {
        continuer = false;
        for (auto& mouvement : mouvements) {
            // On r�cup�re tous les indices des mouvements qui vont sur cette case
            vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(mouvements, mouvement.getTileDestination());

            // Si ils sont plusieurs � vouloir aller sur cette case
            if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
                // On r�cup�re le mouvement associ� au Npc ayant le plus de chemin � faire
                int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(mouvements, indicesMouvementsSurMemeCaseCible);

                // On passe tous les autres mouvements en Center !
                stopNonPrioritaireMouvements(mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
            }
        }
    }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& mouvements) noexcept {
    // Si deux npcs veulent aller sur la m�me case, alors celui qui a le plus de chemin � faire passe, et tous les autres restent sur place !
    gererCollisionsMemeCaseCible(mouvements);
}

void GameManager::addNewTiles(TurnInfo ti) noexcept {
    if (m.getNbTilesDecouvertes() < m.getNbTiles()) {
        // pour tous les npcs
        for (auto& npc : ti.npcs) {
            // On regarde les tuiles qu'ils voyent
            for (auto& tileId : npc.second.visibleTiles) {
                // Si ces tuiles n'ont pas �t� d�couvertes
                if (m.getTile(tileId).getStatut() == MapTile::INCONNU) {
                    // On les setDecouverte
                    m.addTile(ti.tiles[tileId]);
                }
            }
        }
    }
}

void GameManager::addNewObjects(TurnInfo ti) noexcept {
    // Tous les objets visibles par tous les npcs ...
    for (auto npc : ti.npcs) {
        for (auto objet : npc.second.visibleObjects) {
            // Si on ne conna�t pas cet objet on l'ajoute
            if(!m.objectExist(objet)) {
                m.addObject(ti.objects[objet]);
            }
        }
    }
}

void GameManager::updateModel(const TurnInfo &ti) noexcept {
   
    stringstream ss;

    // On essaye de rajouter les nouvelles tiles !
    auto pre = high_resolution_clock::now();
    addNewTiles(ti);
    auto post = high_resolution_clock::now();
    ss << "Dur�e AddTile = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;

    // On essaye de rajouter les nouvelles tiles !
    pre = high_resolution_clock::now();
    addNewObjects(ti);
    post = high_resolution_clock::now();
    ss << "Dur�e AddObjects = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;

    // Mettre � jour nos NPCs
    pre = std::chrono::high_resolution_clock::now();
    for (auto &npc : npcs) {
       npc.second.floodfill(m);
    }
    post = std::chrono::high_resolution_clock::now();
    ss << "Dur�e FloodFill = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms";

    GameManager::Log(ss.str());
}


Npc& GameManager::getNpcById(int id) {
    if (npcs.find(id) == npcs.end())
        throw npc_inexistant{};
    return npcs[id];
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
                    // Si l'interversion des objectifs est b�n�fique pour l'un deux et ne co�te rien � l'autre (ou lui est aussi b�n�fique)
                    if (npc.isAccessibleTile(objectifAutreNpc) // D�j� on v�rifie que l'intervertion est "possible"
                        && autreNpc.isAccessibleTile(objectifNpc)) {
                        if (max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                            // Alors on intervertit !                           
                            ss << "Npc " << npc.getId() << " et Npc " << autreNpc.getId() << " �changent leurs objectifs !" << std::endl;
                            npc.getChemin() = m.aStar(npc.getTileId(), objectifAutreNpc);
                            autreNpc.getChemin() = m.aStar(autreNpc.getTileId(), objectifNpc);
                            continuer = true; // Et on devra continuer pour v�rifier que cette intervertion n'en a pas entrain� de nouvelles !
                        }
                    }
                }
            }
        }
    }

    GameManager::Log(ss.str());
}
