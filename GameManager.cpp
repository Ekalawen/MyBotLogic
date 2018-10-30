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
#include "MyBotLogic/Tools/Minuteur.h"

// On initialise notre attribut statique ...
Logger GameManager::logger{};
Logger GameManager::loggerRelease{};

GameManager::GameManager(LevelInfo _info) :
    map{ Carte(_info) },
	objectifPris{ std::vector<int>{} }
{
    // On récupère l'ensemble des npcs !
    for (auto pair_npc : _info.npcs) {
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

std::vector<Mouvement> GameManager::getAllMouvements() {
    // On va récupérer la liste des mouvements
   std::vector<Mouvement> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        GameManager::log("NPC = " + std::to_string(npc.second.getId()));
        GameManager::log("chemin = " + npc.second.getChemin().toString());
        GameManager::log("case actuelle = " + std::to_string(npc.second.getTileId()));

        // Si le npc doit aller quelquepart !!!
        if (!npc.second.getChemin().empty()) {
            // On récupère la case où il doit aller
            int caseCible = npc.second.getChemin().getFirst();
            GameManager::log("case cible = " + std::to_string(caseCible));

            
            Tile::ETilePosition direction = map.getDirection(npc.second.getTileId(), caseCible);
            GameManager::log("direction = " + std::to_string(direction));

            // On enregistre le mouvement
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), caseCible, direction));

			npc.second.getChemin().removeFirst(); // On peut supprimer le chemin
        } else {
            GameManager::log("case cible = Ne Bouge Pas");
            // Même si le Npc ne bouge pas, il a quand même un mouvement statique !
            mouvements.push_back(Mouvement(npc.second.getId(), npc.second.getTileId(), npc.second.getTileId(), Tile::ETilePosition::CENTER));
        }
    }
    return mouvements;
}

void GameManager::moveNpcs(std::vector<Action*>& _actionList) noexcept {
    // TODO !
    // Il faut réordonner les chemins entre les npcs !
    // Cad que si deux Npcs peuvent échanger leurs objectifs et que cela diminue leurs chemins respectifs, alors il faut le faire !
    reaffecterObjectifsSelonDistance();

    // On récupère tous les mouvements
    std::vector<Mouvement> mouvements = getAllMouvements();

    // Puis on va l'ordonner pour laisser la priorité à celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {
        // On ne prend en compte notre mouvement que s'il compte
        if (mouvement.isNotStopped()) {
            // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
            _actionList.push_back(new Move(mouvement.getNpcId(), mouvement.getDirection()));
            // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
            npcs[mouvement.getNpcId()].move(mouvement.getDirection(), map);
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

std::vector<int> getIndicesMouvementsSurMemeCaseCible(std::vector<Mouvement>& _mouvements, int _caseCible) {
   std::vector<int> indices;
    for (int i = 0; i < _mouvements.size(); ++i) {
        if (_mouvements[i].getTileDestination() == _caseCible) indices.push_back(i);
    }
    return indices;
}

int GameManager::getIndiceMouvementPrioritaire(std::vector<Mouvement>& _mouvements, std::vector<int> _indicesAConsiderer) {
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

void GameManager::stopNonPrioritaireMouvements(std::vector<Mouvement>& _mouvements, std::vector<int> _indicesMouvementsSurMemeCaseCible, int _indiceMouvementPrioritaire, bool& _continuer) {
    for (int i = 0; i < _indicesMouvementsSurMemeCaseCible.size(); ++i) {
        if (_indicesMouvementsSurMemeCaseCible[i] != _indiceMouvementPrioritaire) {
            int indice = _indicesMouvementsSurMemeCaseCible[i];
            // Si le mouvement n'était pas déjà à l'arrêt alors on a réellement effectué un changement !
            if (_mouvements[indice].isNotStopped())
                _continuer = true;
            _mouvements[indice].stop();
            npcs[_mouvements[indice].getNpcId()].getChemin().resetChemin();
            if (_indiceMouvementPrioritaire != -1)
                GameManager::log("Npc " + std::to_string(_mouvements[indice].getNpcId()) + " a stoppé son mouvement pour laisser la place à Npc " + std::to_string(_mouvements[_indiceMouvementPrioritaire].getNpcId()));
            else
                GameManager::log("Npc " + std::to_string(_mouvements[indice].getNpcId()) + " a stoppé son mouvement car quelqu'un est immobile.");
        }
    }
}

void GameManager::gererCollisionsMemeCaseCible(std::vector<Mouvement>& _mouvements) {
    // Tant que l'on a fait une modification
    bool continuer = true;
    // Pour toutes les cases cibles
    while(continuer) {
        continuer = false;
        for (auto& mouvement : _mouvements) {
            // On récupère tous les indices des mouvements qui vont sur cette case
           std::vector<int> indicesMouvementsSurMemeCaseCible = getIndicesMouvementsSurMemeCaseCible(_mouvements, mouvement.getTileDestination());

            // Si ils sont plusieurs à vouloir aller sur cette case
            if (indicesMouvementsSurMemeCaseCible.size() >= 2) {
                // On récupère le mouvement associé au Npc ayant le plus de chemin à faire
                int indiceMouvementPrioritaire = getIndiceMouvementPrioritaire(_mouvements, indicesMouvementsSurMemeCaseCible);

                // On passe tous les autres mouvements en Center !
                stopNonPrioritaireMouvements(_mouvements, indicesMouvementsSurMemeCaseCible, indiceMouvementPrioritaire, continuer);
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

void GameManager::ordonnerMouvements(std::vector<Mouvement>& _mouvements) noexcept {
    // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
    gererCollisionsMemeCaseCible(_mouvements);

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

void GameManager::addNewTiles(TurnInfo _tile) noexcept {
    if (map.getNbTilesDecouvertes() < map.getNbTiles()) {
        // pour tous les npcs
        for (auto& npc : _tile.npcs) {
            // On regarde les tuiles qu'ils voyent
            for (auto& tileId : npc.second.visibleTiles) {
                // Si ces tuiles n'ont pas été découvertes
                if (map.getTile(tileId).getStatut() == MapTile::INCONNU) {
                    // On les setDecouverte
                    map.addTile(_tile.tiles[tileId]);
                }
            }
        }
    }
}

void GameManager::addNewObjects(TurnInfo _tile) noexcept {
    // Tous les objets visibles par tous les npcs ...
    for (auto npc : _tile.npcs) {
        for (auto objet : npc.second.visibleObjects) {
            // Si on ne connaît pas cet objet on l'ajoute
            if(!map.objectExist(objet)) {
                map.addObject(_tile.objects[objet]);
            }
        }
    }
}

void GameManager::updateModel(const TurnInfo &_tile) noexcept {
   
    // On essaye de rajouter les nouvelles tiles !
    auto pre = Minuteur::now();
    addNewTiles(_tile);
    auto post = Minuteur::now();
    GameManager::log("Durée AddTile = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");

    // On essaye de rajouter les nouvelles tiles !
    pre = Minuteur::now();
    addNewObjects(_tile);
    post = Minuteur::now();
    GameManager::log("Durée AddObjects = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");

    // Mettre à jour nos NPCs
    pre = Minuteur::now();
    for (auto &npc : npcs) {
       npc.second.floodfill(map);
    }
    post = Minuteur::now();
    GameManager::log("Durée FloodFill = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");
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
                    // Si l'interversion des objectifs est bénéfique pour l'un deux et ne coûte rien à l'autre (ou lui est aussi bénéfique)
                    if (npc.isAccessibleTile(objectifAutreNpc) // Déjà on vérifie que l'intervertion est "possible"
                        && autreNpc.isAccessibleTile(objectifNpc)) {
                        if (std::max(npc.distanceToTile(objectifAutreNpc), autreNpc.distanceToTile(objectifNpc)) < tempsMaxChemins) {// Ensuite que c'est rentable
                            // Alors on intervertit !
                            GameManager::log("Npc " + std::to_string(npc.getId()) + " et Npc " + std::to_string(autreNpc.getId()) + " échangent leurs objectifs !");
                            npc.getChemin() = map.aStar(npc.getTileId(), objectifAutreNpc);
                            autreNpc.getChemin() = map.aStar(autreNpc.getTileId(), objectifNpc);
                            continuer = true; // Et on devra continuer pour vérifier que cette intervertion n'en a pas entrainé de nouvelles !
                        }
                    }
                }
            }
        }
    }
}
