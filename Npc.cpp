
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
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); i++) {
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
        GameManager::Log("Appel de affecterMeilleurChemin sur des données vides !");
    }

    // On cherche le meilleur score
    float bestScore = -1;
    int bestScoreIndice = 0;
    for (int i = 0; i < scoresAssocies.size(); ++i) {
        if (scoresAssocies[i] > bestScore) {
            bestScore = scoresAssocies[i];
            bestScoreIndice = i;
        }
    }

    // On affecte son chemin
    chemin = cheminsPossibles[bestScoreIndice];

    // On renvoie la destination
    return chemin.destination();
}
