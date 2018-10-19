
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


void Npc::move(Tile::ETilePosition direction, Map &m) noexcept {
    tileId = m.getAdjacentTileAt(tileId, direction);
    m.tiles[tileId].statut = MapTile::Statut::VISITE;
}

void Npc::resetChemins() noexcept {
    cheminsPossibles.clear();
    scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) noexcept {
    cheminsPossibles.push_back(chemin);
}

void Npc::addScore(int tileIndice, float score) noexcept {
    scoresAssocies[tileIndice] = score;
}

Chemin Npc::getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas déjà pris et qu'il est plus court !
		int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est déjà arrivé il reste là
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}

int Npc::affecterMeilleurChemin(Map m) noexcept {
    if (scoresAssocies.empty()) {
        // Dans ce cas-là on reste sur place !
        chemin = Chemin{};
        GameManager::Log("Le Npc " + to_string(id) + " n'a rien a rechercher et reste sur place !");
        return tileId;
    }
	
    // On cherche le meilleur score
    float bestScore = scoresAssocies.begin()->second;
    int bestScoreIndice = scoresAssocies.begin()->first;
    GameManager::Log("Case potentielle à explorer : " + to_string(bestScoreIndice) + " de score " + to_string(bestScore));
    for (auto pair : scoresAssocies) {
        int tileId = pair.first;
        float score = pair.second;
        GameManager::Log("Case potentielle à explorer : " + to_string(tileId) + " de score " + to_string(score));
        if (score > bestScore) {
            bestScore = score;
            bestScoreIndice = tileId;
        }
    }

    // On affecte son chemin, mais il nous faut le calculer ! =)
    chemin = m.WAStar(tileId, bestScoreIndice);
    GameManager::Log("Le Npc " + to_string(id) + " va rechercher la tile " + to_string(chemin.destination()));

    // On renvoie la destination
    return chemin.destination();
}
