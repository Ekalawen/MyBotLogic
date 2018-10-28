
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
#include<chrono>
using namespace std;
using namespace std::chrono;

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo info) :
    m{ Map(info) },
	objectifPris{ vector<int>{} }
{
    // On récupère l'ensemble des npcs !
    for (auto pair_npc : info.npcs) {
        NPCInfo npc = pair_npc.second;
        npcs[npc.npcID] = Npc(npc);
    }
}

void GameManager::InitializeBehaviorTree() noexcept {
    //  Création du behaviorTree Manager
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
    // On va récupérer la liste des mouvements
    vector<Mouvement> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        GameManager::Log("NPC = " + to_string(npc.second.getId()));
        GameManager::Log("chemin = " + npc.second.getChemin().toString());
        GameManager::Log("case actuelle = " + to_string(npc.second.getTileId()));

        // Si le npc doit aller quelquepart !!!
        if (!npc.second.getChemin().empty()) {
            // On récupère la case où il doit aller
            int caseCible = npc.second.getChemin().getFirst();
            GameManager::Log("case cible = " + to_string(caseCible));

            
            Tile::ETilePosition direction = m.getDirection(npc.second.getTileId(), caseCible);
            GameManager::Log("direction = " + to_string(direction));

            // On enregistre le mouvement
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

			npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
        } else {
            GameManager::Log("case cible = Ne Bouge Pas");
            // Même si le Npc ne bouge pas, il a quand même un mouvement statique !
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
        }
    }
    return mouvements;
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
    // TODO !
    // Il faut réordonner les chemins entre les npcs !
    // Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
    reafecterObjectifsSelonDistance();

    // On récupère tous les mouvements
    vector<Mouvement> mouvements = getAllMouvements();

    // Puis on va l'ordonner pour laisser la priorité à celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {
        // On ne prend en compte notre mouvement que s'il compte
        if (mouvement.isNotStopped()) {
            // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
            actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
            // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
            npcs[mouvement.getNpcId()].move(mouvement.getDirection(), m);
            // TEST : pour chaque npc qui se déplace sur son objectif à ce tour, alors mettre estArrive à vrai
            if (mouvement.getDirection() != Tile::ETilePosition::CENTER && npcs[mouvement.getNpcId()].getTileObjectif() == mouvement.getTileDestination())
                // il faut aussi vérifier si tous les NPC ont un objectif atteignable, donc si on est en mode Exploitation
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
        // Si un mouvement est stationnaire, alors personne n'est autorisé à passer !
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
    for (int i = 0; i < indicesMouvementsSurMemeCaseCible.size(); ++i) {
        if (indicesMouvementsSurMemeCaseCible[i] != indiceMouvementPrioritaire) {
            int indice = indicesMouvementsSurMemeCaseCible[i];
            // Si le mouvement n'était pas déjà à l'arrêt alors on a réellement effectué un changement !
            if (mouvements[indice].isNotStopped())
                continuer = true;
            mouvements[indice].stop();
            npcs[mouvements[indice].getNpcId()].getChemin().resetChemin();
            if (indiceMouvementPrioritaire != -1)
                GameManager::Log("Npc " + to_string(mouvements[indice].getNpcId()) + " a stoppé son mouvement pour laisser la place à Npc " + to_string(mouvements[indiceMouvementPrioritaire].getNpcId()));
            else
                GameManager::Log("Npc " + to_string(mouvements[indice].getNpcId()) + " a stoppé son mouvement car quelqu'un est immobile.");
        }
    }
}

void GameManager::gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements) {
    // Tant que l'on a fait une modification
    bool continuer = true;
    // Pour toutes les cases cibles
    while(continuer) {
        continuer = false;
        for (auto& mouvement : mouvements) {
            // On récupère tous les indices des mouvements qui vont sur cette case
            vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(mouvements, mouvement.getTileDestination());

            // Si ils sont plusieurs à vouloir aller sur cette case
            if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
                // On récupère le mouvement associé au Npc ayant le plus de chemin à faire
                int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(mouvements, indicesMouvementsSurMemeCaseCible);

                // On passe tous les autres mouvements en Center !
                stopNonPrioritaireMouvements(mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
            }
        }


            //int tileCible = m.getAdjacentTileAt(npcs[mouvement->getNpcId()].getTileId(), mouvement->getDirection());
            //// On cherche celui qui est le plus long
            //int distMax = -1;
            //int npcMax = -1;
            //for (auto& mvt : mouvements) {
            //    int t = m.getAdjacentTileAt(npcs[mvt->getNpcId()].getTileId(), mvt->getDirection());
            //    if (t == tileCible) {
            //        int dist = npcs[mvt->getNpcId()].getChemin().distance();
            //        if (dist > distMax) {
            //            distMax = dist;
            //            npcMax = npcs[mvt->getNpcId()].getId();
            //        }
            //    }
            //}
            //// Puis pour tous ceux qui ne sont pas le plus long, on les passe en center !
            //for (auto& mvt : mouvements) {
            //    int t = m.getAdjacentTileAt(npcs[mvt->getNpcId()].getTileId(), mvt->getDirection());
            //    if (t == tileCible && npcs[mvt->getNpcId()].getId() != npcMax) {
            //        // Et il faut aussi lui rendre son mouvement pour le prochain tour !
            //        npcs[mvt->getNpcId()].getChemin().addFirst(m.getAdjacentTileAt(npcs[mvt->getNpcId()].getTileId(), mvt->getDirection()));
            //        mvt->stop();
            //        GameManager::Log("Npc " + to_string(mvt->getNpcId()) + " est poli et laisse passer.");
            //    }
            //}
    }
}

void GameManager::ordonnerMouvements(vector<Mouvement>& mouvements) noexcept {
    // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
    gererCollisionsMemeCaseCible(mouvements);

    // Si deux npcs adjacents veulent aller à peu près dans la même direction, le plus en avant doit bouger en premier !
    // On a même plus besoin de faire cela car l'ordre n'importe pas ! :)


    //// On récupère toutes les cases "sources" et toutes les cases "cibles"
    //// Si une case "cible" overlap une case "source", alors on doit faire se déplacer la "source" en premier !
    //// Pour ça on place le mouvement associé à la "cible" juste après la source =)
    //vector<tuple<int, int>> mouvementsCibles; // Première position = indice de la tile, deuxième l'indice du mouvement associé
    //vector<tuple<int, int>> mouvementsSources; // idem
    //for (int i = 0; i < mouvements.size(); ++i) {
    //    auto mouvement = mouvements[i];
    //    int tileCible = m.getAdjacentTileAt(npcs[mouvement->getNpcId()].getTileId(), mouvement->getDirection());
    //    int tileSource = npcs[mouvement->getNpcId()].getTileId();
    //    mouvementsCibles.push_back(tuple<int, int>(tileCible, i));
    //    mouvementsCibles.push_back(tuple<int, int>(tileSource, i));
    //}

    //while (!mouvementsCibles.empty()) {
    //    auto mouvement = mouvementsCibles.back();
    //    mouvementsCibles.pop_back();
    //    // On regarde si notre mouvement overlap !
    //    int lastToGo = -1;
    //    for (auto autre : mouvementsSources) {
    //        if (get<0>(autre) == get<0>(mouvement)
    //        && get<1>(autre) > lastToGo) {
    //            lastToGo = get<1>(autre);
    //        }
    //    }
    //    if (lastToGo != -1) {
    //        ++lastToGo;
    //        // On fait passer notre mouvement en dernière position possible !
    //        // Si lastToGo était en dernière position !
    //        if (lastToGo == mouvements.size()) {
    //            // Alors on supprime l'élément en cours et on le passe à la fin
    //            auto tmp = mouvements[get<1>(mouvement)];
    //            mouvements.erase(mouvements.begin() + get<1>(mouvement));
    //            mouvements.push_back(tmp);

    //        } else {
    //            // Alors on le fait passer en "lastToGo"
    //            auto tmp = mouvements[get<1>(mouvement)];
    //            mouvements.erase(mouvements.begin() + get<1>(mouvement));
    //            mouvements.insert(mouvements.begin() + lastToGo, tmp);
    //        }
    //    }
    //}
}

void GameManager::addNewTiles(TurnInfo ti) noexcept {
    if (m.getNbTilesDecouvertes() < m.getNbTiles()) {
        // pour tous les npcs
        for (auto& npc : ti.npcs) {
            // On regarde les tuiles qu'ils voyent
            for (auto& tileId : npc.second.visibleTiles) {
                // Si ces tuiles n'ont pas été découvertes
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
            // Si on ne connaît pas cet objet on l'ajoute
            if(!m.objectExist(objet)) {
                m.addObject(ti.objects[objet]);
            }
        }
    }
}

void GameManager::updateModel(const TurnInfo &ti) noexcept {
   
    // On essaye de rajouter les nouvelles tiles !
    auto pre = high_resolution_clock::now();
    addNewTiles(ti);
    auto post = high_resolution_clock::now();
    GameManager::Log("Durée AddTile = " + to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    // On essaye de rajouter les nouvelles tiles !
    pre = high_resolution_clock::now();
    addNewObjects(ti);
    post = high_resolution_clock::now();
    GameManager::Log("Durée AddObjects = " + to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    // Mettre à jour nos NPCs
    pre = std::chrono::high_resolution_clock::now();
    for (auto &npc : npcs) {
       npc.second.floodfill(m);
    }
    post = std::chrono::high_resolution_clock::now();
    GameManager::Log("Durée FloodFill = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
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
                        if (max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                            // Alors on intervertit !
                            GameManager::Log("Npc " + to_string(npc.getId()) + " et Npc " + to_string(autreNpc.getId()) + " échangent leurs objectifs !");
                            npc.getChemin() = m.aStar(npc.getTileId(), objectifAutreNpc);
                            autreNpc.getChemin() = m.aStar(autreNpc.getTileId(), objectifNpc);
                            continuer = true; // Et on devra continuer pour vérifier que cette intervertion n'en a pas entrainé de nouvelles !
                        }
                    }
                }
            }
        }
    }
}
