
#include "GameManager.h"
#include "Mouvement.h"

#include "BehaviorTree/BT_Noeud.h"
#include "BehaviorTree/BT_Composite.h"
#include "BehaviorTree/Composite/Sequenceur.h"
#include "BehaviorTree/Composite/Selecteur.h"
#include "BT_Tests/ObjectifsForAllNpcs.h"
#include "Strategies/Expedition.h"
#include "Strategies/Exploration.h"
#include "Strategies/Exploitation.h"

#include <algorithm>
#include <tuple>
using namespace std;

// On initialise notre attribut statique ...
Logger GameManager::logger{};

GameManager::GameManager(LevelInfo info) :
    m{Map(info)},
    objectifPris{}
{
    // On récupère l'ensemble des npcs !
    for (auto pair_npc : info.npcs) {
        NPCInfo npc = pair_npc.second;
        npcs[npc.npcID] = Npc(npc);
    }
}

void GameManager::InitializeBehaviorTree() {
    //  Création du behaviorTree Manager
    ObjectifsForAllNpcs *objectifs = new ObjectifsForAllNpcs(*this);
    ScoreStrategie *expedition = new Expedition(*this, "Expedition");
    ScoreStrategie *exploration = new Exploration(*this, "Exploration");
    Exploitation *exploitation = new Exploitation(*this);

    Selecteur *selecteur = new Selecteur({ exploitation, expedition });

    Sequenceur *sequenceur = new Sequenceur({ objectifs, selecteur });

    behaviorTreeManager = Selecteur({ sequenceur, exploration });
}


void GameManager::associateNpcsWithObjectiv() {
    // Pour chaque npc, on calcul l'ensemble de ses chemins possibles !
    for (auto& pair_npc : npcs) {
        Npc& npc = pair_npc.second;
        npc.resetChemins();

        for (auto objectif : m.objectifs) {
            Chemin chemin = m.aStar(npc.tileId, objectif.second.id);
            npc.addChemin(chemin);
        }
    }

    // Tant qu'il reste des Npcs qui n'ont pas de chemins
    vector<int> npcAffectes;
    vector<int> objectifsPris; // Les objectifs ayant déjà été affectés
    while (npcAffectes.size() < npcs.size()) {
        // On récupère le npc qui a le chemin minimal le plus long
        Npc* lastNpc;
        int distMax = -1;
        Chemin cheminMin;
        for (auto& pair_npc : npcs) {
            Npc& npc = pair_npc.second;
            if (find(npcAffectes.begin(), npcAffectes.end(), npc.id) == npcAffectes.end()) { // Si on a pas déjà affecté cet npc !
                Chemin chemin = npc.getCheminMinNonPris(objectifsPris, m.tailleCheminMax());
                if (chemin.distance() > distMax) {
                    lastNpc = &npc;
                    distMax = chemin.distance();
                    cheminMin = chemin;
                }
            }
        }

        // Puis on lui affecte son chemin le plus court !
        lastNpc->chemin = cheminMin;
        lastNpc->tileObjectif = cheminMin.destination();
        objectifsPris.push_back(cheminMin.destination());

        npcAffectes.push_back(lastNpc->id);
    }

    // Puis on affiche les chemins !
    for (auto npc_pair : npcs) {
        Npc npc = npc_pair.second;
        GameManager::Log("NPC = " + to_string(npc.id));
        GameManager::Log("objectif choisi = " + to_string(npc.chemin.destination()));
        GameManager::Log("distance = " + to_string(npc.chemin.distance()));
        GameManager::Log("chemin :");
        for (auto tile : npc.chemin.chemin) {
            GameManager::Log(to_string(tile));
        }
    }

    /*
    // Les objectifs déjà pris
    vector<int> objectifs_pris;

    // Pour chaque npc, on trouve l'objectif le plus proche possible de lui
    for (auto& pair_npc : npcs) {

        Npc npc = pair_npc.second;
        
        // Chercher l'objectif le plus proche
        int dist_min = m.rowCount * m.colCount + 1;
        int objectifChoisi = -1;
        vector<int> cheminOptimal;
        for (auto objectif : m.objectifs) {
            // Si l'objectif n'est pas déjà pris !
            bool pris = false;
            for (int i = 0; i < objectifs_pris.size(); i++) {
                if (objectifs_pris[i] == objectif.second.id) {
                    pris = true;
                    break;
                }
            }
            if (!pris) {
                vector<int> chemin = m.chemin(npc.tileId, objectif.second.id);
                int dist = static_cast<int>(chemin.size());
                if (dist < dist_min) {
                    dist_min = dist;
                    objectifChoisi = objectif.second.id;
                    cheminOptimal = chemin;
                }
            }
        }
        // Lui affecter le chemin et l'objectif !
        pair_npc.second.tileObjectif = objectifChoisi;
        pair_npc.second.chemin = cheminOptimal;

        GameManager::Log("NPC = " + to_string(npc.id));
        GameManager::Log("objectif choisi = " + to_string(objectifChoisi));
        GameManager::Log("distance = " + to_string(dist_min));
        GameManager::Log("chemin :");
        for (auto tile : pair_npc.second.chemin) {
            GameManager::Log(to_string(tile));
        }

        // Rendre cet objectif inutilisable par les autres npcs !
        objectifs_pris.push_back(objectifChoisi);
    }
    */
}

void GameManager::moveNpcs(vector<Action*>& actionList) {
    // On va récupérer la liste des mouvements
    vector<Mouvement*> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        // Si le npc doit aller quelquepart !!!
        GameManager::Log("NPC = " + to_string(npc.second.id));
        GameManager::Log("case actuelle = " + to_string(npc.second.tileId));
        if (!npc.second.chemin.chemin.empty()) {
            // On récupère la case où il doit aller
            int caseCible = npc.second.chemin.chemin.back();
            GameManager::Log("case cible = " + to_string(caseCible));

            npc.second.chemin.chemin.pop_back(); // On peut supprimer le chemin
            Tile::ETilePosition direction = m.getDirection(npc.second.tileId, caseCible);
            GameManager::Log("direction = " + to_string(direction));

            // On enregistre le mouvement
            mouvements.push_back(new Mouvement(npc.second.id, npc.second.tileId, caseCible, direction));

            //}
        } else {
            GameManager::Log("case cible = Ne Bouge Pas");
        }
    }

    // Puis on va l'ordonner pour laisser la priorité à celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {
        // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
        actionList.push_back(new Move(mouvement->npcID, mouvement->direction));
        // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
        npcs[mouvement->npcID].move(mouvement->direction, m);
    }
}

void GameManager::ordonnerMouvements(vector<Mouvement*>& mouvements) {
    // Quels sont les cas particuliers ?
    // Si deux npcs veulent aller sur la même case, alors celui qui a le plus de chemin à faire passe, et tous les autres restent sur place !
    // OK !
    // Si deux npcs adjacents veulent aller à peu près dans la même direction, le plus en avant doit bouger en premier !
    // OK !
    // Si deux npcs adjacents veulent aller dans des directions opposées ... XD TODO !!!
    // TODO !

    // Pour toutes les tiles auxquelles on pourrait accéder ...
    for (auto& mouvement : mouvements) {
        int tileCible = m.getAdjacentTileAt(npcs[mouvement->npcID].tileId, mouvement->direction);
        // On cherche celui qui est le plus long
        int distMax = -1;
        int npcMax = -1;
        for (auto& mvt : mouvements) {
            int t = m.getAdjacentTileAt(npcs[mvt->npcID].tileId, mvt->direction);
            if (t == tileCible) {
                int dist = npcs[mvt->npcID].chemin.distance();
                if (dist > distMax) {
                    distMax = dist;
                    npcMax = npcs[mvt->npcID].id;
                }
            }
        }
        // Puis pour tous ceux qui ne sont pas le plus long, on les passe en center !
        for (auto& mvt : mouvements) {
            int t = m.getAdjacentTileAt(npcs[mvt->npcID].tileId, mvt->direction);
            if (t == tileCible && npcs[mvt->npcID].id != npcMax) {
                // Et il faut aussi lui rendre son mouvement pour le prochain tour !
                npcs[mvt->direction].chemin.chemin.push_back(m.getAdjacentTileAt(npcs[mvt->direction].tileId, mvt->direction));
                mvt->direction = Tile::CENTER;
                GameManager::Log("Npc " + to_string(mvt->npcID) + " est poli et laisse passer.");
            }
        }
    }

    // On récupère toutes les cases "sources" et toutes les cases "cibles"
    // Si une case "cible" overlap une case "source", alors on doit faire se déplacer la "source" en premier !
    // Pour ça on place le mouvement associé à la "cible" juste après la source =)
    vector<tuple<int, int>> mouvementsCibles; // Première position = indice de la tile, deuxième l'indice du mouvement associé
    vector<tuple<int, int>> mouvementsSources; // idem
    for (int i = 0; i < mouvements.size(); i++) {
        auto mouvement = mouvements[i];
        int tileCible = m.getAdjacentTileAt(npcs[mouvement->npcID].tileId, mouvement->direction);
        int tileSource = npcs[mouvement->npcID].tileId;
        mouvementsCibles.push_back(tuple<int, int>(tileCible, i));
        mouvementsCibles.push_back(tuple<int, int>(tileSource, i));
    }

    while (!mouvementsCibles.empty()) {
        auto mouvement = mouvementsCibles.back();
        mouvementsCibles.pop_back();
        // On regarde si notre mouvement overlap !
        int lastToGo = -1;
        for (auto autre : mouvementsSources) {
            if (get<0>(autre) == get<0>(mouvement)
            && get<1>(autre) > lastToGo) {
                lastToGo = get<1>(autre);
            }
        }
        if (lastToGo != -1) {
            lastToGo++;
            // On fait passer notre mouvement en dernière position possible !
            // Si lastToGo était en dernière position !
            if (lastToGo == mouvements.size()) {
                // Alors on supprime l'élément en cours et on le passe à la fin
                auto tmp = mouvements[get<1>(mouvement)];
                mouvements.erase(mouvements.begin() + get<1>(mouvement));
                mouvements.push_back(tmp);

            } else {
                // Alors on le fait passer en "lastToGo"
                auto tmp = mouvements[get<1>(mouvement)];
                mouvements.erase(mouvements.begin() + get<1>(mouvement));
                mouvements.insert(mouvements.begin() + lastToGo, tmp);
            }
        }
    }
}

void GameManager::addNewTiles(TurnInfo ti) {
    // On vérifie si on a pas déjà la connaissance totale sur les tiles de la map
    if (m.nbtilesDecouvertes < m.nbTiles) {
        // On va regarder si on a découvert des tiles
        for (auto tile : ti.tiles) {
            // Si on ne connaît pas cette tile, on l'ajoute
            if (m.tiles.find(tile.second.tileID) == m.tiles.end()) {
                m.addTile(tile.second);
            }
        }
    }
}

void GameManager::addNewObjects(TurnInfo ti) {
    // Tous les objets
    for (auto objet : ti.objects) {
        // Si on ne connaît pas cet objet on l'ajoute
        if (m.murs.find(objet.second.objectID) == m.murs.end()
         && m.portes.find(objet.second.objectID) == m.portes.end()
         && m.fenetres.find(objet.second.objectID) == m.fenetres.end()
         && m.activateurs.find(objet.second.objectID) == m.activateurs.end()) {
            m.addObject(objet.second);
        }
    }
}

void GameManager::updateModel(TurnInfo ti) {
    // On essaye de rajouter les nouveaux objectifs et les nouvelles tiles !
    addNewTiles(ti);
    addNewObjects(ti);
}