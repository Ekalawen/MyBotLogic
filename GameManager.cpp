
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

GameManager::GameManager(LevelInfo info) :
    m{Map(info)},
    objectifPris{}
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


void GameManager::associateNpcsWithObjectiv() noexcept {
    // Pour chaque npc, on calcul l'ensemble de ses chemins possibles !
    for (auto& pair_npc : npcs) {
        Npc& npc = pair_npc.second;
        npc.resetChemins();

        for (auto objectif : m.objectifs) {
            Chemin chemin = m.WAStar(npc.tileId, objectif);
            npc.addChemin(chemin);
        }
    }

    // Tant qu'il reste des Npcs qui n'ont pas de chemins
    vector<int> npcAffectes;
    vector<int> objectifsPris; // Les objectifs ayant d�j� �t� affect�s
    while (npcAffectes.size() < npcs.size()) {
        // On r�cup�re le npc qui a le chemin minimal le plus long
        Npc* lastNpc;
        int distMax = -1;
        Chemin cheminMin;
        for (auto& pair_npc : npcs) {
            Npc& npc = pair_npc.second;
            if (find(npcAffectes.begin(), npcAffectes.end(), npc.id) == npcAffectes.end()) { // Si on a pas d�j� affect� cet npc !
                Chemin chemin = npc.getCheminMinNonPris(objectifsPris, m.tailleCheminMax());
                if (chemin.isAccessible() && chemin.distance() > distMax) {
                    lastNpc = &npc;
                    distMax = chemin.distance();
                    cheminMin = chemin;
                }
            }
        }

        // Puis on lui affecte son chemin le plus court !
        lastNpc->chemin = cheminMin;
        if (!cheminMin.empty()) {
            lastNpc->tileObjectif = cheminMin.destination();
            objectifsPris.push_back(cheminMin.destination());
        } else {
            lastNpc->tileObjectif = lastNpc->tileId;
            objectifsPris.push_back(lastNpc->tileId);
        }

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
}

void GameManager::moveNpcs(vector<Action*>& actionList) noexcept {
    // On va r�cup�rer la liste des mouvements
    vector<Mouvement*> mouvements;

    // Pour tous les NPCs, s'il n'y a aucun autre Npc devant eux
    for (auto& npc : npcs) {
        // Si le npc doit aller quelquepart !!!
        GameManager::Log("NPC = " + to_string(npc.second.id));
        GameManager::Log("chemin = " + npc.second.chemin.toString());
        GameManager::Log("case actuelle = " + to_string(npc.second.tileId));
        if (!npc.second.chemin.chemin.empty()) {
            // On r�cup�re la case o� il doit aller
            int caseCible = npc.second.chemin.chemin.back();
            GameManager::Log("case cible = " + to_string(caseCible));

            
            Tile::ETilePosition direction = m.getDirection(npc.second.tileId, caseCible);
            GameManager::Log("direction = " + to_string(direction));

            // On enregistre le mouvement
            mouvements.push_back(new Mouvement(npc.second.id, npc.second.tileId, caseCible, direction));

			npc.second.chemin.chemin.pop_back(); // On peut supprimer le chemin
        } else {
            GameManager::Log("case cible = Ne Bouge Pas");
        }
    }

    // Puis on va l'ordonner pour laisser la priorit� � celui qui va le plus loin !
	ordonnerMouvements(mouvements);

    // Puis pour chaque mouvement
    for (auto mouvement : mouvements) {
        // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
        actionList.push_back(new Move(mouvement->npcID, mouvement->direction));
        // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
        npcs[mouvement->npcID].move(mouvement->direction, m);
		// TEST : pour chaque npc qui se d�place sur son objectif � ce tour, alors mettre estArrive � vrai
		if (mouvement->direction != Tile::ETilePosition::CENTER && npcs[mouvement->npcID].tileObjectif == mouvement->tileDestination)
			// il faut aussi v�rifier si tous les NPC ont un objectif atteignable, donc si on est en mode Exploitation
		{
			npcs[mouvement->npcID].estArrive = true;
		}
		else {
			npcs[mouvement->npcID].estArrive = false;
		}
    }
}

void GameManager::ordonnerMouvements(vector<Mouvement*>& mouvements) noexcept {
    // Quels sont les cas particuliers ?
    // Si deux npcs veulent aller sur la m�me case, alors celui qui a le plus de chemin � faire passe, et tous les autres restent sur place !
    // OK !
    // Si deux npcs adjacents veulent aller � peu pr�s dans la m�me direction, le plus en avant doit bouger en premier !
    // OK !
    // Si deux npcs adjacents veulent aller dans des directions oppos�es ... C'est g�r� par le moteur du prof =)
    // TODO !

    // Pour toutes les tiles auxquelles on pourrait acc�der ...
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
                npcs[mvt->npcID].chemin.chemin.push_back(m.getAdjacentTileAt(npcs[mvt->npcID].tileId, mvt->direction));
                mvt->direction = Tile::CENTER;
                GameManager::Log("Npc " + to_string(mvt->npcID) + " est poli et laisse passer.");
            }
        }
    }

    // On r�cup�re toutes les cases "sources" et toutes les cases "cibles"
    // Si une case "cible" overlap une case "source", alors on doit faire se d�placer la "source" en premier !
    // Pour �a on place le mouvement associ� � la "cible" juste apr�s la source =)
    vector<tuple<int, int>> mouvementsCibles; // Premi�re position = indice de la tile, deuxi�me l'indice du mouvement associ�
    vector<tuple<int, int>> mouvementsSources; // idem
    for (int i = 0; i < mouvements.size(); ++i) {
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
            ++lastToGo;
            // On fait passer notre mouvement en derni�re position possible !
            // Si lastToGo �tait en derni�re position !
            if (lastToGo == mouvements.size()) {
                // Alors on supprime l'�l�ment en cours et on le passe � la fin
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

void GameManager::addNewTiles(TurnInfo ti) noexcept {
    if (m.nbtilesDecouvertes < m.nbTiles) {
        // pour tous les npcs
        for (auto& npc : ti.npcs) {
            // On regarde les tuiles qu'ils voyent
            for (auto& tileId : npc.second.visibleTiles) {
                // Si ces tuiles n'ont pas �t� d�couvertes
                if (m.tiles[tileId].statut == MapTile::INCONNU) {
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
            if (m.murs.find(objet) == m.murs.end()
                && m.portes.find(objet) == m.portes.end()
                && m.fenetres.find(objet) == m.fenetres.end()
                && m.activateurs.find(objet) == m.activateurs.end()) {
                m.addObject(ti.objects[objet]);
            }
        }
    }
}

void GameManager::updateModel(TurnInfo ti) noexcept {
    // On essaye de rajouter les nouveaux objectifs et les nouvelles tiles !
   
    auto pre = high_resolution_clock::now();
    addNewTiles(ti);
    auto post = high_resolution_clock::now();
    GameManager::Log("Dur�e AddTile = " + to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    pre = high_resolution_clock::now();
    addNewObjects(ti);
    post = high_resolution_clock::now();
    GameManager::Log("Dur�e AddObjects = " + to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    for (auto &npc : npcs) {
       m.floodfill(npc.second);
    }
}

