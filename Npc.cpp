
#include "Npc.h"
#include "Globals.h"
#include "GameManager.h"

Npc::Npc(const NPCInfo info) : 
    id{ static_cast<int>(info.npcID) },
    tileId{static_cast<int>(info.tileID)},
    tileObjectif{ -1 },
    chemin{}
{
}


void Npc::move(Tile::ETilePosition direction, Map m) {
    tileId = m.getAdjacentTileAt(tileId, direction);
}

void Npc::resetChemins() {
    cheminsPossibles.clear();
    scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) {
    cheminsPossibles.push_back(chemin);
}

void Npc::addCheminWithScore(Chemin& chemin, float score) {
    addChemin(chemin);
    scoresAssocies.push_back(score);
}

Chemin Npc::getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas déjà pris et qu'il est plus court !
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), chemin.destination()) == objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}

int Npc::affecterMeilleurChemin() {
    if (scoresAssocies.empty() || cheminsPossibles.empty()) {
        // Dans ce cas-là on reste sur place !
        chemin = Chemin{};
        GameManager::Log("Le Npc " + to_string(id) + " n'a rien a rechercher et reste sur place !");
        return tileId;
    }

    // On cherche le meilleur score
    float bestScore = scoresAssocies[0];
    int bestScoreIndice = 0;
    GameManager::Log("Case potentielle à explorer : " + to_string(cheminsPossibles[0].destination()) + " de score " + to_string(scoresAssocies[0]));
    for (int i = 1; i < scoresAssocies.size(); ++i) {
        GameManager::Log("Case potentielle à explorer : " + to_string(cheminsPossibles[i].destination()) + " de score " + to_string(scoresAssocies[i]));
        if (scoresAssocies[i] > bestScore) {
            bestScore = scoresAssocies[i];
            bestScoreIndice = i;
        }
    }

    // On affecte son chemin
    chemin = cheminsPossibles[bestScoreIndice];
    GameManager::Log("Le Npc " + to_string(id) + " va rechercher la tile " + to_string(chemin.destination()));

    // On renvoie la destination
    return chemin.destination();
}
